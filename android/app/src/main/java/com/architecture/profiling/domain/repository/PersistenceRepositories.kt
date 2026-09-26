package com.architecture.profiling.domain.repository

import com.architecture.profiling.data.entity.TournamentHistoryRecordEntity
import com.architecture.profiling.data.entity.UserPreferencesEntity
import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TournamentMode
import com.architecture.profiling.domain.model.TournamentResult
import kotlinx.coroutines.flow.Flow

interface UserPreferencesRepository {
    val preferencesFlow: Flow<UserPreferencesEntity>
    suspend fun getPreferences(): UserPreferencesEntity
    suspend fun updateUsername(name: String)
    suspend fun updateDefaultMode(mode: TournamentMode)
    suspend fun updateLanguage(lang: String)
    suspend fun updateThemeMode(theme: String)
    suspend fun incrementCompletedTournaments()
}

interface TournamentHistoryRepository {
    val historyFlow: Flow<List<TournamentHistoryRecordEntity>>
    suspend fun getAllResults(): List<TournamentHistoryRecordEntity>
    suspend fun saveResult(
        result: TournamentResult,
        styles: List<Style>,
        categories: List<Category>,
        mode: TournamentMode
    ): TournamentHistoryRecordEntity
    suspend fun getResultById(id: String): TournamentHistoryRecordEntity?
    suspend fun deleteResult(id: String)
    suspend fun clearHistory()
}
