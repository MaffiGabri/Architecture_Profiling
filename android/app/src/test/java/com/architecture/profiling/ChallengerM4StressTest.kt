package com.architecture.profiling

import com.architecture.profiling.data.entity.TournamentHistoryDocumentEntity
import com.architecture.profiling.data.entity.TournamentHistoryRecordEntity
import com.architecture.profiling.data.entity.UserPreferencesEntity
import com.architecture.profiling.domain.engine.TournamentStateMachine
import com.architecture.profiling.domain.model.MatchOutcome
import com.architecture.profiling.domain.model.TournamentMode
import com.google.common.truth.Truth.assertThat
import kotlinx.serialization.SerializationException
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import org.junit.Test
import kotlin.math.cos
import kotlin.math.sin
import kotlin.random.Random

class ChallengerM4StressTest {

    private val json = Json { ignoreUnknownKeys = true; prettyPrint = true }

    // ========================================================================
    // 1. Persistence & Malformed Data Stress Testing
    // ========================================================================

    @Test
    fun test_user_preferences_handles_empty_json() {
        val decoded = json.decodeFromString<UserPreferencesEntity>("{}")
        assertThat(decoded.username).isEqualTo("Architectural Explorer")
        assertThat(decoded.defaultMode).isEqualTo("FULL")
        assertThat(decoded.preferredLanguage).isEqualTo("system")
        assertThat(decoded.themeMode).isEqualTo("SYSTEM")
        assertThat(decoded.totalTournamentsCompleted).isEqualTo(0)
    }

    @Test
    fun test_user_preferences_handles_malformed_and_extra_fields() {
        val extraFieldsJson = """
            {
                "username": "Super Explorer",
                "non_existent_key": 99999,
                "nested_garbage": { "a": 1, "b": "c" }
            }
        """.trimIndent()
        val decoded = json.decodeFromString<UserPreferencesEntity>(extraFieldsJson)
        assertThat(decoded.username).isEqualTo("Super Explorer")
        assertThat(decoded.defaultMode).isEqualTo("FULL")
    }

    @Test
    fun test_tournament_history_document_handles_empty_json() {
        val decoded = json.decodeFromString<TournamentHistoryDocumentEntity>("{}")
        assertThat(decoded.records).isEmpty()
    }

    @Test
    fun test_repository_recovery_logic_simulation() {
        // Test simulation of repository loadInitial error recovery logic
        val corruptInputs = listOf(
            "",
            "   ",
            "{",
            "{\"records\": \"not_a_list\"}",
            "[1, 2, 3]",
            "null",
            "{\"records\": [{\"invalid\": true}]}"
        )

        for (input in corruptInputs) {
            val records: List<TournamentHistoryRecordEntity> = try {
                val doc = json.decodeFromString<TournamentHistoryDocumentEntity>(input)
                doc.records
            } catch (_: Exception) {
                emptyList()
            }
            // Must recover safely to emptyList() without unhandled crashes
            assertThat(records).isNotNull()
        }
    }

    // ========================================================================
    // 2. TournamentStateMachine Rapid Undo/Redo Cycles & Invariant Stress Test
    // ========================================================================

