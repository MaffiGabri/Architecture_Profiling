package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.PairingScheduler
import com.architecture.profiling.domain.model.TournamentMode
import org.junit.Assert.*
import org.junit.Test
import kotlin.math.max
import kotlin.math.min

class PairingSchedulerTest {

    @Test
    fun fullTournament_has45Matches() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        assertEquals(45, schedule.size)
    }

    @Test
    fun quickTournament_has15Matches() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.QUICK)
        assertEquals(15, schedule.size)
    }

    @Test
    fun fullTournament_all45PairsAreDistinctAndValid() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val uniquePairs = schedule.map { min(it.leftStyleId, it.rightStyleId) to max(it.leftStyleId, it.rightStyleId) }.toSet()
        assertEquals("Must contain all 45 distinct pairs of K_10", 45, uniquePairs.size)

        for (m in schedule) {
            assertTrue("Left style ID must be in 1..10", m.leftStyleId in 1..10)
            assertTrue("Right style ID must be in 1..10", m.rightStyleId in 1..10)
            assertNotEquals("Style cannot be paired against itself", m.leftStyleId, m.rightStyleId)
        }
    }

    @Test
    fun schedule_enforcesMinimumSpacingOfAtLeast4Matches() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)

        for (item in 1..10) {
            val appearanceIndices = schedule.mapIndexedNotNull { idx, m ->
                if (m.leftStyleId == item || m.rightStyleId == item) idx else null
            }
            assertEquals("Each item must appear exactly 9 times in 45 matches", 9, appearanceIndices.size)

            for (k in 0 until appearanceIndices.size - 1) {
                val gap = appearanceIndices[k + 1] - appearanceIndices[k]
                assertTrue(
                    "Violation for item $item: gap between match ${appearanceIndices[k]} and ${appearanceIndices[k+1]} is $gap (< 4)",
                    gap >= 4
                )
            }
        }
    }

    @Test
    fun schedule_maintainsLeftRightPresentationBalance() {
        val schedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val leftCounts = IntArray(11)
        val rightCounts = IntArray(11)

        for (m in schedule) {
            leftCounts[m.leftStyleId]++
            rightCounts[m.rightStyleId]++
        }

        for (item in 1..10) {
            assertTrue(
                "Item $item left count (${leftCounts[item]}) must be 4 or 5",
                leftCounts[item] in 4..5
            )
            assertTrue(
                "Item $item right count (${rightCounts[item]}) must be 4 or 5",
                rightCounts[item] in 4..5
            )
            assertEquals("Total appearances must be 9", 9, leftCounts[item] + rightCounts[item])
        }
    }

    @Test
    fun quickSchedule_matchesPrefixOfFullSchedule() {
        val fullSchedule = PairingScheduler.generateSchedule(TournamentMode.FULL)
        val quickSchedule = PairingScheduler.generateSchedule(TournamentMode.QUICK)

        assertEquals(15, quickSchedule.size)
        for (i in 0 until 15) {
            assertEquals(fullSchedule[i].leftStyleId, quickSchedule[i].leftStyleId)
            assertEquals(fullSchedule[i].rightStyleId, quickSchedule[i].rightStyleId)
            assertEquals(fullSchedule[i].roundIndex, quickSchedule[i].roundIndex)
        }
    }
}
