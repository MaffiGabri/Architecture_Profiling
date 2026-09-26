package com.architecture.profiling.domain.model

data class StyleScore(
    val styleId: Int,
    val probability: Double,
    val rank: Int,
    val wins: Int,
    val totalPlayed: Int
) {
    val percentage: Double get() = probability * 100.0
}
