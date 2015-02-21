package com.icondb;

import android.util.Log;

public class PyDKInterpreter {

	interface OnRequestInput {
		String onRequestInput(String prompt);
	}

	interface OnOutputCallback {
		void onOutputCallback(String mesg);
	}

	interface OnErrorCallback {
		void onErrorCallback(String err);
	}

	private OnRequestInput inputReq = null;
	private OnOutputCallback outputCb = null;
	private OnErrorCallback errorCb = null;

	public PyDKInterpreter() {
		if (this.bindNative() == false)
			throw new RuntimeException("PyDKInterpreter registration failed.");

		Log.d("PyDKInterpreter", "Registration Key: "
				+ this.nativeInterpreterKey);
	}

	public boolean runString(String s) {
		Log.d("PyDKInterpreter", "runString (thread:" + Thread.currentThread().getId() + ")");
		return this.runStringNative(s);
	}

	private void printOutput(String s) {
		if (outputCb != null)
			outputCb.onOutputCallback(s);
		else
			Log.d("PyDKInterpreter.out["+Thread.currentThread().getId()+"]", "" + s);
	}

	private void printError(String s) {
		if (errorCb != null)
			errorCb.onErrorCallback(s);
		else
			Log.e("PyDKInterpreter.err["+Thread.currentThread().getId()+"]", "" + s);
	}

	private String requestInput(String prompt) {
		if (inputReq != null)
			return inputReq.onRequestInput(prompt);
		else
			return "Hello World from JAVA! (thread:" + Thread.currentThread().getId() + ")";
	}

	@Override
	protected void finalize() throws Throwable {
		if (this.nativeInterpreterKey != 0)
			this.unbindNative();
		super.finalize();
	}

	private long nativeInterpreterKey = 0;
	private native boolean bindNative();
	private native boolean runStringNative(String s);
	private native void unbindNative();
}
