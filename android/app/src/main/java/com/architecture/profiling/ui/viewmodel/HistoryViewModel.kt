package com.architecture.profiling.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import com.architecture.profiling.data.entity.TournamentHistoryRecordEntity
import com.architecture.profiling.domain.repository.TournamentHistoryRepository
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

sealed interface HistoryUiState {
    data object Loading : HistoryUiState
    data class Success(val records: List<TournamentHistoryRecordEntity>) : HistoryUiState
    data object Empty : HistoryUiState
}

class HistoryViewModel(
    private val historyRepository: TournamentHistoryRepository
) : ViewModel() {

    val uiState: StateFlow<HistoryUiState> = historyRepository.historyFlow
        .map { list ->
            if (list.isEmpty()) HistoryUiState.Empty else HistoryUiState.Success(list)
        }
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = HistoryUiState.Loading
        )

    fun deleteRecord(id: String) {
        viewModelScope.launch {
            historyRepository.deleteResult(id)
        }
    }

    fun clearAll() {
        viewModelScope.launch {
            historyRepository.clearHistory()
        }
    }
}

class HistoryViewModelFactory(
    private val historyRepository: TournamentHistoryRepository
) : ViewModelProvider.Factory {
    @Suppress("UNCHECKED_CAST")
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(HistoryViewModel::class.java)) {
            return HistoryViewModel(historyRepository) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
