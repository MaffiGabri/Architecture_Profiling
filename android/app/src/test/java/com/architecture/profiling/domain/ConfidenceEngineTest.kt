package com.architecture.profiling.domain

import com.architecture.profiling.domain.engine.ConfidenceEngine
import org.junit.Assert.*
import org.junit.Test

class ConfidenceEngineTest {

    @Test
    fun confidence_clampedTo15MinimumOnZeroConsistencyAndMargin() {
        val conf = ConfidenceEngine.calculateConfidence(zeta = 0.0, victoryMargin = 0.0, entropyConcentration = 0.0)
        assertEquals("Confidence must clamp to 15.0%", 15.0, conf, 1e-6)
    }

    @Test
    fun confidence_clampedTo99MaximumOnHighDecisiveness() {
        val conf = ConfidenceEngine.calculateConfidence(zeta = 1.0, victoryMargin = 1.0, entropyConcentration = 1.0)
        assertEquals("Confidence must clamp to 99.0%", 99.0, conf, 1e-6)
    }

    @Test
    fun confidence_increasesMonotonicallyWithZeta() {
        val lowConf = ConfidenceEngine.calculateConfidence(zeta = 0.2, victoryMargin = 0.05, entropyConcentration = 0.1)
        val highConf = ConfidenceEngine.calculateConfidence(zeta = 0.8, victoryMargin = 0.05, entropyConcentration = 0.1)
        assertTrue("Higher zeta must yield strictly higher confidence", highConf > lowConf)
    }

    @Test
    fun entropyConcentration_boundaryValues() {
        // Uniform distribution: H = ln(10), C_H = 1 - 1 = 0
        val uniform = DoubleArray(10) { 0.10 }
        val uniformH = ConfidenceEngine.calculateEntropy(uniform)
        val uniformCH = ConfidenceEngine.calculateEntropyConcentration(uniformH, 10)
        assertEquals(0.0, uniformCH, 1e-4)

        // Degenerate distribution (all on 1 style): H = 0, C_H = 1 - 0 = 1
        val concentrated = DoubleArray(10) { if (it == 0) 1.0 else 0.0 }
        val concH = ConfidenceEngine.calculateEntropy(concentrated)
        val concCH = ConfidenceEngine.calculateEntropyConcentration(concH, 10)
        assertEquals(1.0, concCH, 1e-4)
    }

    @Test
    fun victoryMargin_handlesSingleItemGracefully() {
        assertEquals(0.0, ConfidenceEngine.calculateVictoryMargin(doubleArrayOf(1.0)), 1e-6)
        assertEquals(0.0, ConfidenceEngine.calculateVictoryMargin(doubleArrayOf()), 1e-6)
    }
}
