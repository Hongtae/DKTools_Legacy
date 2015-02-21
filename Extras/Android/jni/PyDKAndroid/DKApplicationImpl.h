//
//  File: DKApplicationImpl.h
//  Encoding: UTF-8 ☃
//  Platform: Android
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2013,2014 ICONDB.COM. All rights reserved.
//

#pragma once

#ifdef __ANDROID__
#include <DKFramework/Interface/DKApplicationInterface.h>
using namespace DKFoundation;
using namespace DKFramework;

class DKApplicationImpl : public DKApplicationInterface
{
public:
	DKApplicationImpl(DKApplication* app);
	~DKApplicationImpl(void);

	int Run(DKFoundation::DKArray<char*>& args);
	void Terminate(int exitCode);

	void PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone);

	DKFoundation::DKLogger& DefaultLogger(void);
	DKFoundation::DKString EnvironmentPath(SystemPath);
	DKFoundation::DKString ModulePath(void);

	DKFoundation::DKObject<DKFoundation::DKData> LoadResource(const DKFoundation::DKString& res, DKFoundation::DKAllocator& alloc);
	DKFoundation::DKObject<DKFoundation::DKData> LoadStaticResource(const DKFoundation::DKString& res);

	DKRect DisplayBounds(int displayId) const;
	DKRect ScreenContentBounds(int displayId) const;

	DKFoundation::DKString HostName(void) const;
	DKFoundation::DKString OSName(void) const;
	DKFoundation::DKString UserName(void) const;

	DKApplication* mainApp;
};

#endif
