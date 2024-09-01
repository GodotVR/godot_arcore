// TODO: Update to match your plugin's package name.
package org.godotengine.plugin.android.arcore

import android.app.Activity
import android.content.pm.PackageManager
import android.util.Log
import android.view.View
import android.widget.Toast
import com.google.ar.core.ArCoreApk
import com.google.ar.core.Config
import com.google.ar.core.Frame
import com.google.ar.core.HitResult
import com.google.ar.core.Plane
import com.google.ar.core.Pose
import com.google.ar.core.Session
import com.google.ar.core.exceptions.CameraNotAvailableException
import com.google.ar.core.exceptions.TextureNotSetException
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException
import org.godotengine.godot.Godot
import org.godotengine.godot.gl.GLSurfaceView
import org.godotengine.godot.gl.GodotRenderer
import org.godotengine.godot.plugin.GodotPlugin
import org.godotengine.godot.plugin.UsedByGodot
import org.godotengine.godot.utils.PermissionsUtil
import java.nio.IntBuffer
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

class ARCorePlugin(godot: Godot): GodotPlugin(godot) {

    companion object {
        val TAG = ARCorePlugin::class.java.simpleName // TAG = "ARCorePlugin"

        // Installation and Setup Flow
        var userRequestedInstall: Boolean = false
        var arCoreInstalled: Boolean = false
        var arCoreSupported: Boolean = false
        var cameraPermissionGranted : Boolean = false

        var session : Session? = null
        private var surfaceView: GLSurfaceView? = null
        private var hasSetTextureNames: Boolean = false
        private var cameraTextureId: Int = -1
        private var textureBuffer: IntBuffer? = null
        private var viewportWidth: Int = -1
        private var viewportHeight: Int = -1
        private var viewportChanged: Boolean = false

        init {
            try {
                Log.d(TAG, "Loading ${BuildConfig.GODOT_PLUGIN_NAME} library")
                System.loadLibrary(BuildConfig.GODOT_PLUGIN_NAME)
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Unable to load ${BuildConfig.GODOT_PLUGIN_NAME} shared library")
            }
        }
    }

    override fun onMainCreate(activity: Activity): View? {
        super.onMainCreate(activity)
        Log.d(TAG, "onMainCreate()")

        // 1. Check if ARCore is available and/or can be installed
        Log.d(TAG, "checkAvailability")
        when(ArCoreApk.getInstance().checkAvailability(activity)) {
            ArCoreApk.Availability.SUPPORTED_APK_TOO_OLD, ArCoreApk.Availability.SUPPORTED_NOT_INSTALLED -> {
                Log.d(TAG, "ARCoreApk.Availability: SUPPORTED_NOT_INSTALLED or SUPPORTED_APK_TOO_OLD. Please install or update ARCore")
                arCoreSupported = true
            }
            ArCoreApk.Availability.SUPPORTED_INSTALLED -> {
                Log.d(TAG, "ARCoreApk.Availability: SUPPORTED_INSTALLED")
                arCoreInstalled = true
                arCoreSupported = true
            }
            else -> {
                Log.d(TAG, "ARCoreApk.Availability: Not SUPPORTED_*")
                Toast.makeText(activity, "ARCore support can't be determined, AR features probably won't work", Toast.LENGTH_LONG).show()
            }
            // the else branch represents all these (non)-availabilities:
            /*ArCoreApk.Availability.UNKNOWN_CHECKING -> Log.d(TAG, "ARCoreApk.Availability: UNKNOWN_CHECKING")
            ArCoreApk.Availability.UNKNOWN_ERROR -> Log.d(TAG, "ARCoreApk.Availability: UNKNOWN_ERROR")
            ArCoreApk.Availability.UNKNOWN_TIMED_OUT -> Log.d(TAG, "ARCoreApk.Availability: UNKNOWN_TIMED_OUT")
            ArCoreApk.Availability.UNSUPPORTED_DEVICE_NOT_CAPABLE -> Log.d(TAG, "ARCoreApk.Availability: UNSUPPORTED_DEVICE_NOT_CAPABLE")*/
        }

        // These are placeholders needed to get ARCore to report hit tests
        // Will be removed later
        surfaceView = GLSurfaceView(activity)
        surfaceView!!.setRenderer(GodotRenderer())

        return null
    }

