package com.architecture.profiling.domain.model

data class Category(
    val id: String,
    val nameEn: String,
    val nameIt: String,
    val descriptionEn: String = "",
    val descriptionIt: String = "",
    val colorHex: String = "#000000"
)
