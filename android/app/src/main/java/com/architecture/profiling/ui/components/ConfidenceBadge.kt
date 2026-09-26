package com.architecture.profiling.ui.components

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Info
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.architecture.profiling.R
import java.util.Locale

@Composable
fun ConfidenceBadge(
    confidencePercentage: Double,
    consistencyZeta: Double,
    circularTriads: Int,
    victoryMargin: Double,
    modifier: Modifier = Modifier,
    onInfoClick: (() -> Unit)? = null
) {
    val (badgeColor, statusText) = when {
        confidencePercentage >= 80.0 -> Color(0xFF2E7D32) to "High Certainty"
        confidencePercentage >= 60.0 -> Color(0xFFE65100) to "Moderate Certainty"
        else -> Color(0xFFC62828) to "Exploratory"
    }

    val surfaceModifier = if (onInfoClick != null) {
        modifier
            .fillMaxWidth()
            .clickable { onInfoClick() }
    } else {
        modifier.fillMaxWidth()
    }

    Surface(
        modifier = surfaceModifier,
        shape = RoundedCornerShape(12.dp),
        color = badgeColor.copy(alpha = 0.12f),
        border = BorderStroke(1.dp, badgeColor.copy(alpha = 0.4f))
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(14.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = stringResource(R.string.results_confidence_badge, confidencePercentage.toInt()),
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Black,
                    color = badgeColor
                )
                Spacer(modifier = Modifier.height(2.dp))
                Text(
                    text = "$statusText \u2022 ${stringResource(R.string.results_consistency_stat, consistencyZeta * 100.0)}",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Text(
                    text = "${stringResource(R.string.results_victory_margin_stat, victoryMargin * 100.0)} \u2022 ${stringResource(R.string.results_triads_stat, circularTriads)}",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            }

            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                Surface(
                    shape = RoundedCornerShape(6.dp),
                    color = badgeColor.copy(alpha = 0.2f)
                ) {
                    Text(
                        text = if (circularTriads == 0) "Transitive" else "$circularTriads Triads",
                        style = MaterialTheme.typography.labelSmall,
                        color = badgeColor,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp)
                    )
                }

                if (onInfoClick != null) {
                    IconButton(
                        onClick = onInfoClick,
                        modifier = Modifier.size(32.dp)
                    ) {
                        Icon(
                            imageVector = Icons.Default.Info,
                            contentDescription = stringResource(R.string.dialog_help_confidence_title),
                            tint = badgeColor,
                            modifier = Modifier.size(20.dp)
                        )
                    }
                }
            }
        }
    }
}
