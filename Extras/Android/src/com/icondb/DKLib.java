package com.icondb;

import android.content.Context;
import android.graphics.Point;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

public final class DK {

	static {
		Log.d("DK", "Loading DK...");
		System.loadLibrary("OpenAL");
		System.loadLibrary("PyDKAndroid");
	}

	// DK native functions
	static native boolean setTemporaryDirectory(String dir);
	static native void setEnvironmentPath(String env, String path);
	static native void setModulePath(String path);
	static native boolean setResourcePath(String path, String prefix);
	static native void setDisplayResolution(int x, int y, int w, int h);
	static native void setContentResolution(int x, int y, int w, int h);
	
	private static native boolean initApp();
	private static native void releaseApp();

	public static native void processAppOperations();

	// PyDK
	public static native boolean initPython(String appName, String[] paths);
	public static native boolean isPythonInitialized();
	public static native void releasePython();

	private static Context context = null;

	public static synchronized void init(Context ctxt) {
		if (ctxt == null) {
			Log.e("DK", "DK.init(): invalid context.");
			throw new RuntimeException("DK invalid context!");
		} else if (ctxt == context) {
			Log.v("DK", "DK.init(): update context!");
			return;
		} else if (context != null) {
			Log.e("DK", "DK.init(): context is not null");
			throw new RuntimeException("DK initialized already!");
		}

		DK.context = ctxt;
		Log.v("DK", "init DK");

		setTemporaryDirectory(context.getCacheDir().getAbsolutePath());
		setModulePath(context.getPackageCodePath());
		setResourcePath(context.getPackageResourcePath(), "assets/");
		
		setEnvironmentPath("SystemRoot", "/");
		setEnvironmentPath("AppRoot", context.getFilesDir().getAbsolutePath());
		setEnvironmentPath("AppResource", context.getPackageResourcePath() + "/assets/");
		setEnvironmentPath("AppExecutable", context.getPackageCodePath());
		setEnvironmentPath("AppData", context.getDir("DK.AppData", Context.MODE_PRIVATE).getAbsolutePath());
		setEnvironmentPath("UserHome", context.getDir("DK.UserHome", Context.MODE_PRIVATE).getAbsolutePath());
		setEnvironmentPath("UserDocuments", context.getFilesDir().getAbsolutePath());
		setEnvironmentPath("UserPreferences", context.getCacheDir().getAbsolutePath());
		setEnvironmentPath("UserCache", context.getCacheDir().getAbsolutePath());
		setEnvironmentPath("UserTemp", context.getCacheDir().getAbsolutePath());
		
		// 스크린 해상도 가져오기.
		WindowManager wm = (WindowManager) context
				.getSystemService(Context.WINDOW_SERVICE);
		Display display = wm.getDefaultDisplay();
		Point screenSize = new Point();
		display.getSize(screenSize);

		Log.d("DK", "ScreenSize: " + screenSize.x + " x " + screenSize.y);
		setDisplayResolution(0, 0, screenSize.x, screenSize.y);
		initApp();
	}

	public static synchronized Point getScreenSize() {
		// 스크린 해상도 가져오기.
		WindowManager wm = (WindowManager) context
				.getSystemService(Context.WINDOW_SERVICE);
		Display display = wm.getDefaultDisplay();
		Point size = new Point();
		display.getSize(size);
		return size;
	}

	public static synchronized void release() {
		if (context == null) {
			Log.v("DK", "Release: context is null");
		} else {
			// stop service!!
			releasePython();
			releaseApp();
			context = null;
			Log.v("DK", "release DK");
		}
	}

	public static synchronized boolean isInitialized() {
		return context != null;
	}
}
