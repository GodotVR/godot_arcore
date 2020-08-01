package org.godotengine.plugin.arcore;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.provider.Settings;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.google.ar.core.ArCoreApk;
import org.godotengine.godot.xr.XRMode;

/**
 * AR Core related utility methods.
 */
public final class ARCoreUtil {

	private static final int CAMERA_PERMISSION_CODE = 4324;
	private static final String CAMERA_PERMISSION = Manifest.permission.CAMERA;

	private ARCoreUtil() {
	}

	/**
     * Returns true if we should start the process of setting up ARCore for the app.
     * <p>
     * This usually occurs when {@link XRMode#ARCORE} is the XR mode, but CAMERA permission
     * is missing or ARCore is not installed or up-to-date.
     */
	public static boolean shouldSetupARCore(Activity activity, XRMode xrMode) {
		// Validate we have the correct XR mode.
		if (XRMode.ARCORE != xrMode) {
			return false;
		}

		// Check if the CAMERA permission is available.
		if (!hasCameraPermission(activity)) {
			return true;
		}

		// Check that ARCore is installed and up-to-date.
		return ArCoreApk.getInstance().checkAvailability(activity) != ArCoreApk.Availability.SUPPORTED_INSTALLED;
	}

	/**
     * Launches the {@link ARCoreSetupActivity} to initiate the setup process for ARCore.
     */
	public static void setupARCore(Activity activity, Intent previousActivityStartIntent) {
		activity.startActivity(new Intent(activity, ARCoreSetupActivity.class)
									   .putExtra(ARCoreSetupActivity.PREVIOUS_ACTIVITY_START_INTENT_KEY, previousActivityStartIntent));
		activity.finish();
	}

	static boolean isARCoreRequestPermissionCode(int requestCode) {
		return requestCode == CAMERA_PERMISSION_CODE;
	}

	/** Check to see we have the necessary permissions for this app. */
	static boolean hasCameraPermission(Activity activity) {
		return ContextCompat.checkSelfPermission(activity, CAMERA_PERMISSION) == PackageManager.PERMISSION_GRANTED;
	}

	/** Check to see we have the necessary permissions for this app, and ask for them if we don't. */
	static void requestCameraPermission(Activity activity) {
		ActivityCompat.requestPermissions(
				activity, new String[] { CAMERA_PERMISSION }, CAMERA_PERMISSION_CODE);
	}

	/** Check to see if we need to show the rationale for this permission. */
	static boolean shouldShowRequestPermissionRationale(Activity activity) {
		return ActivityCompat.shouldShowRequestPermissionRationale(activity, CAMERA_PERMISSION);
	}

	/** Launch Application Setting to grant permission. */
	static void launchPermissionSettings(Activity activity) {
		Intent intent = new Intent();
		intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
		intent.setData(Uri.fromParts("package", activity.getPackageName(), null));
		activity.startActivity(intent);
	}
}
