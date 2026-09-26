package com.architecture.profiling.data.entity

import com.architecture.profiling.data.dto.TraitRadarDto
import com.architecture.profiling.domain.model.TournamentMode
import kotlinx.serialization.Serializable

@Serializable
data class UserPreferencesEntity(
    val username: String = "Architectural Explorer",
    val defaultMode: String = TournamentMode.FULL.name,
    val preferredLanguage: String = "system", // "en", "it", "system"
    val themeMode: String = "SYSTEM",         // "LIGHT", "DARK", "SYSTEM"
    val totalTournamentsCompleted: Int = 0
)

@Serializable
data class TournamentHistoryRecordEntity(
    val id: String,                          // UUID
    val timestamp: Long,                     // Epoch milliseconds
    val mode: String,                        // "FULL" or "QUICK"
    val championStyleId: Int,
    val championStyleName: String,
    val dominantCategoryId: String,
    val dominantCategoryName: String,
    val archetypeId: String,
    val archetypeTitleEn: String,
    val archetypeTitleIt: String,
    val archetypeNarrativeEn: String = "",
    val archetypeNarrativeIt: String = "",
    val confidencePercentage: Double,
    val consistencyZeta: Double,
    val circularTriads: Int,
    val userTraits: TraitRadarDto,
    val totalMatches: Int,
    val matchesPlayed: Int
)

@Serializable
data class TournamentHistoryDocumentEntity(
    val records: List<TournamentHistoryRecordEntity> = emptyList()
)
