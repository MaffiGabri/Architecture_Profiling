package com.architecture.profiling.ui.navigation

import androidx.compose.animation.AnimatedContentTransitionScope
import androidx.compose.animation.core.tween
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.architecture.profiling.ui.screens.HistoryScreen
import com.architecture.profiling.ui.screens.ResultsScreen
import com.architecture.profiling.ui.screens.TournamentScreen
import com.architecture.profiling.ui.screens.WelcomeScreen
import com.architecture.profiling.ui.state.TournamentUiState
import com.architecture.profiling.ui.theme.ThemeMode
import com.architecture.profiling.ui.viewmodel.HistoryViewModel
import com.architecture.profiling.ui.viewmodel.TournamentViewModel

@Composable
fun AppNavGraph(
    navController: NavHostController,
    tournamentViewModel: TournamentViewModel,
    historyViewModel: HistoryViewModel,
    currentLanguage: String = "system",
    currentTheme: ThemeMode = ThemeMode.SYSTEM,
    onLanguageChange: (String) -> Unit = {},
    onThemeChange: (ThemeMode) -> Unit = {}
) {
    NavHost(
        navController = navController,
        startDestination = Screen.Welcome.route,
        enterTransition = {
            fadeIn(animationSpec = tween(300)) + slideIntoContainer(
                AnimatedContentTransitionScope.SlideDirection.Start,
                tween(300)
            )
        },
        exitTransition = {
            fadeOut(animationSpec = tween(300)) + slideOutOfContainer(
                AnimatedContentTransitionScope.SlideDirection.Start,
                tween(300)
            )
        },
        popEnterTransition = {
            fadeIn(animationSpec = tween(300)) + slideIntoContainer(
                AnimatedContentTransitionScope.SlideDirection.End,
                tween(300)
            )
        },
        popExitTransition = {
            fadeOut(animationSpec = tween(300)) + slideOutOfContainer(
                AnimatedContentTransitionScope.SlideDirection.End,
                tween(300)
            )
        }
    ) {
        // 1. Welcome Screen
        composable(Screen.Welcome.route) {
            val uiState by tournamentViewModel.uiState.collectAsState()
            when (val state = uiState) {
                is TournamentUiState.Loading -> {
                    Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        CircularProgressIndicator()
                    }
                }
                is TournamentUiState.Error -> {
                    Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                        Text(text = state.message)
                    }
                }
                is TournamentUiState.Welcome -> {
                    WelcomeScreen(
                        state = state,
                        onStartTournament = { mode, userName ->
                            tournamentViewModel.setUsername(userName)
                            tournamentViewModel.startTournament(mode)
                            navController.navigate(Screen.Tournament.route)
                        },
                        onViewHistory = {
                            navController.navigate(Screen.History.route)
                        },
                        currentTheme = currentTheme,
                        currentLanguage = currentLanguage,
                        onThemeChange = onThemeChange,
                        onLanguageChange = onLanguageChange
                    )
                }
                is TournamentUiState.InProgress -> {
                    LaunchedEffect(Unit) {
                        navController.navigate(Screen.Tournament.route)
                    }
                }
                is TournamentUiState.Finished -> {
                    LaunchedEffect(Unit) {
                        navController.navigate(Screen.Results.route)
                    }
                }
            }
        }

        // 2. Tournament Screen
        composable(Screen.Tournament.route) {
            val uiState by tournamentViewModel.uiState.collectAsState()
            when (val state = uiState) {
                is TournamentUiState.InProgress -> {
                    TournamentScreen(
                        state = state,
                        viewModel = tournamentViewModel,
                        onExit = {
                            navController.popBackStack()
                        }
                    )
                }
                is TournamentUiState.Finished -> {
                    LaunchedEffect(Unit) {
                        navController.navigate(Screen.Results.route) {
                            popUpTo(Screen.Welcome.route)
                        }
                    }
                }
                else -> {
                    LaunchedEffect(Unit) {
                        navController.navigate(Screen.Welcome.route) {
                            popUpTo(Screen.Welcome.route) { inclusive = true }
                        }
                    }
                }
            }
        }

        // 3. Results Screen
        composable(Screen.Results.route) {
            val uiState by tournamentViewModel.uiState.collectAsState()
            when (val state = uiState) {
                is TournamentUiState.Finished -> {
                    ResultsScreen(
                        state = state,
                        viewModel = tournamentViewModel,
                        onRestart = {
                            tournamentViewModel.reset()
                            navController.navigate(Screen.Welcome.route) {
                                popUpTo(Screen.Welcome.route) { inclusive = true }
                            }
                        },
                        onViewHistory = {
                            navController.navigate(Screen.History.route)
                        },
                        currentLanguage = currentLanguage
                    )
                }
                else -> {
                    LaunchedEffect(Unit) {
                        navController.navigate(Screen.Welcome.route) {
                            popUpTo(Screen.Welcome.route) { inclusive = true }
                        }
                    }
                }
            }
        }

        // 4. History Screen
        composable(Screen.History.route) {
            HistoryScreen(
                viewModel = historyViewModel,
                onBack = { navController.popBackStack() },
                currentLanguage = currentLanguage
            )
        }
    }
}
