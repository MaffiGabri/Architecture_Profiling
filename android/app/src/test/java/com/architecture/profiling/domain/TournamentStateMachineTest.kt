package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.TournamentStateMachine
import com.architecture.profiling.domain.model.MatchOutcome
import com.architecture.profiling.domain.model.TournamentMode
import org.junit.Assert.*
import org.junit.Test

class TournamentStateMachineTest {

    @Test
    fun initialState_isCorrect() {
        val state = TournamentStateMachine.create(TournamentMode.FULL)
        assertEquals(0, state.currentIndex)
        assertEquals(45, state.totalMatches)
        assertEquals(0, state.matchesPlayed)
        assertEquals(0.0, state.progressPercentage, 1e-6)
        assertTrue(state.canVote)
        assertFalse(state.canUndo)
        assertFalse(state.canRedo)
        assertFalse(state.isComplete)
        assertNotNull(state.currentMatch)
    }

    @Test
    fun voting_advancesStateAndRecordsWinner() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        val firstMatch = state.currentMatch!!
        val chosenWinner = firstMatch.leftStyleId

        state = TournamentStateMachine.vote(state, chosenWinner)
        assertEquals(1, state.currentIndex)
        assertEquals(1, state.historyStack.size)
        assertTrue(state.canUndo)
        assertFalse(state.canRedo)

        val recorded = state.matches[0]
        assertTrue(recorded.isPlayed)
        assertEquals(MatchOutcome.LEFT_WON, recorded.outcome)
        assertEquals(chosenWinner, recorded.winnerStyleId)
    }

    @Test
    fun undoAndRedo_cyclesStateCorrectly() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        val m1 = state.currentMatch!!
        state = TournamentStateMachine.vote(state, m1.leftStyleId)
        val m2 = state.currentMatch!!
        state = TournamentStateMachine.vote(state, m2.rightStyleId)

        assertEquals(2, state.currentIndex)

        // Undo once
        state = TournamentStateMachine.undo(state)
        assertEquals(1, state.currentIndex)
        assertTrue(state.canRedo)
        assertFalse(state.matches[1].isPlayed)

        // Redo once
        state = TournamentStateMachine.redo(state)
        assertEquals(2, state.currentIndex)
        assertTrue(state.matches[1].isPlayed)
        assertEquals(m2.rightStyleId, state.matches[1].winnerStyleId)
    }

    @Test
    fun newVote_afterUndo_clearsRedoStack() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        state = TournamentStateMachine.vote(state, state.currentMatch!!.leftStyleId)
        state = TournamentStateMachine.vote(state, state.currentMatch!!.rightStyleId)
        state = TournamentStateMachine.undo(state)
        assertTrue(state.canRedo)

        // Cast different vote
        state = TournamentStateMachine.vote(state, state.currentMatch!!.leftStyleId)
        assertFalse("Redo stack must be cleared after branching vote", state.canRedo)
        assertEquals(0, state.redoStack.size)
    }

    @Test
    fun rapidUndo_atBoundary_doesNotCrash() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        repeat(10) {
            state = TournamentStateMachine.undo(state)
        }
        assertEquals(0, state.currentIndex)
        assertFalse(state.canUndo)
    }

    @Test
    fun rapidRedo_atBoundary_doesNotCrash() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        repeat(10) {
            state = TournamentStateMachine.redo(state)
        }
        assertEquals(0, state.currentIndex)
        assertFalse(state.canRedo)
    }

    @Test
    fun fullTournamentCompletion_setsIsComplete() {
        var state = TournamentStateMachine.create(TournamentMode.QUICK)
        repeat(15) {
            state = TournamentStateMachine.vote(state, state.currentMatch!!.leftStyleId)
        }
        assertTrue(state.isComplete)
        assertFalse(state.canVote)
        assertEquals(100.0, state.progressPercentage, 1e-6)
    }

    @Test(expected = IllegalArgumentException::class)
    fun votingInvalidWinner_throwsException() {
        val state = TournamentStateMachine.create(TournamentMode.FULL)
        TournamentStateMachine.vote(state, 999)
    }
}
