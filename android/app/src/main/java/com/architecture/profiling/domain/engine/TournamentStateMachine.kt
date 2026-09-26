package com.architecture.profiling.domain.engine

import com.architecture.profiling.domain.model.*

object TournamentStateMachine {

    fun create(mode: TournamentMode = TournamentMode.FULL): TournamentState {
        val schedule = PairingScheduler.generateSchedule(mode)
        val matchRecords = schedule.map { pair ->
            MatchRecord(
                matchIndex = pair.matchIndex,
                roundIndex = pair.roundIndex,
                leftStyleId = pair.leftStyleId,
                rightStyleId = pair.rightStyleId
            )
        }
        return TournamentState(
            mode = mode,
            matches = matchRecords,
            currentIndex = 0
        )
    }

    fun vote(state: TournamentState, winnerStyleId: Int): TournamentState {
        if (!state.canVote) return state
        val current = state.currentMatch ?: return state
        require(winnerStyleId == current.leftStyleId || winnerStyleId == current.rightStyleId) {
            "Winner $winnerStyleId must be either ${current.leftStyleId} or ${current.rightStyleId}"
        }

        val outcome = if (winnerStyleId == current.leftStyleId) MatchOutcome.LEFT_WON else MatchOutcome.RIGHT_WON
        val updatedRecord = current.copy(outcome = outcome, winnerStyleId = winnerStyleId)

        val updatedMatches = state.matches.toMutableList()
        updatedMatches[state.currentIndex] = updatedRecord

        return state.copy(
            matches = updatedMatches,
            currentIndex = state.currentIndex + 1,
            historyStack = state.historyStack + updatedRecord,
            redoStack = emptyList() // Clear redo stack on new vote branch
        )
    }

    fun undo(state: TournamentState): TournamentState {
        if (!state.canUndo) return state
        val lastRecord = state.historyStack.last()
        val targetIndex = state.currentIndex - 1

        val resetRecord = state.matches[targetIndex].copy(
            outcome = MatchOutcome.UNDECIDED,
            winnerStyleId = null
        )

        val updatedMatches = state.matches.toMutableList()
        updatedMatches[targetIndex] = resetRecord

        return state.copy(
            matches = updatedMatches,
            currentIndex = targetIndex,
            historyStack = state.historyStack.dropLast(1),
            redoStack = state.redoStack + lastRecord
        )
    }

    fun redo(state: TournamentState): TournamentState {
        if (!state.canRedo) return state
        val redoRecord = state.redoStack.last()
        val targetIndex = state.currentIndex

        val updatedMatches = state.matches.toMutableList()
        updatedMatches[targetIndex] = redoRecord

        return state.copy(
            matches = updatedMatches,
            currentIndex = targetIndex + 1,
            historyStack = state.historyStack + redoRecord,
            redoStack = state.redoStack.dropLast(1)
        )
    }

    fun buildWinMatrix(state: TournamentState, n: Int = 10): Array<IntArray> {
        val matrix = Array(n) { IntArray(n) }
        for (m in state.matches) {
            if (m.isPlayed && m.winnerStyleId != null) {
                val loserId = if (m.winnerStyleId == m.leftStyleId) m.rightStyleId else m.leftStyleId
                matrix[m.winnerStyleId - 1][loserId - 1]++
            }
        }
        return matrix
    }

    fun generateResult(
        state: TournamentState,
        styles: List<Style>,
        categories: List<Category>
    ): TournamentResult {
        val winMatrix = buildWinMatrix(state)
        val probabilities = PreferenceScorer.calculateBradleyTerry(winMatrix)
        val styleRankings = PreferenceScorer.rankStyles(probabilities, winMatrix)
        val categoryRankings = PreferenceScorer.aggregateCategories(probabilities, styles, categories)

        val circularTriads = TransitivityCalculator.calculateCircularTriads(winMatrix, state.totalMatches)
        val consistencyZeta = TransitivityCalculator.calculateZeta(circularTriads)

        val victoryMargin = ConfidenceEngine.calculateVictoryMargin(probabilities)
        val entropy = ConfidenceEngine.calculateEntropy(probabilities)
        val entropyConcentration = ConfidenceEngine.calculateEntropyConcentration(entropy, styles.size)
        val confidence = ConfidenceEngine.calculateConfidence(consistencyZeta, victoryMargin, entropyConcentration)

        val userTraits = ArchetypeEngine.calculateUserTraits(probabilities, styles)
        val dominantCategory = categoryRankings.firstOrNull()?.categoryId ?: "classical_renaissance"
        val championStyleId = styleRankings.firstOrNull()?.styleId ?: 1
        val archetype = ArchetypeEngine.classify(dominantCategory, userTraits, consistencyZeta, styles)

        return TournamentResult(
            styleRankings = styleRankings,
            categoryRankings = categoryRankings,
            championStyleId = championStyleId,
            dominantCategoryId = dominantCategory,
            circularTriads = circularTriads,
            consistencyZeta = consistencyZeta,
            victoryMargin = victoryMargin,
            entropyConcentration = entropyConcentration,
            confidencePercentage = confidence,
            userTraits = userTraits,
            archetype = archetype,
            matchesPlayed = state.matchesPlayed,
            totalMatches = state.totalMatches
        )
    }
}
