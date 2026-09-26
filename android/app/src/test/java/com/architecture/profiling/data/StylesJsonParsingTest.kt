package com.architecture.profiling.data

import com.architecture.profiling.data.dto.StylesDocumentDto
import com.google.common.truth.Truth.assertThat
import kotlinx.serialization.json.Json
import org.junit.Test
import java.io.File

class StylesJsonParsingTest {

    private val jsonParser = Json { ignoreUnknownKeys = true; isLenient = true }

    @Test
    fun test_parse_styles_json_extracts_correct_structure_and_counts() {
        val file = File("../../shared/data/styles.json")
        assertThat(file.exists()).isTrue()

        val jsonString = file.readText()
        val doc = jsonParser.decodeFromString<StylesDocumentDto>(jsonString)

        assertThat(doc.categories).hasSize(5)
        assertThat(doc.styles).hasSize(10)

        val catIds = doc.categories.map { it.id }
        assertThat(catIds).containsExactly(
            "classical_renaissance",
            "historicist_sacred",
            "early_modern_industrial",
            "modernism_functionalism",
            "contemporary_parametric"
        )
    }

    @Test
    fun test_style_dto_mapping_to_domain_retains_all_attributes() {
        val file = File("../../shared/data/styles.json")
        val jsonString = file.readText()
        val doc = jsonParser.decodeFromString<StylesDocumentDto>(jsonString)

        val firstStyle = doc.styles.first { it.id == 1 }
        val domainStyle = firstStyle.toDomain()

        assertThat(domainStyle.id).isEqualTo(1)
        assertThat(domainStyle.title).isEqualTo("The Parthenon Colonnade")
        assertThat(domainStyle.styleEn).isEqualTo("Classical Antiquity")
        assertThat(domainStyle.styleIt).isEqualTo("Antichità Classica")
        assertThat(domainStyle.categoryId).isEqualTo("classical_renaissance")
        assertThat(domainStyle.traits.structuralHonesty).isEqualTo(7.0)
        assertThat(domainStyle.traits.era).isEqualTo(0.5)
        assertThat(domainStyle.traits.ornamentation).isEqualTo(5.5)
        assertThat(domainStyle.traits.geometricOrder).isEqualTo(0.5)
        assertThat(domainStyle.traits.materialWarmth).isEqualTo(8.5)
    }

    @Test
    fun test_category_dto_mapping_to_domain_retains_all_attributes() {
        val file = File("../../shared/data/styles.json")
        val jsonString = file.readText()
        val doc = jsonParser.decodeFromString<StylesDocumentDto>(jsonString)

        val firstCategory = doc.categories.first { it.id == "classical_renaissance" }
        val domainCategory = firstCategory.toDomain()

        assertThat(domainCategory.id).isEqualTo("classical_renaissance")
        assertThat(domainCategory.nameEn).isEqualTo("Classical & Renaissance")
        assertThat(domainCategory.nameIt).isEqualTo("Classico e Rinascimentale")
        assertThat(domainCategory.colorHex).isNotEmpty()
    }
}
