package com.icondb;

import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class DKSurfaceView extends SurfaceView implements
		SurfaceHolder.Callback {

	DKSurfaceView(Context context) {
		super(context);
		init();
	}

	DKSurfaceView(Context context, AttributeSet attrs) {
		super(context, attrs);
		init();
	}

	DKSurfaceView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init();
	}

	private void init() {
		this.getHolder().setFormat(PixelFormat.RGB_888);
		this.getHolder().addCallback(this);
	}
	
	@Override
	protected void onAttachedToWindow() {
		Log.d("DKSurfaceView", "onAttachedToWindow()");
		super.onAttachedToWindow();
	}

	@Override
	protected void onWindowVisibilityChanged(int visibility) {
		Log.d("DKSurfaceView", "onWindowVisibilityChanged(" + visibility + ") (nativeKey:" + this.nativeWindowKey + ")");
		super.onWindowVisibilityChanged(visibility);
		if (this.nativeWindowKey != 0)
		{
			this.nativeWindowVisibilityChanged(visibility == View.VISIBLE);
			if (visibility == View.VISIBLE)
				this.nativeWindowUpdate();
		}
	}
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		super.onTouchEvent(event);

		final int nativeMouseDown = 0;
		final int nativeMouseMove = 1;
		final int nativeMouseUp = 2;
		
		final int viewHeight = getHeight(); // to inverse y-axis

		final int action = event.getAction();		
		if ((action & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_MOVE) {
			for (int index = 0; index < event.getPointerCount(); ++index) {
				final int pid = event.getPointerId(index);
				final float x = event.getX(index);
				final float y = viewHeight - event.getY(index);

				nativeWindowTouchEvent(pid, nativeMouseMove, x, y);
			}
		} else {
			final int index = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
			final int pid = event.getPointerId(index);
			final float x = event.getX(index);
			final float y = viewHeight - event.getY(index);

			switch (action & MotionEvent.ACTION_MASK) {
			case MotionEvent.ACTION_POINTER_DOWN:
			case MotionEvent.ACTION_DOWN:
				nativeWindowTouchEvent(pid, nativeMouseDown, x, y);
				break;
			case MotionEvent.ACTION_POINTER_UP:
			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_CANCEL:
				nativeWindowTouchEvent(pid, nativeMouseUp, x, y);
				break;
			}
		}
				
		return true;
	}
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.d("DKSurfaceView", "surfaceCreated()");
		if (this.nativeWindowKey != 0)
			if (this.nativeWindowSurfaceCreated(holder.getSurface()) == false)
				throw new RuntimeException("native error");
	}

	@Override
	public	void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		Log.d("DKSurfaceView", "surfaceChanged(" + width + "," + height + ")");
		if (this.nativeWindowKey != 0)
			if (this.nativeWindowSurfaceChanged(width, height) == false)
				throw new RuntimeException("native error");
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.d("DKSurfaceView", "surfaceDestroyed()");
		if (this.nativeWindowKey != 0)
			if (this.nativeWindowSurfaceDestroyed() == false)
				throw new RuntimeException("native error");
	}

	final void detachFromNative() {
		Log.d("DKSurfaceView", "detachFromNative()");
		this.nativeWindowKey = 0;
		this.getHolder().removeCallback(this);
	}

	public void activate() {
		handler.post(new Runnable() {
			public void run() {
				Log.d("DKSurfaceView", "activate()");
				DKSurfaceView.this.getParent().bringChildToFront(DKSurfaceView.this);
				DKSurfaceView.this.setVisibility(View.VISIBLE);
				DKSurfaceView.this.nativeWindowUpdate();
			}
		});	
	}
	public void minimize() {
		handler.post(new Runnable() {
			public void run() {
				Log.d("DKSurfaceView", "minimize()");
				DKSurfaceView.this.setVisibility(View.INVISIBLE);
			}
		});		
	}
	public void show() {
		handler.post(new Runnable() {
			public void run() {
				Log.d("DKSurfaceView", "show()");
				DKSurfaceView.this.setVisibility(View.VISIBLE);
				DKSurfaceView.this.nativeWindowUpdate();
			}
		});
	}
	public void hide() {
		handler.post(new Runnable() {
			public void run() {
				Log.d("DKSurfaceView", "hide()");
				DKSurfaceView.this.setVisibility(View.INVISIBLE);
			}
		});		
	}
	public void resize(int w, int h) {
		handler.post(new Runnable() {
			public void run() {
				Log.e("DKSurfaceView", "resize(w,h) not implemented!");
			}
		});		
	}
	public void resize(int x, int y, int w, int h) {
		handler.post(new Runnable() {
			public void run() {
				Log.e("DKSurfaceView", "resize(x,y,w,h) not implemented!");
			}
		});
	}
	public void setOrigin(final int x, final int y) {
		handler.post(new Runnable() {
			public void run() {
				Log.d("DKSurfaceView", "setOrigin(" + x + ", " + y + ")");
				DKSurfaceView.this.setX(x);
				DKSurfaceView.this.setY(y);
				DKSurfaceView.this.nativeWindowUpdate();
			}
		});		
	}
	public void setTitle(String title) {
		Log.d("DKSurfaceView", "setTitle(" + title + ")");
		synchronized(this)
		{
			this.viewTitle = title;
		}
	}
	public String getTitle() {
		Log.d("DKSurfaceView", "getTitle()");
		synchronized (this) {
			if (this.viewTitle != null)
				return this.viewTitle;
		}
		return "";
	}
	
	private static Handler handler = new Handler(Looper.getMainLooper());
	private native boolean nativeWindowSurfaceCreated(Surface surface);
	private native boolean nativeWindowSurfaceChanged(int w, int h);
	private native boolean nativeWindowSurfaceDestroyed();
	private native void nativeWindowUpdate();
	private native void nativeWindowVisibilityChanged(boolean visible);
	private native void nativeWindowTouchEvent(int id, int act, float x, float y);
	private long nativeWindowKey = 0;
	private String viewTitle = "";
}