    // Called once at the start and upon every reopening of the app after
    // sending it to the background e.g. when installing ARCore from the Play Store
    override fun onMainResume() {
        super.onMainResume()
        Log.d(TAG, "onMainResume")

        if(arCoreSupported && !arCoreInstalled) {
            try {
                when(ArCoreApk.getInstance().requestInstall(activity, userRequestedInstall)) {
                    ArCoreApk.InstallStatus.INSTALLED -> {
                        Log.d(TAG, "ARCore is installed. Requesting camera permission now")
                        arCoreSupported = true
                        arCoreInstalled = true
                    }
                    ArCoreApk.InstallStatus.INSTALL_REQUESTED -> userRequestedInstall = true
                }
            } catch (e: UnavailableUserDeclinedInstallationException) {
                Toast.makeText(activity, "UnavailableUserDeclinedInstallationException: Please install ARCore to use this app", Toast.LENGTH_LONG).show()
                Log.d(TAG, "UnavailableUserDeclinedInstallationException: Please install ARCore to use this app")
                //userRequestedInstall = false
                // How should the setup flow continue here?
            } catch (e: UnavailableDeviceNotCompatibleException) {
                Toast.makeText(activity, "UnavailableDeviceNotCompatibleException: Your device is not compatible with ARCore", Toast.LENGTH_LONG).show()
                Log.d(TAG, "UnavailableDeviceNotCompatibleException: Your device is not compatible with ARCore")
                // How should the setup flow continue here?
                //userRequestedInstall = false
            }
        }

        // Check if the camera permission is already granted or not
        cameraPermissionGranted = checkCameraPermission()

        // ARCore is supported and installed, but the camera permission is not yet granted
        if((arCoreSupported && arCoreInstalled) && !cameraPermissionGranted) {
            Log.d(TAG, "Requesting camera permissions")
            PermissionsUtil.requestManifestPermissions(activity)
        }

        // Everything set up, create or resume session
        /*if(arCoreSupported && arCoreInstalled && cameraPermissionGranted) {
            if (session == null) {
                session = createSession()
                session!!.resume()
            } else {
                // Note that order of resuming matters - see the note in onMainPause(), the reverse applies here.
                session!!.resume()
                surfaceView!!.onResume()
            }
        }*/
    }

    fun checkCameraPermission(): Boolean {
        Log.d(TAG, "checkCameraPermission")
        val grantedPermissions: Array<String> = PermissionsUtil.getGrantedPermissions(activity)
        if(grantedPermissions.isNotEmpty()) {
            grantedPermissions.forEach { permission ->
                if (permission == "android.permission.CAMERA") {
                    return true
                }
            }
        }
        return false
    }

    override fun onMainPause() {
        super.onMainPause()
        Log.d(TAG, "onMainPause()")

        if(session != null) {
            // Note that the order of pausing matters - GLSurfaceView is paused first so that it does not try
            // to query the session. If Session is paused before GLSurfaceView, GLSurfaceView may
            // still call session.update() and get a SessionPausedException.
            surfaceView!!.onPause()
            session!!.pause()
        }
    }

    override fun onMainDestroy() {
        Log.d(TAG, "onMainDestroy")
        if(session != null) {
            // Explicitly close ARCore Session to release native resources.
            // Review the API reference for important considerations before calling close() in apps with
            // more complicated lifecycle requirements:
            // https://developers.google.com/ar/reference/java/arcore/reference/com/google/ar/core/Session#close()
            session!!.close()
            session = null
        }
        super.onMainDestroy()
    }

