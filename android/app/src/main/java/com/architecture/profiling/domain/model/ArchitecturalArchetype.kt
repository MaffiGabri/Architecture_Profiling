package com.architecture.profiling.domain.model

data class ArchitecturalArchetype(
    val id: String,
    val titleEn: String,
    val titleIt: String,
    val taglineEn: String,
    val taglineIt: String,
    val narrativeEn: String,
    val narrativeIt: String,
    val dominantCategoryId: String
)
