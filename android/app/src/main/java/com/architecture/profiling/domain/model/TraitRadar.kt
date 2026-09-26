package com.architecture.profiling.domain.model

import kotlin.math.sqrt

data class TraitRadar(
    val era: Double = 5.0,
    val ornamentation: Double = 5.0,
    val structuralHonesty: Double = 5.0,
    val geometricOrder: Double = 5.0,
    val materialWarmth: Double = 5.0
) {
    operator fun plus(other: TraitRadar): TraitRadar = TraitRadar(
        era = this.era + other.era,
        ornamentation = this.ornamentation + other.ornamentation,
        structuralHonesty = this.structuralHonesty + other.structuralHonesty,
        geometricOrder = this.geometricOrder + other.geometricOrder,
        materialWarmth = this.materialWarmth + other.materialWarmth
    )

    operator fun times(scalar: Double): TraitRadar = TraitRadar(
        era = this.era * scalar,
        ornamentation = this.ornamentation * scalar,
        structuralHonesty = this.structuralHonesty * scalar,
        geometricOrder = this.geometricOrder * scalar,
        materialWarmth = this.materialWarmth * scalar
    )

    fun distanceSquared(other: TraitRadar): Double {
        val d1 = this.era - other.era
        val d2 = this.ornamentation - other.ornamentation
        val d3 = this.structuralHonesty - other.structuralHonesty
        val d4 = this.geometricOrder - other.geometricOrder
        val d5 = this.materialWarmth - other.materialWarmth
        return d1 * d1 + d2 * d2 + d3 * d3 + d4 * d4 + d5 * d5
    }

    fun distance(other: TraitRadar): Double = sqrt(distanceSquared(other))

    fun toDoubleArray(): DoubleArray = doubleArrayOf(
        era, ornamentation, structuralHonesty, geometricOrder, materialWarmth
    )

    companion object {
        val ZERO = TraitRadar(0.0, 0.0, 0.0, 0.0, 0.0)
    }
}
