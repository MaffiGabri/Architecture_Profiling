package com.architecture.profiling.domain.model

data class Style(
    val id: Int,
    val filename: String,
    val title: String,
    val styleEn: String,
    val styleIt: String,
    val categoryId: String,
    val eraCentury: String,
    val baseWeight: Double = 1.0,
    val traits: TraitRadar,
    val tags: List<String> = emptyList(),
    val colorHex: String = "#000000"
)
