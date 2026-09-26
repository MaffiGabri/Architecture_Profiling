package com.architecture.profiling.ui.screens

import androidx.compose.animation.*
import androidx.compose.animation.core.FastOutSlowInEasing
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.focusable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.HelpOutline
import androidx.compose.material.icons.automirrored.filled.Redo
import androidx.compose.material.icons.automirrored.filled.Undo
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.input.key.*
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.architecture.profiling.R
import com.architecture.profiling.ui.components.StyleCard
import com.architecture.profiling.ui.components.TournamentMechanicsDialog
import com.architecture.profiling.ui.state.TournamentUiState
import com.architecture.profiling.ui.viewmodel.TournamentViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TournamentScreen(
    state: TournamentUiState.InProgress,
    viewModel: TournamentViewModel,
    onExit: () -> Unit
) {
    var showExitDialog by remember { mutableStateOf(false) }
    var showResetDialog by remember { mutableStateOf(false) }
    var showHelpDialog by remember { mutableStateOf(false) }

    val focusRequester = remember { FocusRequester() }
    val haptic = LocalHapticFeedback.current

    LaunchedEffect(state.matchIndex) {
        focusRequester.requestFocus()
    }

    if (showExitDialog) {
        AlertDialog(
            onDismissRequest = { showExitDialog = false },
            title = { Text(stringResource(R.string.dialog_restart_title)) },
            text = { Text(stringResource(R.string.dialog_restart_desc)) },
            confirmButton = {
                TextButton(onClick = {
                    showExitDialog = false
                    viewModel.reset()
                    onExit()
                }) {
                    Text(stringResource(R.string.btn_confirm))
                }
            },
            dismissButton = {
                TextButton(onClick = { showExitDialog = false }) {
                    Text(stringResource(R.string.btn_cancel))
                }
            }
        )
    }

    if (showResetDialog) {
        AlertDialog(
            onDismissRequest = { showResetDialog = false },
            title = { Text(stringResource(R.string.dialog_restart_title)) },
            text = { Text(stringResource(R.string.dialog_restart_desc)) },
            confirmButton = {
                TextButton(onClick = {
                    showResetDialog = false
                    viewModel.reset()
                    onExit()
                }) {
                    Text(stringResource(R.string.btn_confirm))
                }
            },
            dismissButton = {
                TextButton(onClick = { showResetDialog = false }) {
                    Text(stringResource(R.string.btn_cancel))
                }
            }
        )
    }

    if (showHelpDialog) {
        TournamentMechanicsDialog(
            onDismiss = { showHelpDialog = false }
        )
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = {
                    Column {
                        Text(
                            text = stringResource(R.string.tournament_step, state.matchIndex + 1, state.totalMatches),
                            style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Bold
                        )
                        Text(
                            text = "Round ${state.roundIndex + 1} \u2022 ${state.mode.name}",
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                },
                navigationIcon = {
                    IconButton(onClick = { showExitDialog = true }) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = stringResource(R.string.btn_back)
                        )
                    }
                },
                actions = {
                    IconButton(onClick = { showHelpDialog = true }) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.HelpOutline,
                            contentDescription = stringResource(R.string.action_help)
                        )
                    }
                    IconButton(
                        onClick = {
                            haptic.performHapticFeedback(HapticFeedbackType.TextHandleMove)
                            viewModel.undo()
                        },
                        enabled = state.canUndo
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.Undo,
                            contentDescription = stringResource(R.string.btn_undo)
                        )
                    }
                    IconButton(
                        onClick = {
                            haptic.performHapticFeedback(HapticFeedbackType.TextHandleMove)
                            viewModel.redo()
                        },
                        enabled = state.canRedo
                    ) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.Redo,
                            contentDescription = stringResource(R.string.btn_redo)
                        )
                    }
                    IconButton(onClick = { showResetDialog = true }) {
                        Icon(
                            imageVector = Icons.Default.Refresh,
                            contentDescription = stringResource(R.string.btn_restart)
                        )
                    }
                }
            )
        }
    ) { padding ->
        val targetProgress = (state.progressPercentage / 100.0).toFloat().coerceIn(0f, 1f)
        val animatedProgress by animateFloatAsState(
            targetValue = targetProgress,
            animationSpec = tween(durationMillis = 350, easing = FastOutSlowInEasing),
            label = "tournament_progress_animation"
        )

        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(horizontal = 16.dp, vertical = 8.dp)
                .focusRequester(focusRequester)
                .focusable()
                .onKeyEvent { event ->
                    if (event.type == KeyEventType.KeyDown) {
                        when (event.key) {
                            Key.One, Key.NumPad1, Key.DirectionLeft, Key.A -> {
                                haptic.performHapticFeedback(HapticFeedbackType.TextHandleMove)
                                viewModel.vote(state.leftStyle.id)
                                true
                            }
                            Key.Two, Key.NumPad2, Key.DirectionRight, Key.B -> {
                                haptic.performHapticFeedback(HapticFeedbackType.TextHandleMove)
                                viewModel.vote(state.rightStyle.id)
                                true
                            }
                            Key.U, Key.Z -> {
                                if (state.canUndo) {
                                    haptic.performHapticFeedback(HapticFeedbackType.TextHandleMove)
                                    viewModel.undo()
                                    true
                                } else false
                            }
                            Key.R, Key.Y -> {
                                if (state.canRedo) {
                                    haptic.performHapticFeedback(HapticFeedbackType.TextHandleMove)
                                    viewModel.redo()
                                    true
                                } else false
                            }
                            Key.Escape, Key.Back -> {
                                showExitDialog = true
                                true
                            }
                            else -> false
                        }
                    } else false
                }
        ) {
            // Linear Progress Indicator with smooth animation
            LinearProgressIndicator(
                progress = { animatedProgress },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(6.dp)
                    .clip(RoundedCornerShape(3.dp)),
                color = MaterialTheme.colorScheme.primary,
                trackColor = MaterialTheme.colorScheme.surfaceVariant
            )

            Spacer(modifier = Modifier.height(8.dp))

            Text(
                text = stringResource(R.string.tournament_prompt),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.padding(bottom = 8.dp)
            )

            Box(modifier = Modifier.weight(1f)) {
                val leftCategory = remember(state.leftStyle.categoryId) {
                    viewModel.getCategory(state.leftStyle.categoryId)
                }
                val rightCategory = remember(state.rightStyle.categoryId) {
                    viewModel.getCategory(state.rightStyle.categoryId)
                }

                AnimatedContent(
                    targetState = state.matchIndex,
                    transitionSpec = {
                        (fadeIn(animationSpec = tween(220)) + slideInHorizontally { width -> width / 4 })
                            .togetherWith(fadeOut(animationSpec = tween(180)) + slideOutHorizontally { width -> -width / 4 })
                    },
                    label = "match_transition",
                    modifier = Modifier.fillMaxSize()
                ) { _ ->
                    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
                        val isWideScreen = maxWidth >= 600.dp

                        if (isWideScreen) {
                            Row(
                                modifier = Modifier.fillMaxSize(),
                                horizontalArrangement = Arrangement.spacedBy(16.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                StyleCard(
                                    style = state.leftStyle,
                                    category = leftCategory,
                                    imageBitmap = state.leftBitmap,
                                    isLeft = true,
                                    onVote = { id -> viewModel.vote(id) },
                                    modifier = Modifier
                                        .weight(1f)
                                        .fillMaxHeight()
                                )
                                Box(
                                    modifier = Modifier
                                        .size(40.dp)
                                        .clip(RoundedCornerShape(20.dp))
                                        .background(MaterialTheme.colorScheme.surfaceVariant),
                                    contentAlignment = Alignment.Center
                                ) {
                                    Text(
                                        text = stringResource(R.string.match_vs),
                                        style = MaterialTheme.typography.labelMedium,
                                        fontWeight = FontWeight.Black,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant
                                    )
                                }
                                StyleCard(
                                    style = state.rightStyle,
                                    category = rightCategory,
                                    imageBitmap = state.rightBitmap,
                                    isLeft = false,
                                    onVote = { id -> viewModel.vote(id) },
                                    modifier = Modifier
                                        .weight(1f)
                                        .fillMaxHeight()
                                )
                            }
                        } else {
                            Column(
                                modifier = Modifier.fillMaxSize(),
                                verticalArrangement = Arrangement.spacedBy(8.dp)
                            ) {
                                StyleCard(
                                    style = state.leftStyle,
                                    category = leftCategory,
                                    imageBitmap = state.leftBitmap,
                                    isLeft = true,
                                    onVote = { id -> viewModel.vote(id) },
                                    modifier = Modifier
                                        .weight(1f)
                                        .fillMaxWidth()
                                )
                                StyleCard(
                                    style = state.rightStyle,
                                    category = rightCategory,
                                    imageBitmap = state.rightBitmap,
                                    isLeft = false,
                                    onVote = { id -> viewModel.vote(id) },
                                    modifier = Modifier
                                        .weight(1f)
                                        .fillMaxWidth()
                                )
                            }
                        }
                    }
                }
            }
        }
    }
}
