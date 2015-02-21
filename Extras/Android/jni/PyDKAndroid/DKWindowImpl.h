//
//  File: DKWindowImpl.h
//  Encoding: UTF-8 ☃
//  Platform: Android
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013,2014 ICONDB.COM. All rights reserved.
//

#pragma once

#ifdef __ANDROID__
#include <jni.h>
#include <android/native_window.h>
#include <DKFramework/Interface/DKWindowInterface.h>

using namespace DKFoundation;
using namespace DKFramework;

class DKWindowImpl : public DKWindowInterface
{
public:
	DKWindowImpl(DKWindow *window);
	~DKWindowImpl(void);
	bool CreateProxy(void* systemHandle);		// 이미 존재하는 윈도우에 붙는다.
	void UpdateProxy(void);						// 프록시 윈도우의 크기를 갱신한다.
	bool IsProxy(void) const;
	bool Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style);
	void Destroy(void);
	void Show(void);
	void Hide(void);
	void Activate(void);
	void Minimize(void);
	void Resize(const DKSize&, const DKPoint*);
	void SetOrigin(const DKPoint&);
	void SetTitle(const DKString& title);
	DKString Title(void) const;
	bool IsVisible(void) const;
	void* PlatformHandle(void) const;
	void ShowMouse(int deviceId, bool bShow);
	bool IsMouseVisible(int deviceId) const;
	void HoldMouse(int deviceId, bool bHold);
	bool IsMouseHeld(int deviceId) const;
	void SetMousePosition(int deviceId, const DKPoint& pt);
	DKPoint MousePosition(int deviceId) const;
	void EnableTextInput(int deviceId, bool bTextInput);
	bool IsValid(void) const;

	DKObject<DKOperation> DestroyOperation(JNIEnv*);

	DKMap<int, DKPoint> mousePositionMap;

	DKWindow* window;
	jobject objectRef;
	ANativeWindow* nativeWindow;
	DKSize nativeWindowSize;
	DKPoint nativeWindowPos;
	DKSpinLock nativeWindowLock;
};

#endif // ifdef __ANDROID__
