# GDExtension ARCore Plugin


## Usage
**Note:** [Android Studio](https://developer.android.com/studio) is the recommended IDE for 
developing Godot Android plugins. 
You can install the latest version from https://developer.android.com/studio.

Clone the repository to your local machine, run the following command in the project root 
directory to initialize the `godot-cpp` submodule:
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
  [`plugin/demo/addons`](plugin/demo/addons)

### Building the configured Android plugin with Android Studio
- Open Android Studio and Click on `Open...` navigate to the git repo and click on `godot_arcore_plugin` where the android robot shows up and wait for Android Studio to finish loading

- In the top bar click on `Add Configuration...` and then `Edit Configurations...`

- For the development you can create two configurations, one for building the project and one for cleaning the output folders:

- Click on the `+` sign and choose "Gradle" as the type for the configuration

- In the window to the right give the configuration a name - *`build` in this case* - and under "Run" type `assemble`, which corresponds to the command `./gradlew assemble` which you would do in the command line

- You can repeat the steps for the gradle task `clean` if you want

### Testing the Android plugin
You can use the included [Godot demo project](plugin/demo/project.godot) to test the built Android plugin

- Open the demo in Godot (4.2 or higher)
- Navigate to `Project` -> `Project Settings...` -> `Plugins`, and ensure the plugin is enabled
- Install the Godot Android build template by clicking on `Project` -> `Install Android Build Template...`
- Open [`plugin/demo/main.gd`](plugin/demo/main.gd) and update the logic as needed to reference 
  your plugin and its methods
- Connect an Android device to your machine and run the demo on it

### Testing the Android Plugin with the Android Studio Device Emulator

Follow the installation in [Run AR Apps in Android Emulator](https://developers.google.com/ar/develop/java/emulator) and create a device

For the Android plugin and the resulting app to compile you have to add x86 ABIs to your setup. That means compiling godot-cpp for x86_32 and x86_64 and making some changes to the build.gradle.kts file of the plugin:

Add these lines to you `plugin/build.gradle.kts`:
```gradle
ndk {
	abiFilters.add("arm64-v8a")
	abiFilters.add("x86")
	abiFilters.add("x86_64")
}
```

To compile godot-cpp for the two architectures, navigate to the `godot-cpp` submodule and build it. See [Building the C++ bindings](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#building-the-c-bindings) for details.

```bash
cd godot-cpp
scons platform=android target=template_debug arch=x86_32
scons platform=android target=template_debug arch=x86_64
scons platform=android target=template_release arch=x86_32
scons platform=android target=template_release arch=x86_64
```

When exporting the Godot app with the plugin enabled, don't forget to add the architectures to your build.  
Open your project in the Godot Engine, navigate to `Project` -> `Export...` click on your Android Preset and under "Architectures" enable both `x86` and `x86_64`

#### Tips

##### Simplify access to the exposed Java / Kotlin APIs

To make it easier to access the exposed Java / Kotlin APIs in the Godot Editor, it's recommended to
provide one (or multiple) gdscript wrapper class(es) for your plugin users to interface with.

For example:

```
class_name PluginInterface extends Object

## Interface used to access the functionality provided by this plugin

var _plugin_name = "GDExtensionAndroidPluginTemplate"
var _plugin_singleton

func _init():
	if Engine.has_singleton(_plugin_name):
		_plugin_singleton = Engine.get_singleton(_plugin_name)
	else:
		printerr("Initialization error: unable to access the java logic")

## Print a 'Hello World' message to the logcat.
func helloWorld():
	if _plugin_singleton:
		_plugin_singleton.helloWorld()
	else:
		printerr("Initialization error")

```

##### Support using the gdextension functionality in the Godot Editor

If planning to use the gdextension functionality in the Godot Editor, it is recommended that the 
gdextension's native binaries are compiled not just for Android, but also for the OS onto which 
developers / users intend to run the Godot Editor. Not doing so may prevent developers / 
users from writing code that accesses the plugin from within the Godot Editor.

This may involve creating dummy plugins for the host OS just so the API is published to the 
editor. You can use the [godot-cpp-template](https://github.com/godotengine/godot-cpp-template) 
github template for reference on how to do so.

## Gotchas

Currently, the gradlew script has Windows-style line endings which stops the script from executing under Linux.

To fix that, navigate to the folder where the `gradlew` script is and enter the following command:

```sh
dos2unix ./gradlew
```

This will convert the line endings to LF.
