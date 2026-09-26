package com.architecture.profiling.domain.model

data class MatchPair(
    val matchIndex: Int,
    val roundIndex: Int,
    val leftStyleId: Int,
    val rightStyleId: Int
)
