plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
    id("org.jetbrains.kotlin.plugin.serialization")
}

android {
    namespace = "com.architecture.profiling"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.architecture.profiling"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }

    buildFeatures {
        compose = true
    }
}

// ============================================================================
// Automatic Shared Asset Packaging (Pre-Build)
// Copies shared styles.json and 10 mock PNG images into Android assets directory
// ============================================================================
val copySharedAssets by tasks.registering(Copy::class) {
    description = "Copies shared styles.json and architectural PNG images into Android assets directory"
    group = "assets"

    val sharedDir = file("${project.rootDir}/../shared")

    // Copy styles.json into root assets and assets/data
    from("${sharedDir}/data") {
        include("styles.json")
        into("data")
    }
    from("${sharedDir}/data") {
        include("styles.json")
    }

    // Copy 10 mock PNG images into root assets and assets/images
    from("${sharedDir}/images") {
        include("*.png")
        into("images")
    }
    from("${sharedDir}/images") {
        include("*.png")
    }

    into(layout.projectDirectory.dir("src/main/assets"))
}

tasks.named("preBuild") {
    dependsOn(copySharedAssets)
}

dependencies {
    // Kotlin & Coroutines & Serialization
    implementation(platform("org.jetbrains.kotlin:kotlin-bom:2.0.20"))
    implementation("org.jetbrains.kotlin:kotlin-stdlib")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:1.8.1")
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.8.1")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.3")

    // AndroidX Core & Lifecycle
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.appcompat:appcompat:1.7.0")
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.8.5")
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.8.5")
    implementation("androidx.activity:activity-compose:1.9.2")

    // Jetpack Compose BOM & Material 3
    implementation(platform("androidx.compose:compose-bom:2024.09.00"))
    implementation("androidx.compose.ui:ui")
    implementation("androidx.compose.ui:ui-graphics")
    implementation("androidx.compose.ui:ui-tooling-preview")
    implementation("androidx.compose.material3:material3")
    implementation("androidx.compose.material:material-icons-extended")
    implementation("androidx.navigation:navigation-compose:2.8.0")

    // DataStore Preferences (Local User Profile & Theme Persistence)
    implementation("androidx.datastore:datastore-preferences:1.1.1")

    // Testing dependencies (Pure JVM Unit Tests)
    testImplementation("junit:junit:4.13.2")
    testImplementation("org.jetbrains.kotlin:kotlin-test:2.0.20")
    testImplementation("com.google.truth:truth:1.4.2")
    testImplementation("org.json:json:20240303")
}
