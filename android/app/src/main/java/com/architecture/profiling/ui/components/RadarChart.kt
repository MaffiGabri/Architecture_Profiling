package com.architecture.profiling.ui.components

import androidx.compose.animation.core.Animatable
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.drawText
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.rememberTextMeasurer
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.architecture.profiling.domain.model.TraitRadar
import java.util.Locale
import kotlin.math.cos
import kotlin.math.min
import kotlin.math.sin

private val LABELS_EN = listOf(
    "Era",
    "Ornamentation",
    "Structural Honesty",
    "Geometric Order",
    "Material Warmth"
)

private val LABELS_IT = listOf(
    "Epoca",
    "Ornamentazione",
    "Onestà Strutturale",
    "Ordine Geometrico",
    "Calore dei Materiali"
)

/**
 * 5-Axis Spider/Radar Chart implemented entirely using Jetpack Compose Canvas.
 * No third-party charting libraries are used.
 *
 * Axes:
 * 0: Era
 * 1: Ornamentation
 * 2: Structural Honesty
 * 3: Geometric Order
 * 4: Material Warmth
 */
@Composable
fun RadarChart(
    traits: TraitRadar,
    isItalian: Boolean = false,
    modifier: Modifier = Modifier
        .fillMaxWidth()
        .height(280.dp),
    accentColor: Color = MaterialTheme.colorScheme.primary,
    gridColor: Color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.5f),
    labelColor: Color = MaterialTheme.colorScheme.onSurface
) {
    val progress = remember { Animatable(0f) }
    LaunchedEffect(traits) {
        progress.snapTo(0f)
        progress.animateTo(
            targetValue = 1f,
            animationSpec = tween(durationMillis = 800, easing = FastOutSlowInEasing)
        )
    }

    val textMeasurer = rememberTextMeasurer()
    val traitValues: DoubleArray = remember(traits) { traits.toDoubleArray() }
    val axisLabels: List<String> = if (isItalian) LABELS_IT else LABELS_EN

    Canvas(modifier = modifier) {
        val center = Offset(size.width / 2f, size.height / 2f)
        val radius = (min(size.width, size.height) / 2f) - 52.dp.toPx()
        val numAxes = 5
        val angleStep = (2.0 * Math.PI / numAxes).toFloat()

        // 1. Draw Concentric Web Grids (Levels: 2, 4, 6, 8, 10 out of 10)
        val levels = listOf(0.2f, 0.4f, 0.6f, 0.8f, 1.0f)
        for (lvl in levels) {
            val gridPath = Path()
            val currentRadius = radius * lvl
            for (i in 0 until numAxes) {
                val angle = (-Math.PI / 2.0 + i * angleStep).toFloat()
                val x = center.x + currentRadius * cos(angle)
                val y = center.y + currentRadius * sin(angle)
                if (i == 0) gridPath.moveTo(x, y) else gridPath.lineTo(x, y)
            }
            gridPath.close()
            drawPath(
                path = gridPath,
                color = gridColor,
                style = Stroke(width = if (lvl == 1.0f) 1.5.dp.toPx() else 1.dp.toPx())
            )
        }

        // 2. Draw Radial Spokes and Axis Labels
        for (i in 0 until numAxes) {
            val angle = (-Math.PI / 2.0 + i * angleStep).toFloat()
            val spokeEnd = Offset(
                x = center.x + radius * cos(angle),
                y = center.y + radius * sin(angle)
            )
            drawLine(
                color = gridColor,
                start = center,
                end = spokeEnd,
                strokeWidth = 1.dp.toPx()
            )

            // Text Label placement
            if (i < axisLabels.size) {
                val labelDistance = radius + 24.dp.toPx()
                val labelX = center.x + labelDistance * cos(angle)
                val labelY = center.y + labelDistance * sin(angle)

                val labelText = "${axisLabels[i]}\n${String.format(Locale.US, "%.1f", traitValues[i])}"
                val textLayout = textMeasurer.measure(
                    text = labelText,
                    style = TextStyle(
                        fontSize = 10.sp,
                        color = labelColor,
                        fontWeight = FontWeight.SemiBold,
                        textAlign = androidx.compose.ui.text.style.TextAlign.Center
                    )
                )

                drawText(
                    textLayoutResult = textLayout,
                    topLeft = Offset(
                        labelX - (textLayout.size.width / 2f),
                        labelY - (textLayout.size.height / 2f)
                    )
                )
            }
        }

        // 3. Draw User's Trait Polygon (with smooth animation)
        val polygonPath = Path()
        val vertexPoints = mutableListOf<Offset>()

        for (i in 0 until numAxes) {
            val angle = (-Math.PI / 2.0 + i * angleStep).toFloat()
            val normalizedVal = ((traitValues[i] / 10.0) * progress.value).toFloat().coerceIn(0f, 1f)
            val currentRadius = radius * normalizedVal
            val x = center.x + currentRadius * cos(angle)
            val y = center.y + currentRadius * sin(angle)
            val pt = Offset(x, y)
            vertexPoints.add(pt)

            if (i == 0) polygonPath.moveTo(x, y) else polygonPath.lineTo(x, y)
        }
        polygonPath.close()

        // Fill with translucent primary accent color
        drawPath(
            path = polygonPath,
            color = accentColor.copy(alpha = 0.30f)
        )

        // Draw outline of the polygon
        drawPath(
            path = polygonPath,
            color = accentColor,
            style = Stroke(width = 2.5.dp.toPx(), cap = StrokeCap.Round)
        )

        // Draw points at each vertex
        for (pt in vertexPoints) {
            drawCircle(
                color = Color.White,
                radius = 4.5.dp.toPx(),
                center = pt
            )
            drawCircle(
                color = accentColor,
                radius = 3.dp.toPx(),
                center = pt
            )
        }
    }
}
