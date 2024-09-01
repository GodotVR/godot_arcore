import com.android.build.gradle.internal.tasks.factory.dependsOn

plugins {
    id("com.android.library")
    id("org.jetbrains.kotlin.android")
}

// TODO: Update value to your plugin's name.
val pluginName = "ARCorePlugin"

// TODO: Update value to match your plugin's package name.
val pluginPackageName = "org.godotengine.plugin.android.arcore"

/*
The ARCore AAR library contains native shared libraries that are
extracted before building to a temporary directory.
*/
val arcoreLibpath = "${buildDir}/arcore-native"

/**
 * Flag used to specify whether the `plugin.gdextension` config file has libraries for platforms
 * other than Android and can be used by the Godot Editor
 *
 * TODO: Update the flag value based on your plugin's configuration
 */
val gdextensionSupportsNonAndroidPlatforms = false

android {
    namespace = pluginPackageName
    compileSdk = 33

    buildFeatures {
        buildConfig = true
    }

    packagingOptions {
        jniLibs.keepDebugSymbols.add("**/*.so")
    }

    defaultConfig {
        minSdk = 24

        externalNativeBuild {
            cmake {
                cppFlags("-std=c++17", "-Wall")
                arguments("-DANDROID_STL=c++_static")
            }
        }
        ndk {
            abiFilters.add("arm64-v8a")
            // Comment these two lines back in to enable Emulator support
            //abiFilters.add("x86")
            //abiFilters.add("x86_64")
        }

        manifestPlaceholders["godotPluginName"] = pluginName
        manifestPlaceholders["godotPluginPackageName"] = pluginPackageName
        buildConfigField("String", "GODOT_PLUGIN_NAME", "\"${pluginName}\"")
        setProperty("archivesBaseName", pluginName)
    }
    externalNativeBuild {
        cmake {
            path("CMakeLists.txt")
            version = "3.22.1"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }
    ndkVersion = "23.2.8568313"
    buildToolsVersion = "33.0.2"
}

dependencies {
    implementation("org.godotengine:godot:4.2.0.stable")
    implementation("com.google.ar:core:1.44.0")
    configurations.create("arcoreImplementation")
    "arcoreImplementation" ("com.google.ar:core:1.44.0")
    implementation("androidx.activity:activity-ktx:1.2.0-alpha04")
    implementation("androidx.appcompat:appcompat:1.4.0")
    implementation("androidx.fragment:fragment-ktx:1.3.0")
    implementation("androidx.core:core-ktx:1.3.1")
}

// BUILD TASKS DEFINITION
val extractNativeLibs by tasks.registering(Copy::class) {
    val arcoreImplementation = configurations.getByName("arcoreImplementation")

    dependsOn(arcoreImplementation)

    from(arcoreImplementation.map {zipTree(it)})
    into(arcoreLibpath)
}

tasks.named("preBuild") {
    dependsOn(tasks.getByName("extractNativeLibs"))
}

val cleanAssetsAddons by tasks.registering(Copy::class) {
    delete("src/main/assets/addons")
}

val copyGdExtensionConfigToAssets by tasks.registering(Copy::class) {
    description = "Copies the gdextension config file to the plugin's assets directory"

    dependsOn(cleanAssetsAddons)

    from("export_scripts_template")
    include("plugin.gdextension")
    into("src/main/assets/addons/$pluginName")
}

val copyDebugAARToDemoAddons by tasks.registering(Copy::class) {
    description = "Copies the generated debug AAR binary to the plugin's addons directory"
    from("build/outputs/aar")
    include("$pluginName-debug.aar")
    into("demo/addons/$pluginName/bin/debug")
}

val copyReleaseAARToDemoAddons by tasks.registering(Copy::class) {
    description = "Copies the generated release AAR binary to the plugin's addons directory"
    from("build/outputs/aar")
    include("$pluginName-release.aar")
    into("demo/addons/$pluginName/bin/release")
}

val copyDebugSharedLibs by tasks.registering(Copy::class) {
    description = "Copies the generated debug .so shared library to the plugin's addons directory"
    from("build/intermediates/cmake/debug/obj")
    into("demo/addons/$pluginName/bin/debug")
}

val copyReleaseSharedLibs by tasks.registering(Copy::class) {
    description = "Copies the generated release .so shared library to the plugin's addons directory"
    from("build/intermediates/cmake/release/obj")
    into("demo/addons/$pluginName/bin/release")
}

val cleanDemoAddons by tasks.registering(Delete::class) {
    delete("demo/addons/$pluginName")
}

val copyAddonsToDemo by tasks.registering(Copy::class) {
    description = "Copies the plugin's output artifact to the output directory"

    dependsOn(cleanDemoAddons)
    finalizedBy(copyDebugAARToDemoAddons)
    finalizedBy(copyReleaseAARToDemoAddons)

    from("export_scripts_template")
    if (!gdextensionSupportsNonAndroidPlatforms) {
        exclude("plugin.gdextension")
    } else {
        finalizedBy(copyDebugSharedLibs)
        finalizedBy(copyReleaseSharedLibs)
    }
    into("demo/addons/$pluginName")
}

tasks.named("preBuild").dependsOn(copyGdExtensionConfigToAssets)

tasks.named("assemble").configure {
    dependsOn(copyGdExtensionConfigToAssets)
    finalizedBy(copyAddonsToDemo)
}

tasks.named<Delete>("clean").apply {
    dependsOn(cleanDemoAddons)
    dependsOn(cleanAssetsAddons)
}
