package org.godotengine.plugin.arcore;

import android.app.Activity;
import android.view.View;

import androidx.annotation.NonNull;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

public class ARCorePlugin extends GodotPlugin {

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
}
