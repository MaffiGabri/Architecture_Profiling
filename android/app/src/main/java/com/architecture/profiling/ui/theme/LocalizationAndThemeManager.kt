package com.architecture.profiling.ui.theme

import androidx.appcompat.app.AppCompatDelegate
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.core.os.LocaleListCompat
import java.util.Locale

enum class AppLanguage(val code: String, val displayName: String) {
    ENGLISH("en", "English"),
    ITALIAN("it", "Italiano");

    companion object {
        fun fromCode(code: String): AppLanguage =
            entries.find { it.code.equals(code, ignoreCase = true) } ?: ENGLISH
    }
}

val LocalAppLanguage = staticCompositionLocalOf { AppLanguage.ENGLISH }

@Composable
fun ArchitecturalLocalizationProvider(
    language: AppLanguage,
    content: @Composable () -> Unit
) {
    val locale = Locale(language.code)
    Locale.setDefault(locale)

    CompositionLocalProvider(
        LocalAppLanguage provides language,
        content = content
    )
}

fun applyAppLocale(language: AppLanguage) {
    val appLocale = LocaleListCompat.forLanguageTags(language.code)
    AppCompatDelegate.setApplicationLocales(appLocale)
}
