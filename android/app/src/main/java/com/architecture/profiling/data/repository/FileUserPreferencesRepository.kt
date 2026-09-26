package com.architecture.profiling.data.repository

import android.content.Context
import com.architecture.profiling.data.entity.UserPreferencesEntity
import com.architecture.profiling.domain.model.TournamentMode
import com.architecture.profiling.domain.repository.UserPreferencesRepository
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

class FileUserPreferencesRepository(
    private val context: Context,
    private val json: Json = Json { ignoreUnknownKeys = true; prettyPrint = true }
) : UserPreferencesRepository {

    private val mutex = Mutex()
    private val file = File(context.filesDir, "user_preferences.json")
    private val _preferencesFlow = MutableStateFlow(UserPreferencesEntity())
    override val preferencesFlow: Flow<UserPreferencesEntity> = _preferencesFlow.asStateFlow()

    init {
        loadInitial()
    }

    private fun loadInitial() {
        if (file.exists()) {
            try {
                val content = file.readText()
                val parsed = json.decodeFromString<UserPreferencesEntity>(content)
                _preferencesFlow.value = parsed
            } catch (_: Exception) {
                // Fallback to default
            }
        }
    }

    override suspend fun getPreferences(): UserPreferencesEntity = _preferencesFlow.value

    override suspend fun updateUsername(name: String) = update { it.copy(username = name) }
    override suspend fun updateDefaultMode(mode: TournamentMode) = update { it.copy(defaultMode = mode.name) }
    override suspend fun updateLanguage(lang: String) = update { it.copy(preferredLanguage = lang) }
    override suspend fun updateThemeMode(theme: String) = update { it.copy(themeMode = theme) }
    override suspend fun incrementCompletedTournaments() = update {
        it.copy(totalTournamentsCompleted = it.totalTournamentsCompleted + 1)
    }

    private suspend fun update(transform: (UserPreferencesEntity) -> UserPreferencesEntity) {
        mutex.withLock {
            withContext(Dispatchers.IO) {
                val updated = transform(_preferencesFlow.value)
                val tempFile = File(context.filesDir, "user_preferences.json.tmp")
                tempFile.writeText(json.encodeToString(updated))
                if (tempFile.renameTo(file) || (file.delete() && tempFile.renameTo(file))) {
                    _preferencesFlow.value = updated
                }
            }
        }
    }
}
