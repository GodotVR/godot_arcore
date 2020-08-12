# Godot ARCore Plugin

This repository contains the source code for the Godot ARCore plugin.

## License

- This project is released under the MIT license.
- Note that the build process will download additional binaries that fall under Googles own licenses.

## Contributions

### Setup

Clone the repository and **initialise the submodules** with `git submodule update --init --recursive`.
  - **Note**: When you've pulled a newer version make sure to run `git submodule update --init --recursive`.<br>

#### Godot C++ library
Checkout the [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) repo as a submodule by running the following command: `git submodule update --init --recursive`.
  - The [godot-cpp](https://github.com/GodotNativeTools/godot-cpp) repo will checkout under the `plugin/libs/godot-cpp` directory.

##### Building the Godot C++ library
Navigate to the `plugin/libs/godot-cpp` directory and follow [these steps](https://github.com/GodotNativeTools/godot-cpp/tree/master#compiling-the-cpp-bindings-library) to generate the cpp bindings.

**Note:**
- While this plugin is in development, you'll need to check-out the Godot Engine ARCore branch in
[PR #41026](https://github.com/godotengine/godot/pull/41026) and update the `GODOT_HEADERS_DIR`
variable in the `CMakeLists.txt` file to point to its `godot/modules/gdnative/include` directory.
- You need to generate an `api.json` file from the Godot Engine binary built from the Godot Engine
ARCore branch and use that api file when building the Godot C++ library by following [these instructions](https://github.com/godotengine/godot-cpp#updating-the-apijson-file).

#### Android SDK & NDK
- Download and setup [Android Studio version **4.0** or higher](https://developer.android.com/studio).
  - After setup, ensure you have the latest versions by checking the [SDK Manager](https://developer.android.com/studio/intro/update.html#sdk-manager) for updates.
- Set the environment variable `ANDROID_HOME` to the Android SDK location.
- Follow the instructions [here](https://developer.android.com/studio/projects/install-ndk#specific-version) and install version **21.1.6352462** of the NDK.

### Build

#### Generate plugin binary files
In the project root directory:
 - Run `./gradlew :generatePluginBinary` to generate the plugin binary files.
 - Once the build successfully completes, the binary files can be found in the `build/outputs/pluginBin` directory.
