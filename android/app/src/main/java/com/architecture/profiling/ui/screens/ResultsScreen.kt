package com.architecture.profiling.ui.screens

import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.architecture.profiling.R
import com.architecture.profiling.domain.model.ArchitecturalArchetype
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.StyleScore
import com.architecture.profiling.ui.components.ConfidenceBadge
import com.architecture.profiling.ui.components.ConfidenceExplanationDialog
import com.architecture.profiling.ui.components.RadarChart
import com.architecture.profiling.ui.state.TournamentUiState
import com.architecture.profiling.ui.viewmodel.TournamentViewModel
import java.util.Locale

fun Style.localizedName(isItalian: Boolean): String = if (isItalian) styleIt else styleEn
fun ArchitecturalArchetype.localizedTitle(isItalian: Boolean): String = if (isItalian) titleIt else titleEn
fun ArchitecturalArchetype.localizedTagline(isItalian: Boolean): String = if (isItalian) taglineIt else taglineEn
fun ArchitecturalArchetype.localizedNarrative(isItalian: Boolean): String = if (isItalian) narrativeIt else narrativeEn
val StyleScore.losses: Int get() = totalPlayed - wins

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ResultsScreen(
    state: TournamentUiState.Finished,
    viewModel: TournamentViewModel,
    onRestart: () -> Unit,
    onViewHistory: () -> Unit,
    currentLanguage: String = "system"
) {
    val result = state.result
    val scrollState = rememberScrollState()
    val isItalian = currentLanguage == "it" || (currentLanguage == "system" && Locale.getDefault().language == "it")
    var showConfidenceDialog by remember { mutableStateOf(false) }

    if (showConfidenceDialog) {
        ConfidenceExplanationDialog(
            onDismiss = { showConfidenceDialog = false }
        )
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text(stringResource(R.string.results_title), fontWeight = FontWeight.Bold) },
                actions = {
                    IconButton(onClick = { showConfidenceDialog = true }) {
                        Icon(
                            imageVector = Icons.Default.Info,
                            contentDescription = stringResource(R.string.dialog_help_confidence_title)
                        )
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .verticalScroll(scrollState)
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            // 1. Confidence % & Consistency Badge
            ConfidenceBadge(
                confidencePercentage = result.confidencePercentage,
                consistencyZeta = result.consistencyZeta,
                circularTriads = result.circularTriads,
                victoryMargin = result.victoryMargin,
                onInfoClick = { showConfidenceDialog = true }
            )

            // 2. #1 Champion Banner Card
            ElevatedCard(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp),
                colors = CardDefaults.elevatedCardColors(containerColor = MaterialTheme.colorScheme.primaryContainer)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = "\uD83C\uDFC6 ${stringResource(R.string.results_champion_label)}",
                        style = MaterialTheme.typography.labelMedium,
                        fontWeight = FontWeight.Black,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.height(4.dp))
                    Text(
                        text = state.championStyle.title,
                        style = MaterialTheme.typography.titleLarge,
                        fontWeight = FontWeight.Bold,
                        color = MaterialTheme.colorScheme.onPrimaryContainer
                    )
                    Text(
                        text = "${state.championStyle.localizedName(isItalian)} \u2022 ${state.championStyle.eraCentury}",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onPrimaryContainer.copy(alpha = 0.8f)
                    )

                    if (state.championBitmap != null) {
                        Spacer(modifier = Modifier.height(12.dp))
                        Image(
                            bitmap = state.championBitmap,
                            contentDescription = state.championStyle.title,
                            modifier = Modifier
                                .fillMaxWidth()
                                .height(160.dp)
                                .clip(RoundedCornerShape(10.dp)),
                            contentScale = ContentScale.Crop
                        )
                    }
                }
            }

            // 3. Personality Archetype Card (The 110% Feature)
            ElevatedCard(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = "PERSONALITY ARCHETYPE",
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.primary,
                        fontWeight = FontWeight.Bold
                    )
                    Spacer(modifier = Modifier.height(4.dp))
                    Text(
                        text = result.archetype.localizedTitle(isItalian),
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold
                    )
                    Spacer(modifier = Modifier.height(4.dp))
                    Text(
                        text = "\"${result.archetype.localizedTagline(isItalian)}\"",
                        style = MaterialTheme.typography.bodyMedium,
                        fontStyle = FontStyle.Italic,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = result.archetype.localizedNarrative(isItalian),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                }
            }

            // 4. Custom 5-Axis Radar Chart
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp)
            ) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Text(
                        text = stringResource(R.string.results_radar_header),
                        style = MaterialTheme.typography.titleSmall,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.fillMaxWidth()
                    )
                    Spacer(modifier = Modifier.height(12.dp))
                    RadarChart(
                        traits = result.userTraits,
                        isItalian = isItalian,
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(280.dp)
                    )
                }
            }

            // 5. Category Affinity Breakdown
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = stringResource(R.string.results_category_breakdown_header),
                        style = MaterialTheme.typography.titleSmall,
                        fontWeight = FontWeight.Bold
                    )
                    Spacer(modifier = Modifier.height(10.dp))
                    result.categoryRankings.forEach { catScore ->
                        val category = viewModel.getCategory(catScore.categoryId)
                        val catColor = remember(category?.colorHex) {
                            try { Color(android.graphics.Color.parseColor(category?.colorHex ?: "#4A6984")) }
                            catch (_: Exception) { Color(0xFF4A6984) }
                        }
                        Column(modifier = Modifier.padding(vertical = 4.dp)) {
                            Row(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalArrangement = Arrangement.SpaceBetween
                            ) {
                                Text(
                                    text = if (isItalian) category?.nameIt ?: catScore.categoryId else category?.nameEn ?: catScore.categoryId,
                                    style = MaterialTheme.typography.bodySmall,
                                    fontWeight = FontWeight.Medium
                                )
                                Text(
                                    text = "${String.format(Locale.US, "%.1f", catScore.percentage)}%",
                                    style = MaterialTheme.typography.bodySmall,
                                    fontWeight = FontWeight.Bold
                                )
                            }
                            Spacer(modifier = Modifier.height(3.dp))
                            val animatedCatProgress by animateFloatAsState(
                                targetValue = (catScore.percentage / 100.0).toFloat().coerceIn(0f, 1f),
                                animationSpec = tween(durationMillis = 500, easing = FastOutSlowInEasing),
                                label = "category_progress_${catScore.categoryId}"
                            )
                            LinearProgressIndicator(
                                progress = { animatedCatProgress },
                                modifier = Modifier
                                    .fillMaxWidth()
                                    .height(6.dp)
                                    .clip(RoundedCornerShape(3.dp)),
                                color = catColor,
                                trackColor = MaterialTheme.colorScheme.surfaceVariant
                            )
                        }
                    }
                }
            }

            // 6. Complete Ranked List of Styles
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(16.dp)
            ) {
                Column(modifier = Modifier.padding(16.dp)) {
                    Text(
                        text = stringResource(R.string.results_style_rankings_header),
                        style = MaterialTheme.typography.titleSmall,
                        fontWeight = FontWeight.Bold
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    result.styleRankings.forEachIndexed { index, styleScore ->
                        val style = viewModel.getStyle(styleScore.styleId)
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(vertical = 6.dp),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Row(verticalAlignment = Alignment.CenterVertically) {
                                Surface(
                                    shape = RoundedCornerShape(4.dp),
                                    color = if (index == 0) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.surfaceVariant,
                                    modifier = Modifier.size(24.dp)
                                ) {
                                    Box(contentAlignment = Alignment.Center) {
                                        Text(
                                            text = "#${index + 1}",
                                            style = MaterialTheme.typography.labelSmall,
                                            fontWeight = FontWeight.Bold
                                        )
                                    }
                                }
                                Spacer(modifier = Modifier.width(8.dp))
                                Column {
                                    Text(
                                        text = style?.title ?: "Style ${styleScore.styleId}",
                                        style = MaterialTheme.typography.bodySmall,
                                        fontWeight = FontWeight.SemiBold
                                    )
                                    Text(
                                        text = "${styleScore.wins}W - ${styleScore.losses}L \u2022 ${style?.localizedName(isItalian)}",
                                        style = MaterialTheme.typography.labelSmall,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant
                                    )
                                }
                            }
                            Text(
                                text = "${String.format(Locale.US, "%.1f", styleScore.probability * 100)}%",
                                style = MaterialTheme.typography.bodySmall,
                                fontWeight = FontWeight.Bold,
                                color = MaterialTheme.colorScheme.primary
                            )
                        }
                        if (index < result.styleRankings.size - 1) {
                            HorizontalDivider(color = MaterialTheme.colorScheme.outlineVariant.copy(alpha = 0.3f))
                        }
                    }
                }
            }

            // 7. Action Buttons
            Column(
                modifier = Modifier.fillMaxWidth(),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                if (state.savedRecordId != null) {
                    Text(
                        text = stringResource(R.string.results_saved_snackbar),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.primary
                    )
                }
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    OutlinedButton(
                        onClick = onRestart,
                        modifier = Modifier.weight(1f),
                        shape = RoundedCornerShape(10.dp)
                    ) {
                        Text(stringResource(R.string.btn_retake_tournament))
                    }
                    FilledTonalButton(
                        onClick = onViewHistory,
                        modifier = Modifier.weight(1f),
                        shape = RoundedCornerShape(10.dp)
                    ) {
                        Text(stringResource(R.string.btn_view_history))
                    }
                }
            }
        }
    }
}
