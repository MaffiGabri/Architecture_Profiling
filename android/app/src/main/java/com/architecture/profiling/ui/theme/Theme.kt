package com.architecture.profiling.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.core.view.WindowCompat

val ArchitecturalLightColorScheme = lightColorScheme(
    primary = ClassicalGoldPrimaryLight,
    onPrimary = OnGoldLight,
    primaryContainer = WarmSandstoneContainerLight,
    onPrimaryContainer = OnWarmSandstoneLight,
    secondary = SlateSteelSecondaryLight,
    onSecondary = OnSlateSteelLight,
    secondaryContainer = SlateSteelContainerLight,
    onSecondaryContainer = OnSlateSteelContainerLight,
    tertiary = TuscanTerracottaLight,
    onTertiary = OnTuscanTerracottaLight,
    tertiaryContainer = TuscanTerracottaContainerLight,
    onTertiaryContainer = OnTuscanTerracottaContainerLight,
    background = TravertineLightBg,
    onBackground = StoneEngravingInk,
    surface = CarraraSurfaceLight,
    onSurface = StoneEngravingInk,
    surfaceVariant = PietraSerenaVariantLight,
    onSurfaceVariant = SlateSteelSecondaryLight,
    outline = ChiseledStoneOutlineLight,
    outlineVariant = SoftMortarOutlineLight
)

val ArchitecturalDarkColorScheme = darkColorScheme(
    primary = RadiantGoldPrimaryDark,
    onPrimary = OnGoldDark,
    primaryContainer = BurnishedBronzeContainerDark,
    onPrimaryContainer = OnBurnishedBronzeDark,
    secondary = TitaniumBlueSecondaryDark,
    onSecondary = OnTitaniumBlueDark,
    secondaryContainer = TitaniumBlueContainerDark,
    onSecondaryContainer = OnTitaniumBlueContainerDark,
    tertiary = TuscanCottoGlowDark,
    onTertiary = OnTuscanCottoDark,
    tertiaryContainer = TuscanCottoContainerDark,
    onTertiaryContainer = OnTuscanCottoContainerDark,
    background = DeepBasaltDarkBg,
    onBackground = PolishedMarbleTextDark,
    surface = DarkSlateSurfaceDark,
    onSurface = PolishedMarbleTextDark,
    surfaceVariant = BasaltPedestalVariantDark,
    onSurfaceVariant = TitaniumBlueSecondaryDark,
    outline = BasaltJointOutlineDark,
    outlineVariant = DeepJointOutlineDark
)

enum class ThemeMode {
    SYSTEM,
    LIGHT,
    DARK
}

@Composable
fun ArchitectureProfilingTheme(
    themeMode: ThemeMode = ThemeMode.SYSTEM,
    dynamicColor: Boolean = false,
    content: @Composable () -> Unit
) {
    val darkTheme = when (themeMode) {
        ThemeMode.SYSTEM -> isSystemInDarkTheme()
        ThemeMode.LIGHT -> false
        ThemeMode.DARK -> true
    }

    val colorScheme = when {
        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
            val context = LocalContext.current
            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
        }
        darkTheme -> ArchitecturalDarkColorScheme
        else -> ArchitecturalLightColorScheme
    }

    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            val window = (view.context as? Activity)?.window
            if (window != null) {
                window.statusBarColor = colorScheme.background.toArgb()
                window.navigationBarColor = colorScheme.background.toArgb()
                val insetsController = WindowCompat.getInsetsController(window, view)
                insetsController.isAppearanceLightStatusBars = !darkTheme
                insetsController.isAppearanceLightNavigationBars = !darkTheme
            }
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = ArchitecturalTypography,
        content = content
    )
}
