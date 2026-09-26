package com.architecture.profiling.domain.model

data class TournamentResult(
    val styleRankings: List<StyleScore>,
    val categoryRankings: List<CategoryScore>,
    val championStyleId: Int,
    val dominantCategoryId: String,
    val circularTriads: Int,
    val consistencyZeta: Double,
    val victoryMargin: Double,
    val entropyConcentration: Double,
    val confidencePercentage: Double,
    val userTraits: TraitRadar,
    val archetype: ArchitecturalArchetype,
    val matchesPlayed: Int,
    val totalMatches: Int
)
