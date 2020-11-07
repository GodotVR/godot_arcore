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
[PR #41026](https://github.com/godotengine/godot/pull/41026) and compile the runtime for Android and the editor for your desktop.
- You need to copy the contents of your Godot build in the folder `modules\gdnative\include` into `plugin/libs/godot-cpp/godot_headers`
- You need to generate an `api.json` file from the Godot Engine binary built from the Godot Engine 
ARCore branch and copy this into `plugin/libs/godot-cpp/godot_headers`. See [these instructions](https://github.com/godotengine/godot-cpp#updating-the-apijson-file) for more details.
- You need to copy the build template created after [Compiling Godot for Android](https://docs.godotengine.org/en/stable/development/compiling/compiling_for_android.html)

#### Android SDK & NDK
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

Make sure in the Editor Settings screen the Android tab is setup correctly follownig the instructions on the [Export for Android](https://docs.godotengine.org/en/stable/getting_started/workflow/export/exporting_for_android.html) help pages.
You also need to configure the new `Custom Build Sdk Path` and point it to your Android Studio SDK folder.

Your project needs to be setup with a [Godot Android custom build template](https://docs.godotengine.org/en/3.2/getting_started/workflow/export/android_custom_build.html).

Copy the AAR files for this plugin created during the build phase into the `res://android/plugins` folder created when setting up the custom build templates.
Also copy `plugin/gdarcore.gdap` into the `res://android/plugins` folder.

When exporting the project apk in Godot, the following Android export options should be set:
- `Xr Features`
  - `Xr Mode` must be set to `AR Core`.
- `Custom Template`
  - `Use Custom Build` must be enabled.
- `Plugins`
  -  `ARCore` must be listed and enabled in the `Plugins` section.
