package com.architecture.profiling.data

import com.architecture.profiling.data.dto.TraitRadarDto
import com.architecture.profiling.data.entity.TournamentHistoryDocumentEntity
import com.architecture.profiling.data.entity.TournamentHistoryRecordEntity
import com.architecture.profiling.data.entity.UserPreferencesEntity
import com.google.common.truth.Truth.assertThat
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import org.junit.Test

class PersistenceEntitiesTest {

    private val json = Json { ignoreUnknownKeys = true; prettyPrint = true }

    @Test
    fun test_user_preferences_serialization_roundtrip() {
        val original = UserPreferencesEntity(
            username = "TestUser",
            defaultMode = "QUICK",
            preferredLanguage = "it",
            themeMode = "DARK",
            totalTournamentsCompleted = 5
        )

        val encoded = json.encodeToString(original)
        val decoded = json.decodeFromString<UserPreferencesEntity>(encoded)

        assertThat(decoded).isEqualTo(original)
    }

    @Test
    fun test_tournament_history_document_serialization_roundtrip() {
        val record = TournamentHistoryRecordEntity(
            id = "test-uuid-1234",
            timestamp = 1695168000000L,
            mode = "FULL",
            championStyleId = 1,
            championStyleName = "The Parthenon Colonnade",
            dominantCategoryId = "classical_renaissance",
            dominantCategoryName = "Classical & Renaissance",
            archetypeId = "classical_monumentalist",
            archetypeTitleEn = "The Classical Monumentalist",
            archetypeTitleIt = "Il Monumentalista Classico",
            archetypeNarrativeEn = "You find sanctuary...",
            archetypeNarrativeIt = "Trovi rifugio...",
            confidencePercentage = 88.5,
            consistencyZeta = 0.95,
            circularTriads = 2,
            userTraits = TraitRadarDto(era = 2.0, ornamentation = 7.0, structuralHonesty = 6.0, geometricOrder = 9.0, materialWarmth = 5.0),
            totalMatches = 45,
            matchesPlayed = 45
        )

        val doc = TournamentHistoryDocumentEntity(records = listOf(record))
        val encoded = json.encodeToString(doc)
        val decoded = json.decodeFromString<TournamentHistoryDocumentEntity>(encoded)

        assertThat(decoded.records).hasSize(1)
        assertThat(decoded.records[0].id).isEqualTo("test-uuid-1234")
        assertThat(decoded.records[0].confidencePercentage).isEqualTo(88.5)
        assertThat(decoded.records[0].userTraits.toDomain().geometricOrder).isEqualTo(9.0)
    }
}
