# Setup Instructions

This file contains instructions for setting up the project.  
It is tested and verified to compile on Ubuntu 22.04.2 LTS.

## Requirements

- You need to install the build environment for the Godot Engine.  
  See the [README](./README.md) for details

## Setup the development environment

You will clone two repositories, one for the engine and build templates and one for the plugin during the development.

### 1. Compiling the Engine & Editor

- Clone the godot engine repository (the current 3.5 branch)

  `git clone --branch 3.5 git@github.com:godotengine/godot.git`

- Change into the repository and add the repository that contains the needed changes for ARCore to work

  `git remote add changes git@github.com:BastiaanOlij/godot.git && git fetch changes`

- Cherry-pick the changes onto the engine

  `git cherry-pick changes/arcore_changes cb21841`

- Compile the Editor for the platform of your choice, **and** Android, so you can compile and run the demo project on your device later

  **Windows:**

  - `scons platform=windows target=debug`

  **Linux:**

  - `scons platform=x11 target=debug`

  **Android:**

  - `scons platform=android target=release android_arch=armv7`
  - `scons platform=android target=release android_arch=arm64v8`
  - `scons platform=android target=release_debug android_arch=armv7`
  - `scons platform=android target=release_debug android_arch=arm64v8`

  for creating debug symbols:

  - `scons platform=android target=debug android_arch=armv7`
  - `scons platform=android target=debug android_arch=arm64v8`

### 2. Compiling the Plugin

- Navigate back to your repositories folder and clone the ARCore plugin repository and the needed branch `make_this_work`

  `git clone --branch make_this_work git@github.com:GodotVR/godot_arcore.git && cd godot_arcore`

- Change into the godot_arcore repository and initialise the needed c++ submodules for the plugin

  `git submodule update --init --recursive`

- Copy the folder `modules/gdnative/include` from your Godot compilation in Step 1 into `plugin/libs/godot-cpp/godot-headers` of your godot_arcore repository

  `cp -r godot/modules/gdnative/include/* godot_arcore/plugin/libs/godot-cpp/godot-headers/`

- Create a new api.json

  **Windows:**

  - `./godot/bin/godot.windows.tools.64.exe --gdnative-generate-json-api godot/bin/api.json`

  **Linux:**

  - `./godot/bin/godot.x11.tools.64 --gdnative-generate-json-api godot/bin/api.json`

- Copy the api.json file into the `plugin/libs/godot-cpp/godot-headers` folder of your godot_arcore repository

  `cp godot/bin/api.json godot_arcore/plugin/libs/godot-cpp/godot-headers`

- Navigate to the java folder and generate the Godot templates

  `cd godot/platform/android/java && ./gradlew :generateGodotTemplates`

- Generate the binding classes for c++

  `cd godot_arcore/plugin/libs/godot-cpp && python ../../../generate.py`

- Compile the plugin from inside the godot_arcore repository

  `./gradlew :generatePluginBinary`

> Once the build sucessfully completes the binary files can be found in  
> `godot_arcore/plugin/build/outputs/aar` and should also be in the `demo/android/plugins` folder

- Copy `godot_arcore/plugin/gdarcore.gdap` into the `demo/android/plugins` folder of your godot_arcore repository. There should also be two .aar files

### 3. Setting up the demo in the Editor

- Open your compiled Godot Engine from `godot/bin/godot.<your-platform>.tools.64`

> Maybe add a shortcut to your desktop for easier access

#### Installing Android Build Template

Open the demo project from `godot_arcore/demo` with the compiled engine.

Remove any previous presets and add a new Export preset for Android (`Project -> Export...`).  
Go to `Project -> Install Android Build Template -> Install From File` and choose the template from `godot/bin/android_source.zip`

#### Editor Settings

- Under `Editor -> Editor Settings ...` search for "Android" and set the Android SDK Path:
  - For Linux it will most likely be in your home folder under `Android/Sdk`
  - For Windows, it will most likely be `C:\Programs\Android\Sdk\`

### Export Settings

- Open `Project -> Export...` and click on your export preset.
- Under `Custom Template` navigate to `godot/bin/android_debug.apk` for the Debug Template and `godot/bin/android_release.apk` for the Release Template respectively.
- Under the `Custom Build` section enable `Use Custom Build`
- Under `Custom Build` set the Min SDK to 24
- Under `Plugins` enable the `Ar Core Plugin`
- Under `XRFeatures` set the `XR Mode` to `ARCore`

You are now ready to build and test the demo on your device.

## Debug

- You can open the resulting apk after a sucessful build in Android Studio to debug with the available tools there and connect with source code / debug symbols:

### Paths for easier access

Not working and still throws me into a decompiled view.

**libgodot_android.so:**

- `godot/platform/android/java/lib/libs/dev/arm64-v8a/libgodot_android.so`

**libgodot_arcore.so:**

- `godot_arcore/plugin/build/intermediates/merged_native_libs/debug/out/lib/arm64-v8a/libgodot_arcore.so`

## Gotchas

The NDK version in the make_this_work branches README says "21.1.6528147", but the `config.gradle` says "21.4.7075529"