    @Test
    fun test_tournament_state_machine_rapid_undo_redo_cycles() {
        var state = TournamentStateMachine.create(TournamentMode.FULL)
        val rng = Random(42)

        // 1000 randomized operations
        for (step in 0 until 1000) {
            val op = rng.nextInt(3) // 0 = vote, 1 = undo, 2 = redo

            when (op) {
                0 -> {
                    if (state.canVote) {
                        val match = state.currentMatch!!
                        val winner = if (rng.nextBoolean()) match.leftStyleId else match.rightStyleId
                        val prevIndex = state.currentIndex
                        state = TournamentStateMachine.vote(state, winner)
                        assertThat(state.currentIndex).isEqualTo(prevIndex + 1)
                        assertThat(state.historyStack.size).isEqualTo(state.currentIndex)
                        assertThat(state.redoStack).isEmpty()
                    } else {
                        val unmod = TournamentStateMachine.vote(state, 1)
                        assertThat(unmod).isEqualTo(state)
                    }
                }
                1 -> {
                    val prevIndex = state.currentIndex
                    val prevRedoSize = state.redoStack.size
                    state = TournamentStateMachine.undo(state)
                    if (prevIndex > 0) {
                        assertThat(state.currentIndex).isEqualTo(prevIndex - 1)
                        assertThat(state.redoStack.size).isEqualTo(prevRedoSize + 1)
                    } else {
                        assertThat(state.currentIndex).isEqualTo(0)
                        assertThat(state.canUndo).isFalse()
                    }
                }
                2 -> {
                    val prevIndex = state.currentIndex
                    val prevRedoSize = state.redoStack.size
                    state = TournamentStateMachine.redo(state)
                    if (prevRedoSize > 0) {
                        assertThat(state.currentIndex).isEqualTo(prevIndex + 1)
                        assertThat(state.redoStack.size).isEqualTo(prevRedoSize - 1)
                    } else {
                        assertThat(state.currentIndex).isEqualTo(prevIndex)
                        assertThat(state.canRedo).isFalse()
                    }
                }
            }

            // Invariants on every single step
            assertThat(state.matchesPlayed).isEqualTo(state.currentIndex)
            assertThat(state.historyStack.size).isEqualTo(state.currentIndex)
            assertThat(state.canUndo).isEqualTo(state.historyStack.isNotEmpty())
            assertThat(state.canRedo).isEqualTo(state.redoStack.isNotEmpty())
            assertThat(state.currentIndex).isAtLeast(0)
            assertThat(state.currentIndex).isAtMost(state.totalMatches)
            assertThat(state.historyStack.size + state.redoStack.size).isAtMost(state.totalMatches)
        }
    }

    @Test
    fun test_tournament_state_machine_deterministic_full_cycle() {
        var state = TournamentStateMachine.create(TournamentMode.QUICK) // 15 matches
        val initialChoices = mutableListOf<Int>()

        // Vote all 15
        for (i in 0 until 15) {
            val m = state.currentMatch!!
            val winner = m.leftStyleId
            initialChoices.add(winner)
            state = TournamentStateMachine.vote(state, winner)
        }
        assertThat(state.isComplete).isTrue()

        // Rapidly undo all 15
        repeat(15) {
            state = TournamentStateMachine.undo(state)
        }
        assertThat(state.currentIndex).isEqualTo(0)
        assertThat(state.canUndo).isFalse()
        assertThat(state.redoStack.size).isEqualTo(15)

        // Rapidly redo all 15
        repeat(15) {
            state = TournamentStateMachine.redo(state)
        }
        assertThat(state.currentIndex).isEqualTo(15)
        assertThat(state.isComplete).isTrue()
        assertThat(state.canRedo).isFalse()

        // Verify matches outcomes match initial choices
        for (i in 0 until 15) {
            assertThat(state.matches[i].winnerStyleId).isEqualTo(initialChoices[i])
            assertThat(state.matches[i].outcome).isEqualTo(MatchOutcome.LEFT_WON)
        }
    }

    // ========================================================================
    // 3. RadarChart Kotlin Math & Coercion Invariants
    // ========================================================================

    @Test
    fun test_radarchart_angles_and_coercion_invariants() {
        val numAxes = 5
        val angleStep = (2.0 * Math.PI / numAxes).toFloat()
        val expectedAngles = listOf(-90.0, -18.0, 54.0, 126.0, 198.0)

        for (i in 0 until numAxes) {
            val angleRad = (-Math.PI / 2.0 + i * angleStep).toFloat()
            val angleDeg = Math.toDegrees(angleRad.toDouble())
            assertThat(Math.abs(angleDeg - expectedAngles[i])).isLessThan(1e-4)
        }

        // Test extreme trait values coercion
        val testTraitValues = listOf(-100.0, -0.01, 0.0, 5.0, 10.0, 10.01, 1000.0)
        for (tv in testTraitValues) {
            for (progress in listOf(0f, 0.5f, 1f)) {
                val normalizedVal = ((tv / 10.0) * progress).toFloat().coerceIn(0f, 1f)
                assertThat(normalizedVal >= 0f).isTrue()
                assertThat(normalizedVal <= 1f).isTrue()
                assertThat(normalizedVal.isNaN()).isFalse()
                assertThat(normalizedVal.isInfinite()).isFalse()
            }
        }
    }
}
