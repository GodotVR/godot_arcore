# Godot ARCore Plugin

This repository contains the source code for the Godot ARCore plugin.

> **IMPORTANT:** We are currently updating this plugin to work with Godot 4.4 and later.  
> This is still a work in progress.

> **NOTE** This plugin currently does not yet work or pass CI.
> This requires upstream changes to be merged in both Godot and godot-cpp
> and finishing touches in this repository.
> Help to get this over the finish line would be greatly appreciated.

## License

This project is released under the MIT license.

Note that the build process will download additional binaries that fall under Googles own licenses.

## Requirements

This project requires a working Android Development Setup for compiling the plugin.  
In short, you need the following software and versions:

- Python 3.6+
- SCons 3.1.2+
- Android SDK (command-line tools are sufficient)
- Gradle (will be downloaded and installed automatically if missing)
- JDK 17 (either OpenJDK or Oracle JDK)

> See [https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_for_android.html#requirements](https://docs.godotengine.org/en/stable/contributing/development/compiling/compiling_for_android.html#requirements) for details and installation instructions.


## Usage
**Note:** [Android Studio](https://developer.android.com/studio) is the recommended IDE for developing Godot Android plugins.
You can install the latest version from https://developer.android.com/studio.

Clone the repository to your local machine, run the following command in the project root directory to initialize the `godot-cpp` submodule:
```
git submodule update --init
```

### Building the C++ bindings
Build the Android C++ bindings using the following commands. To speed up compilation, add `-jN` at
the end of the SCons command line where `N` is the number of CPU threads you have on your system.
The example below uses 4 threads.
```
cd godot-cpp
scons platform=android target=template_debug -j4
scons platform=android target=template_release -j4
```

When the command is completed, you should have static libraries stored in `godot-cpp/bin` that
will be used for compilation by the plugin.

### Building the configured Android plugin
- In a terminal window, navigate to the project's root directory and run the following command:
```
./gradlew assemble
```
- On successful completion of the build, the output files can be found in
  [`plugin/demo/addons/ARCorePlugin`](plugin/demo/addons/ARCorePlugin/)

### Building the configured Android plugin with Android Studio (optional)
- Open Android Studio and Click on `Open...` navigate to the git repository and click on `godot_arcore` where the android robot shows up and wait for Android Studio to finish loading

- In the top bar click on `Add Configuration...` and then `Edit Configurations...`

- For the development you can create two configurations, one for building the project and one for cleaning the output folders:

- Click on the `+` sign and choose "Gradle" as the type for the configuration

- In the window to the right give the configuration a name - *`build` in this case* - and under "Run" type `assemble`, which corresponds to the command `./gradlew assemble` which you would do in the command line

- You can repeat the steps for the gradle task `clean` if you want

### Testing the Android plugin
You can use the included [Godot demo project](plugin/demo/project.godot) to test the built Android plugin

- Open the demo in Godot (4.4 or higher)
- Navigate to `Project` -> `Project Settings...` -> `Plugins`, and ensure the plugin is enabled
- Install the Godot Android build template by clicking on `Project` -> `Install Android Build Template...`
- Open [`plugin/demo/main.gd`](plugin/demo/main.gd) and update the logic as needed to reference
  your plugin and its methods
- Connect an Android device to your machine and run the demo on it

### Testing the Android Plugin with the Android Studio Device Emulator (experimental)

Follow the installation in [Run AR Apps in Android Emulator](https://developers.google.com/ar/develop/java/emulator) and create a device

For the Android plugin and the resulting app to compile you have to add x86 ABIs to your setup. That means compiling godot-cpp for x86_32 and x86_64 and making some changes to the build.gradle.kts file of the plugin:

Ensure that the `ndk` config in `plugin/build.gradle.kts` contains these three lines:
```gradle
ndk {
	abiFilters.add("arm64-v8a")
	abiFilters.add("x86")
	abiFilters.add("x86_64")
}
```

To compile godot-cpp for the other two architectures, navigate to the `godot-cpp` submodule and build it:

```bash
cd godot-cpp
scons platform=android target=template_debug arch=x86_32
scons platform=android target=template_debug arch=x86_64
scons platform=android target=template_release arch=x86_32
scons platform=android target=template_release arch=x86_64
```

> See [https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#building-the-c-bindings](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#building-the-c-bindings) for details.


> **IMPORTANT:** When exporting the Godot app with the plugin enabled, don't forget to add the architectures to your build.  
> Open your project in the Godot Engine, navigate to `Project` -> `Export...` click on your Android Preset and under "Architectures" enable both `x86` and `x86_64`