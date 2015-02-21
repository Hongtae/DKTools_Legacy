package com.icondb;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.os.Bundle;
import android.os.Looper;
import android.app.Activity;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.Toast;

public class DKActivity extends Activity {

	private native boolean bindNativeWindowManager();
	private native void unbindNativeWindowManager();
	private native void closeNativeWindows();

	private void logContentRes(View v) {
		int contentWidth = v.getWidth();
		int contentHeight = v.getHeight();
		int measuredWidth = v.getMeasuredWidth();
		int measuredHeight = v.getMeasuredHeight();

		Log.d("DKActivity", "content: " + contentWidth + " x " + contentHeight
				+ " measured: " + measuredWidth + " x " + measuredHeight);
	}

	public boolean extractAssetFile(String assetPath, String copyPath) {
		AssetManager asset = this.getAssets();
		boolean copied = false;
		try {
			InputStream input = asset.open(assetPath);
			OutputStream output = new FileOutputStream(copyPath);

			byte buffer[] = new byte[65536];
			long bytesTotal = 0;
			int bytesRead = 0;
			while ((bytesRead = input.read(buffer)) != -1) {
				output.write(buffer, 0, bytesRead);
				bytesTotal += bytesRead;
			}
			output.flush();
			output.close();
			input.close();

			copied = true;
			Log.i("DKActivity", "assets:" + assetPath + " Copied to "
					+ copyPath + " (" + bytesTotal + " bytes)");
		} catch (IOException e) {
			e.printStackTrace();
		}
		return copied;
	}

	private FrameLayout mainLayout = null;

	public View getMainView() {
		return mainLayout;
	}

	final DKSurfaceView createSurfaceView(int x, int y, int w, int h, boolean resizable) {
		Log.d("DKActivity", "createSurfaceView(" + x + "," + y + "," + w + "," + h + "," + resizable + ")");

		// x <= 0 && y <= 0 && resizable == true 이면 전체화면으로 채운다.
		boolean fullscreen = resizable && w <= 0 && h <= 0;
		
		final FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(0, 0);
		if (fullscreen) {		// 전체화면.
			layoutParams.width = ViewGroup.LayoutParams.MATCH_PARENT;
			layoutParams.height = ViewGroup.LayoutParams.MATCH_PARENT;
			layoutParams.setMargins(0, 0, 0, 0);
		} else {
			layoutParams.width = Math.max(w, 0);
			layoutParams.height = Math.max(h, 0);
			layoutParams.setMargins(x, y, 0, 0);
		}

		class Holder {
			DKSurfaceView view = null;
		}
		final Holder holder = new Holder();
		this.runOnUiThread(new Runnable() {
			public void run() {
				synchronized (holder) {
					holder.view = new DKSurfaceView(DKActivity.this);
					holder.view.setLayoutParams(layoutParams);					
					holder.view.setVisibility(View.INVISIBLE);
					mainLayout.addView(holder.view);
					holder.notify();
				}
			}
		});
		synchronized (holder) {
			try {
				while (holder.view == null)
					holder.wait();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
		return holder.view;
	}

	final void closeSurfaceView(final DKSurfaceView view) {
		Log.d("DKActivity", "closeSurfaceView");
		Runnable r = new Runnable() {
			public void run() {
				synchronized (this) {
					view.detachFromNative();
					mainLayout.removeView(view);
					this.notify();
				}
			}
		};

		if (Looper.getMainLooper().getThread() == Thread.currentThread()) {
			r.run();
		} else {
			this.runOnUiThread(r);
			synchronized (r) {
				try {
					r.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		DK.init(this);
		if (bindNativeWindowManager() == false)
			throw new RuntimeException("bindNativeWindowManager failed.");

		this.mainLayout = new FrameLayout(this);
		setContentView(this.mainLayout);

		this.mainLayout.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
					@Override
					public void onLayoutChange(View v, int left, int top,
							int right, int bottom, int oldLeft, int oldTop,
							int oldRight, int oldBottom) {

						Log.v("DKActivity", "View.OnLayoutChangeListener");
						DKActivity.this.logContentRes(v);
						DK.setContentResolution(left, top, right - left,
								bottom - top);
					}
				});
	}

	protected void closeAllWindows() {
		this.closeNativeWindows();
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		Log.d("DKActivity", "onDestory!");
		//logContentRes(getWindow().getDecorView());
		unbindNativeWindowManager();
		DK.release();
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		Log.d("DKActivity", "onConfigurationChanged");

		// Checks the orientation of the screen
		if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
			Toast.makeText(this, "landscape", Toast.LENGTH_SHORT).show();
		} else if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
			Toast.makeText(this, "portrait", Toast.LENGTH_SHORT).show();
		}
	}
	
	final void processAppOperationsOnUiThread() {
		this.runOnUiThread(new Runnable() {
			public void run() {
				DK.processAppOperations();
			}			
		});
	}
}
