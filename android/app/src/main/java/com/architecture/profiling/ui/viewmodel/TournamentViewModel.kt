package com.architecture.profiling.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import com.architecture.profiling.data.image.AssetImageLoader
import com.architecture.profiling.data.repository.StyleRepository
import com.architecture.profiling.domain.engine.TournamentStateMachine
import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TournamentMode
import com.architecture.profiling.domain.model.TournamentResult
import com.architecture.profiling.domain.model.TournamentState
import com.architecture.profiling.domain.repository.TournamentHistoryRepository
import com.architecture.profiling.domain.repository.UserPreferencesRepository
import com.architecture.profiling.ui.state.TournamentUiState
import com.architecture.profiling.ui.theme.ThemeMode
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class TournamentViewModel(
    private val styleRepository: StyleRepository,
    private val imageLoader: AssetImageLoader,
    private val historyRepository: TournamentHistoryRepository,
    private val preferencesRepository: UserPreferencesRepository
) : ViewModel() {

    private val _uiState = MutableStateFlow<TournamentUiState>(TournamentUiState.Loading)
    val uiState: StateFlow<TournamentUiState> = _uiState.asStateFlow()

    private var currentTournamentState: TournamentState? = null
    private var allStyles: List<Style> = emptyList()
    private var allCategories: List<Category> = emptyList()
    private var selectedMode: TournamentMode = TournamentMode.FULL
    private var lastResult: TournamentResult? = null

    init {
        initializeData()
    }

    private fun initializeData() {
        viewModelScope.launch {
            try {
                _uiState.value = TournamentUiState.Loading
                allStyles = styleRepository.getStyles()
                allCategories = styleRepository.getCategories()

                // Warm up image cache in the background
                imageLoader.preloadImages(allStyles.map { it.filename })

                val prefs = preferencesRepository.getPreferences()
                selectedMode = try {
                    TournamentMode.valueOf(prefs.defaultMode)
                } catch (_: Exception) {
                    TournamentMode.FULL
                }

                val historyList = historyRepository.getAllResults()

                _uiState.value = TournamentUiState.Welcome(
                    userPreferences = prefs,
                    selectedMode = selectedMode,
                    availableStyles = allStyles,
                    totalHistoryCount = historyList.size
                )
            } catch (e: Exception) {
                _uiState.value = TournamentUiState.Error(
                    message = "Failed to load architecture assets: ${e.localizedMessage}",
                    cause = e
                )
            }
        }
    }

    fun setUsername(name: String) {
        viewModelScope.launch {
            preferencesRepository.updateUsername(name)
        }
    }

    fun setMode(mode: TournamentMode) {
        selectedMode = mode
        viewModelScope.launch {
            preferencesRepository.updateDefaultMode(mode)
        }
        val currentState = _uiState.value
        if (currentState is TournamentUiState.Welcome) {
            _uiState.value = currentState.copy(selectedMode = mode)
        }
    }

    fun setLanguage(lang: String) {
        viewModelScope.launch {
            preferencesRepository.updateLanguage(lang)
        }
    }

    fun setTheme(theme: ThemeMode) {
        viewModelScope.launch {
            preferencesRepository.updateThemeMode(theme.name)
        }
    }

    fun startTournament(mode: TournamentMode = selectedMode) {
        selectedMode = mode
        val state = TournamentStateMachine.create(mode)
        currentTournamentState = state
        emitMatchState(state)
    }

    fun vote(winnerStyleId: Int) {
        val state = currentTournamentState ?: return
        val nextState = TournamentStateMachine.vote(state, winnerStyleId)
        currentTournamentState = nextState

        if (nextState.isComplete) {
            onTournamentComplete(nextState)
        } else {
            emitMatchState(nextState)
        }
    }

    fun undo() {
        val state = currentTournamentState ?: return
        if (!state.canUndo) return
        val prevState = TournamentStateMachine.undo(state)
        currentTournamentState = prevState
        emitMatchState(prevState)
    }

    fun redo() {
        val state = currentTournamentState ?: return
        if (!state.canRedo) return
        val nextState = TournamentStateMachine.redo(state)
        currentTournamentState = nextState
        emitMatchState(nextState)
    }

    fun reset() {
        currentTournamentState = null
        lastResult = null
        initializeData()
    }

    fun getStyle(id: Int): Style? = allStyles.find { it.id == id }

    fun getCategory(id: String): Category? = allCategories.find { it.id == id }

    val currentResult: TournamentResult?
        get() = lastResult

    private fun emitMatchState(state: TournamentState) {
        val match = state.currentMatch ?: return
        val left = allStyles.find { it.id == match.leftStyleId } ?: return
        val right = allStyles.find { it.id == match.rightStyleId } ?: return

        val leftBmp = imageLoader.getCached(left.filename)
        val rightBmp = imageLoader.getCached(right.filename)

        _uiState.value = TournamentUiState.InProgress(
            mode = state.mode,
            matchIndex = state.currentIndex,
            totalMatches = state.totalMatches,
            roundIndex = match.roundIndex,
            leftStyle = left,
            rightStyle = right,
            leftBitmap = leftBmp,
            rightBitmap = rightBmp,
            canUndo = state.canUndo,
            canRedo = state.canRedo,
            progressPercentage = state.progressPercentage
        )

        // Asynchronously load any missing bitmaps
        if (leftBmp == null || rightBmp == null) {
            viewModelScope.launch {
                val fetchedLeft = leftBmp ?: imageLoader.loadBitmap(left.filename)
                val fetchedRight = rightBmp ?: imageLoader.loadBitmap(right.filename)
                val current = _uiState.value
                if (current is TournamentUiState.InProgress && current.matchIndex == state.currentIndex) {
                    _uiState.value = current.copy(
                        leftBitmap = fetchedLeft,
                        rightBitmap = fetchedRight
                    )
                }
            }
        }
    }

    private fun onTournamentComplete(completedState: TournamentState) {
        viewModelScope.launch {
            val result = withContext(Dispatchers.Default) {
                TournamentStateMachine.generateResult(completedState, allStyles, allCategories)
            }
            lastResult = result
            val champion = allStyles.find { it.id == result.championStyleId } ?: allStyles.first()
            val dominantCat = allCategories.find { it.id == result.dominantCategoryId } ?: allCategories.first()
            val champBmp = imageLoader.getCached(champion.filename) ?: imageLoader.loadBitmap(champion.filename)

            _uiState.value = TournamentUiState.Finished(
                result = result,
                mode = completedState.mode,
                championStyle = champion,
                dominantCategory = dominantCat,
                championBitmap = champBmp,
                isSaving = true
            )

            // Auto-save to persistence repository
            try {
                val savedRecord = historyRepository.saveResult(result, allStyles, allCategories, completedState.mode)
                preferencesRepository.incrementCompletedTournaments()

                val finishState = _uiState.value
                if (finishState is TournamentUiState.Finished) {
                    _uiState.value = finishState.copy(
                        savedRecordId = savedRecord.id,
                        isSaving = false
                    )
                }
            } catch (e: Exception) {
                val finishState = _uiState.value
                if (finishState is TournamentUiState.Finished) {
                    _uiState.value = finishState.copy(isSaving = false)
                }
            }
        }
    }
}

class TournamentViewModelFactory(
    private val styleRepository: StyleRepository,
    private val imageLoader: AssetImageLoader,
    private val historyRepository: TournamentHistoryRepository,
    private val preferencesRepository: UserPreferencesRepository
) : ViewModelProvider.Factory {
    @Suppress("UNCHECKED_CAST")
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(TournamentViewModel::class.java)) {
            return TournamentViewModel(
                styleRepository,
                imageLoader,
                historyRepository,
                preferencesRepository
            ) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
