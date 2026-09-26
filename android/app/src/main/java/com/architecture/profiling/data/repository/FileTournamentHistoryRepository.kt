package com.architecture.profiling.data.repository

import android.content.Context
import com.architecture.profiling.data.dto.TraitRadarDto
import com.architecture.profiling.data.entity.TournamentHistoryDocumentEntity
import com.architecture.profiling.data.entity.TournamentHistoryRecordEntity
import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TournamentMode
import com.architecture.profiling.domain.model.TournamentResult
import com.architecture.profiling.domain.repository.TournamentHistoryRepository
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import java.io.File
import java.util.UUID

class FileTournamentHistoryRepository(
    private val context: Context,
    private val json: Json = Json { ignoreUnknownKeys = true; prettyPrint = true }
) : TournamentHistoryRepository {

    private val mutex = Mutex()
    private val file = File(context.filesDir, "tournament_history.json")
    private val _historyFlow = MutableStateFlow<List<TournamentHistoryRecordEntity>>(emptyList())
    override val historyFlow: Flow<List<TournamentHistoryRecordEntity>> = _historyFlow.asStateFlow()

    init {
        loadInitial()
    }

    private fun loadInitial() {
        if (file.exists()) {
            try {
                val content = file.readText()
                val doc = json.decodeFromString<TournamentHistoryDocumentEntity>(content)
                _historyFlow.value = doc.records.sortedByDescending { it.timestamp }
            } catch (_: Exception) {
                _historyFlow.value = emptyList()
            }
        }
    }

    override suspend fun getAllResults(): List<TournamentHistoryRecordEntity> = _historyFlow.value

    override suspend fun saveResult(
        result: TournamentResult,
        styles: List<Style>,
        categories: List<Category>,
        mode: TournamentMode
    ): TournamentHistoryRecordEntity = mutex.withLock {
        withContext(Dispatchers.IO) {
            val champion = styles.find { it.id == result.championStyleId }
            val dominantCat = categories.find { it.id == result.dominantCategoryId }

            val record = TournamentHistoryRecordEntity(
                id = UUID.randomUUID().toString(),
                timestamp = System.currentTimeMillis(),
                mode = mode.name,
                championStyleId = result.championStyleId,
                championStyleName = champion?.title ?: "Style #${result.championStyleId}",
                dominantCategoryId = result.dominantCategoryId,
                dominantCategoryName = dominantCat?.nameEn ?: result.dominantCategoryId,
                archetypeId = result.archetype.id,
                archetypeTitleEn = result.archetype.titleEn,
                archetypeTitleIt = result.archetype.titleIt,
                archetypeNarrativeEn = result.archetype.narrativeEn,
                archetypeNarrativeIt = result.archetype.narrativeIt,
                confidencePercentage = result.confidencePercentage,
                consistencyZeta = result.consistencyZeta,
                circularTriads = result.circularTriads,
                userTraits = TraitRadarDto.fromDomain(result.userTraits),
                totalMatches = result.totalMatches,
                matchesPlayed = result.matchesPlayed
            )

            val currentList = _historyFlow.value
            val updatedList = listOf(record) + currentList
            persistAtomic(updatedList)
            _historyFlow.value = updatedList
            record
        }
    }

    override suspend fun getResultById(id: String): TournamentHistoryRecordEntity? {
        return _historyFlow.value.find { it.id == id }
    }

    override suspend fun deleteResult(id: String): Unit = mutex.withLock {
        withContext(Dispatchers.IO) {
            val updatedList = _historyFlow.value.filterNot { it.id == id }
            persistAtomic(updatedList)
            _historyFlow.value = updatedList
        }
    }

    override suspend fun clearHistory(): Unit = mutex.withLock {
        withContext(Dispatchers.IO) {
            persistAtomic(emptyList())
            _historyFlow.value = emptyList()
        }
    }

    private fun persistAtomic(records: List<TournamentHistoryRecordEntity>) {
        val tempFile = File(context.filesDir, "tournament_history.json.tmp")
        val doc = TournamentHistoryDocumentEntity(records)
        tempFile.writeText(json.encodeToString(doc))
        if (!tempFile.renameTo(file)) {
            file.delete()
            tempFile.renameTo(file)
        }
    }
}
