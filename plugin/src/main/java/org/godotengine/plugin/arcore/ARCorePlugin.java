package org.godotengine.plugin.arcore;

import java.util.*;

import android.app.Activity;
import android.view.View;

import androidx.annotation.NonNull;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

public class ARCorePlugin extends GodotPlugin {
	static {
		System.loadLibrary("godot_arcore");
	}

	public ARCorePlugin(Godot godot) {
		super(godot);
	}

	@Override
	public View onMainCreate(Activity activity) {
		if (ARCoreUtil.shouldSetupARCore(activity, getGodot().getXRMode())) {
			ARCoreUtil.setupARCore(activity, activity.getIntent());
		}

		return null;
	}

	@NonNull
	@Override
	public String getPluginName() {
		return "ARCorePlugin";
	}

	@NonNull
	@Override
	public Set<String> getPluginGDExtensionLibrariesPaths() {
		return Set.of("addons/godot_arcore/godot_arcore.gdextension");
	}

	private native void initializeWrapper();

    private native void uninitializeWrapper();
}
