package com.architecture.profiling.ui.state

import androidx.compose.ui.graphics.ImageBitmap
import com.architecture.profiling.data.entity.UserPreferencesEntity
import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TournamentMode
import com.architecture.profiling.domain.model.TournamentResult

sealed interface TournamentUiState {
    data object Loading : TournamentUiState

    data class Error(
        val message: String,
        val cause: Throwable? = null
    ) : TournamentUiState

    data class Welcome(
        val userPreferences: UserPreferencesEntity,
        val selectedMode: TournamentMode = TournamentMode.FULL,
        val availableStyles: List<Style> = emptyList(),
        val totalHistoryCount: Int = 0
    ) : TournamentUiState

    data class InProgress(
        val mode: TournamentMode,
        val matchIndex: Int,          // 0-based index
        val totalMatches: Int,        // 45 (FULL) or 15 (QUICK)
        val roundIndex: Int,          // 0..8
        val leftStyle: Style,
        val rightStyle: Style,
        val leftBitmap: ImageBitmap?,
        val rightBitmap: ImageBitmap?,
        val canUndo: Boolean,
        val canRedo: Boolean,
        val progressPercentage: Double
    ) : TournamentUiState

    data class Finished(
        val result: TournamentResult,
        val mode: TournamentMode,
        val championStyle: Style,
        val dominantCategory: Category,
        val championBitmap: ImageBitmap?,
        val savedRecordId: String? = null,
        val isSaving: Boolean = false
    ) : TournamentUiState
}
