# Godot ARCore Plugin

This repository contains the source code for the Godot ARCore plugin.

> **NOTE** This plugin currently does not yet work or pass CI.
> This requires upstream changes to be merged in both Godot and Godot-cpp
> and finishing touches in this repository.
> Help to get this over the finish line would be greatly appreciated.

## License

- This project is released under the MIT license.
- Note that the build process will download additional binaries that fall under Googles own licenses.

## Requirements

### Base Requirements

- Git
- Python 3.6+
- OpenJDK 11 (`sudo apt install openjdk-11-jdk-headless`)
- Add `ANDROID_SDK_ROOT` to your path
- Scons (via `pip install SCons`)

### Android Studio

Download and install Android Studio Version 2021.1 or newer.

After that, make sure you have the latest versions by [checking the SDK Manager for updates](https://developer.android.com/studio/intro/update.html#sdk-manager)

Also install "NDK (Side By Side)" with the **EXACT version 21.4.7075529** and "Android SDK Command-line Tools (latest)"

#### debug.keystore

Android needs a debug keystore file to install to devices and distribute non-release APKs. For more information, see

[https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_android.html#create-a-debug-keystore](https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_android.html#create-a-debug-keystore)

### Godot Engine Build System

For this plugin to work, you need to compile the Godot Engine.

You can read into it in the [Contribution Guide](https://docs.godotengine.org/en/stable/development/compiling/getting_source.html) and / or use the [one-liners](https://docs.godotengine.org/en/stable/development/compiling/compiling_for_x11.html#distro-specific-one-liners) for Linux-based distros.

## Setup

There is a separate setup readme available at [SETUP.md](./SETUP.md) that tries to combine the setup instructions from multiple commits and branches into one document. Check it out.

Clone the repository and **initialise the submodules** with `git submodule update --init --recursive`.

- **Note**: When you've pulled a newer version make sure to run `git submodule update --init --recursive`.

## Godot C++ library

Checkout the [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) repo as a submodule by running the following command: `git submodule update --init --recursive`.

- The [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) repo will checkout under the `plugin/libs/godot-cpp` directory.

### Building the Godot C++ library

The godot-cpp library is automatically compiled when we compile the plugin however you do first need to generate the binding classes. This can be done by running the following commands:

```
python ./generate.py
```

**Note:**
- While this plugin is in development, you'll need to check out the latest Godot 3.x version and cherry pick [PR #47455](https://github.com/godotengine/godot/pull/47455) and compile the runtime for Android and the editor for your desktop.
- You need to copy the contents of your Godot build in the folder `modules\gdnative\include` into `plugin/libs/godot-cpp/godot-headers`
- You need to generate an `api.json` file from the Godot Engine binary built from the Godot Engine ARCore branch and copy this into `plugin/libs/godot-cpp/godot-headers`. See [these instructions](https://github.com/godotengine/godot-cpp/tree/3.4#updating-the-apijson-file) for more details.
- You need to copy the build template created after [Compiling Godot for Android](https://docs.godotengine.org/en/3.4/development/compiling/compiling_for_android.html)

## Android SDK & NDK

- Download and setup [Android Studio version **4.0** or higher](https://developer.android.com/studio).
  - After setup, ensure you have the latest versions by checking the [SDK Manager](https://developer.android.com/studio/intro/update.html#sdk-manager) for updates.
- Set the environment variable `ANDROID_HOME` to the Android SDK location.
- Follow the instructions [here](https://developer.android.com/studio/projects/install-ndk#specific-version) and install version **21.1.6528147** of the NDK.

### Build

#### Generate plugin binary files

In the project root directory:

- Run `./gradlew :generatePluginBinary` to generate the plugin binary files.
- Once the build successfully completes, the binary files can be found in the `build/outputs/pluginBin` directory.

### Deploy

Make sure in the Editor Settings screen the Android tab is setup correctly follownig the instructions on the [Export for Android](https://docs.godotengine.org/en/3.4/getting_started/workflow/export/exporting_for_android.html) help pages.
You also need to configure the new `Custom Build Sdk Path` and point it to your Android Studio SDK folder.

Your project needs to be setup with a [Godot Android custom build template](https://docs.godotengine.org/en/3.4/getting_started/workflow/export/android_custom_build.html).

Copy the AAR files for this plugin created during the build phase into the `res://android/plugins` folder created when setting up the custom build templates.
Also copy `plugin/gdarcore.gdap` into the `res://android/plugins` folder.

When exporting the project apk in Godot, the following Android export options should be set:

- `Xr Features`
  - `Xr Mode` must be set to `AR Core`.
- `Custom Template`
  - `Use Custom Build` must be enabled.
- 'Version'
  - `Min Sdk` must be set to atleast 24.
- `Plugins`
  - `ARCore` must be listed and enabled in the `Plugins` section.

## About this repository

This repository is mainly being maintained by:

- [Fredia Huya-Kouadio](https://github.com/m4gr3d)
- [Bastiaan Olij](https://github.com/BastiaanOlij)

See contributors file for other people involved in this plugin.
