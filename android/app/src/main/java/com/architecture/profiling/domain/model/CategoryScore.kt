package com.architecture.profiling.domain.model

data class CategoryScore(
    val categoryId: String,
    val affinity: Double,
    val rank: Int
) {
    val percentage: Double get() = affinity * 100.0
}
