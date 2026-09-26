package com.architecture.profiling.data.image

import android.content.Context
import android.graphics.BitmapFactory
import android.util.LruCache
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.graphics.ImageBitmap
import androidx.compose.ui.graphics.asImageBitmap
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.IOException

class AssetImageLoader(
    private val context: Context,
    maxMemoryCacheSizeMb: Int = 32
) {
    // 32 MB cache easily holds all 10 images (~18.3 MB uncompressed)
    private val memoryCache = object : LruCache<String, ImageBitmap>(maxMemoryCacheSizeMb * 1024 * 1024) {
        override fun sizeOf(key: String, value: ImageBitmap): Int {
            return value.width * value.height * 4
        }
    }

    suspend fun loadBitmap(filename: String): ImageBitmap? = withContext(Dispatchers.IO) {
        memoryCache.get(filename)?.let { return@withContext it }

        val candidatePaths = listOf("images/$filename", filename, "data/images/$filename")
        for (path in candidatePaths) {
            try {
                context.assets.open(path).use { stream ->
                    val bitmap = BitmapFactory.decodeStream(stream)
                    if (bitmap != null) {
                        val imageBitmap = bitmap.asImageBitmap()
                        memoryCache.put(filename, imageBitmap)
                        return@withContext imageBitmap
                    }
                }
            } catch (_: IOException) {
                // Try next path
            }
        }
        null
    }

    suspend fun preloadImages(filenames: List<String>) = withContext(Dispatchers.IO) {
        for (filename in filenames) {
            if (memoryCache.get(filename) == null) {
                loadBitmap(filename)
            }
        }
    }

    fun getCached(filename: String): ImageBitmap? = memoryCache.get(filename)
}

@Composable
fun rememberAssetImage(
    filename: String,
    imageLoader: AssetImageLoader
): ImageBitmap? {
    val bitmapState = remember(filename) { mutableStateOf(imageLoader.getCached(filename)) }

    LaunchedEffect(filename) {
        if (bitmapState.value == null) {
            bitmapState.value = imageLoader.loadBitmap(filename)
        }
    }

    return bitmapState.value
}
