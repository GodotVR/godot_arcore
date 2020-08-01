package org.godotengine.plugin.arcore;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;
import com.google.ar.core.ArCoreApk;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;
import org.godotengine.godot.Godot;

/**
 * Activity used to setup ARCore prior to initializing the Godot engine.
 */
public class ARCoreSetupActivity extends Activity {

	static final String PREVIOUS_ACTIVITY_START_INTENT_KEY = "previous_activity_start_intent";

	private static boolean requestARCoreInstall = true;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(new View(this));
	}

	@Override
	public void onResume() {
		super.onResume();

		// Check that ARCore is installed and up-to-date.
		try {
			switch (ArCoreApk.getInstance().requestInstall(this, requestARCoreInstall)) {
				case INSTALLED:
					break;
				case INSTALL_REQUESTED:
					requestARCoreInstall = false;
					return;
			}

			// Request CAMERA permission if needed.
			if (!ARCoreUtil.hasCameraPermission(this)) {
				ARCoreUtil.requestCameraPermission(this);
				return;
			}

			// We have everything we need. Let's restart the original activity.
			Intent previousIntent = null;
			if (getIntent() != null) {
				previousIntent = getIntent().getParcelableExtra(PREVIOUS_ACTIVITY_START_INTENT_KEY);
			}
			if (previousIntent == null) {
				previousIntent = new Intent(this, Godot.class);
			}

			startActivity(previousIntent);

		} catch (UnavailableUserDeclinedInstallationException | UnavailableDeviceNotCompatibleException e) {
			Toast.makeText(this, R.string.setup_arcore_request, Toast.LENGTH_LONG).show();
		}

		finish();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		if (ARCoreUtil.isARCoreRequestPermissionCode(requestCode)) {
			if (!ARCoreUtil.hasCameraPermission(this)) {
				Toast.makeText(this, R.string.missing_camera_permission_warning, Toast.LENGTH_LONG).show();
				if (!ARCoreUtil.shouldShowRequestPermissionRationale(this)) {
					// Permission denied with checking "Do not ask again".
					ARCoreUtil.launchPermissionSettings(this);
				}
				finish();
			}
		}
	}
}
