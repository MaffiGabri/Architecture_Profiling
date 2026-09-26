package com.architecture.profiling.data.repository

import android.content.Context
import com.architecture.profiling.data.dto.StylesDocumentDto
import com.architecture.profiling.domain.model.Category
import com.architecture.profiling.domain.model.Style
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import java.io.InputStreamReader

interface StyleRepository {
    suspend fun getStyles(): List<Style>
    suspend fun getCategories(): List<Category>
    suspend fun getStyleById(id: Int): Style?
    suspend fun getCategoryById(id: String): Category?
}

class AssetStyleRepository(
    private val context: Context,
    private val jsonParser: Json = Json { ignoreUnknownKeys = true; isLenient = true }
) : StyleRepository {

    @Volatile
    private var cachedStyles: List<Style>? = null
    @Volatile
    private var cachedCategories: List<Category>? = null

    override suspend fun getStyles(): List<Style> = withContext(Dispatchers.IO) {
        cachedStyles ?: run {
            loadFromAssets()
            cachedStyles ?: emptyList()
        }
    }

    override suspend fun getCategories(): List<Category> = withContext(Dispatchers.IO) {
        cachedCategories ?: run {
            loadFromAssets()
            cachedCategories ?: emptyList()
        }
    }

    override suspend fun getStyleById(id: Int): Style? {
        return getStyles().find { it.id == id }
    }

    override suspend fun getCategoryById(id: String): Category? {
        return getCategories().find { it.id == id }
    }

    private fun loadFromAssets() {
        val assetManager = context.assets
        val possiblePaths = listOf("styles.json", "data/styles.json")
        var jsonContent: String? = null

        for (path in possiblePaths) {
            try {
                assetManager.open(path).use { stream ->
                    jsonContent = InputStreamReader(stream, Charsets.UTF_8).readText()
                }
                if (jsonContent != null) break
            } catch (_: Exception) {
                // Try next possible path
            }
        }

        val rawJson = checkNotNull(jsonContent) {
            "Could not locate styles.json in assets (checked paths: $possiblePaths)"
        }

        val doc = jsonParser.decodeFromString<StylesDocumentDto>(rawJson)
        cachedCategories = doc.categories.map { it.toDomain() }
        cachedStyles = doc.styles.map { it.toDomain() }
    }
}
