package com.architecture.profiling

import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.rememberNavController
import com.architecture.profiling.data.image.AssetImageLoader
import com.architecture.profiling.data.repository.AssetStyleRepository
import com.architecture.profiling.data.repository.FileTournamentHistoryRepository
import com.architecture.profiling.data.repository.FileUserPreferencesRepository
import com.architecture.profiling.ui.navigation.AppNavGraph
import com.architecture.profiling.ui.theme.AppLanguage
import com.architecture.profiling.ui.theme.ArchitectureProfilingTheme
import com.architecture.profiling.ui.theme.ArchitecturalLocalizationProvider
import com.architecture.profiling.ui.theme.ThemeMode
import com.architecture.profiling.ui.theme.applyAppLocale
import com.architecture.profiling.ui.viewmodel.HistoryViewModel
import com.architecture.profiling.ui.viewmodel.HistoryViewModelFactory
import com.architecture.profiling.ui.viewmodel.TournamentViewModel
import com.architecture.profiling.ui.viewmodel.TournamentViewModelFactory

class MainActivity : AppCompatActivity() {

    private lateinit var styleRepository: AssetStyleRepository
    private lateinit var imageLoader: AssetImageLoader
    private lateinit var historyRepository: FileTournamentHistoryRepository
    private lateinit var preferencesRepository: FileUserPreferencesRepository

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        // Initialize data repositories and loaders
        styleRepository = AssetStyleRepository(applicationContext)
        imageLoader = AssetImageLoader(applicationContext)
        historyRepository = FileTournamentHistoryRepository(applicationContext)
        preferencesRepository = FileUserPreferencesRepository(applicationContext)

        setContent {
            val userPrefs by preferencesRepository.preferencesFlow.collectAsState(
                initial = com.architecture.profiling.data.entity.UserPreferencesEntity()
            )

            val currentThemeMode = when (userPrefs.themeMode) {
                "LIGHT" -> ThemeMode.LIGHT
                "DARK" -> ThemeMode.DARK
                else -> ThemeMode.SYSTEM
            }

            val currentAppLanguage = when (userPrefs.preferredLanguage) {
                "it" -> AppLanguage.ITALIAN
                else -> AppLanguage.ENGLISH
            }

            ArchitecturalLocalizationProvider(language = currentAppLanguage) {
                ArchitectureProfilingTheme(themeMode = currentThemeMode) {
                    Surface(
                        modifier = Modifier.fillMaxSize(),
                        color = MaterialTheme.colorScheme.background
                    ) {
                        val navController = rememberNavController()

                        val tournamentViewModel: TournamentViewModel = viewModel(
                            factory = TournamentViewModelFactory(
                                styleRepository = styleRepository,
                                imageLoader = imageLoader,
                                historyRepository = historyRepository,
                                preferencesRepository = preferencesRepository
                            )
                        )

                        val historyViewModel: HistoryViewModel = viewModel(
                            factory = HistoryViewModelFactory(
                                historyRepository = historyRepository
                            )
                        )

                        AppNavGraph(
                            navController = navController,
                            tournamentViewModel = tournamentViewModel,
                            historyViewModel = historyViewModel,
                            currentLanguage = userPrefs.preferredLanguage,
                            currentTheme = currentThemeMode,
                            onLanguageChange = { lang ->
                                tournamentViewModel.setLanguage(lang)
                                applyAppLocale(AppLanguage.fromCode(lang))
                            },
                            onThemeChange = { theme ->
                                tournamentViewModel.setTheme(theme)
                            }
                        )
                    }
                }
            }
        }
    }
}
