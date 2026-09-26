package com.architecture.profiling.ui.navigation

sealed class Screen(val route: String) {
    data object Welcome : Screen("welcome")
    data object Tournament : Screen("tournament")
    data object Results : Screen("results")
    data object History : Screen("history")
}
