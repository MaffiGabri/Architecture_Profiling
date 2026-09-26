package com.architecture.profiling.data.dto

import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import com.architecture.profiling.domain.model.TraitRadar
import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class StylesDocumentDto(
    val categories: List<CategoryDto> = emptyList(),
    val styles: List<StyleDto> = emptyList()
)

@Serializable
data class CategoryDto(
    val id: String,
    @SerialName("name_en") val nameEn: String,
    @SerialName("name_it") val nameIt: String,
    @SerialName("description_en") val descriptionEn: String = "",
    @SerialName("description_it") val descriptionIt: String = "",
    @SerialName("color_hex") val colorHex: String = "#000000"
) {
    fun toDomain(): Category = Category(
        id = id,
        nameEn = nameEn,
        nameIt = nameIt,
        descriptionEn = descriptionEn,
        descriptionIt = descriptionIt,
        colorHex = colorHex
    )
}

@Serializable
data class TraitRadarDto(
    val era: Double = 5.0,
    val ornamentation: Double = 5.0,
    @SerialName("structural_honesty") val structuralHonesty: Double = 5.0,
    @SerialName("geometric_order") val geometricOrder: Double = 5.0,
    @SerialName("material_warmth") val materialWarmth: Double = 5.0
) {
    fun toDomain(): TraitRadar = TraitRadar(
        era = era,
        ornamentation = ornamentation,
        structuralHonesty = structuralHonesty,
        geometricOrder = geometricOrder,
        materialWarmth = materialWarmth
    )

    companion object {
        fun fromDomain(domain: TraitRadar): TraitRadarDto = TraitRadarDto(
            era = domain.era,
            ornamentation = domain.ornamentation,
            structuralHonesty = domain.structuralHonesty,
            geometricOrder = domain.geometricOrder,
            materialWarmth = domain.materialWarmth
        )
    }
}

@Serializable
data class StyleDto(
    val id: Int,
    val filename: String,
    val title: String,
    @SerialName("style_en") val styleEn: String,
    @SerialName("style_it") val styleIt: String,
    @SerialName("category_id") val categoryId: String,
    @SerialName("era_century") val eraCentury: String,
    @SerialName("base_weight") val baseWeight: Double = 1.0,
    val traits: TraitRadarDto,
    val tags: List<String> = emptyList(),
    @SerialName("color_hex") val colorHex: String = "#000000"
) {
    fun toDomain(): Style = Style(
        id = id,
        filename = filename,
        title = title,
        styleEn = styleEn,
        styleIt = styleIt,
        categoryId = categoryId,
        eraCentury = eraCentury,
        baseWeight = baseWeight,
        traits = traits.toDomain(),
        tags = tags,
        colorHex = colorHex
    )
}