    override fun onMainRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>?,
        grantResults: IntArray?
    ) {
        super.onMainRequestPermissionsResult(requestCode, permissions, grantResults)
        //Log.d(TAG, requestCode.toString()) // "1001" -> comes from PermissionUtil

        permissions?.forEachIndexed { index, permission ->
            // Check if the camera permission was granted and resume in onMainResume()
            if(permission.equals("android.permission.CAMERA") && grantResults?.get(index) == PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "Camera permission is granted")
                cameraPermissionGranted = true
            }
        }
    }

    // Creates a session and configures it (https://developers.google.com/ar/develop/java/session-config)
    fun createSession(): Session {
        val session = Session(activity)
        val config = Config(session)

        // Do feature-specific operations here, such as enabling depth or turning on
        // support for Augmented Faces

        session.configure(config)
        //Log.d(TAG, session.toString())

        return session
    }

    override fun onGLSurfaceChanged(gl: GL10?, width: Int, height: Int) {
        super.onGLSurfaceChanged(gl, width, height)
        onSurfaceChanged(width, height)
    }

    override fun onGLSurfaceCreated(gl: GL10?, config: EGLConfig?) {
        super.onGLSurfaceCreated(gl, config)
        textureBuffer = IntBuffer.wrap(intArrayOf(100*100))
        gl!!.glGenTextures(1, textureBuffer )
        gl.glBindTexture(GL10.GL_TEXTURE_2D, cameraTextureId)
    }

    fun updateSessionIfNeeded(session: Session) {
        if (viewportChanged) {
            //val displayRotation: Int = display.rotation
            // hardcoded values for the display width and height here for now...
            session.setDisplayGeometry(0, 1080, 2400)
            viewportChanged = false
        }
    }

    override fun onGLDrawFrame(gl: GL10?) {
        //Log.d(TAG,"onGLDrawFrame")

        // Do we call the super up here or later?
        super.onGLDrawFrame(gl)

        if(session == null) {
            return
        }

        updateSessionIfNeeded(session!!)

        if (!hasSetTextureNames) {
            session!!.setCameraTextureNames(intArrayOf(cameraTextureId))
            hasSetTextureNames = true;
        }

        var frame: Frame
        var trackables: Collection<Plane>

        try {
            var displayRotation: Int = 0 //display.rotation
            session!!.setDisplayGeometry(displayRotation, viewportWidth, viewportHeight)

            frame = session!!.update()

            trackables = frame.getUpdatedTrackables(Plane::class.java)
            var resultString = ""
            if (trackables.isNotEmpty()) {
                trackables.forEach() { trackable ->
                    resultString += "$trackable, "
                }
                Log.d(TAG, resultString)
            }

            var hitResultList: List<HitResult>  = frame.hitTest(100.0F, 100.0F)

            if(hitResultList.isNotEmpty()) {
                var nearestHit = hitResultList[0]

                var distanceToCamera: Float = nearestHit.distance
                var pose: Pose = nearestHit.hitPose
                Log.d(TAG, "Distance from camera to hit: $distanceToCamera")
                Log.d(TAG, "Pose of the hit: $pose")
            }

        } catch(e: CameraNotAvailableException) {
            Log.e(TAG, "ARCorePlugin: Camera was not available during onGLDrawFrame", e)
        } catch(e: TextureNotSetException) {
            Log.d(TAG, "ARCorePlugin: The texture was not set")
        }
    }

    fun onSurfaceChanged(width: Int, height: Int) {
        viewportWidth = width
        viewportHeight = height
        viewportChanged = true
    }

    override fun getPluginName() = BuildConfig.GODOT_PLUGIN_NAME

    override fun getPluginGDExtensionLibrariesPaths() = setOf("res://addons/${BuildConfig.GODOT_PLUGIN_NAME}/plugin.gdextension")

    @UsedByGodot
    private external fun initializeEnvironment()

    @UsedByGodot
    private external fun uninitializeEnvironment()
}
