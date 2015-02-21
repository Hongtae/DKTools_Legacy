//
//  MediaView.m
//  DKMediaPlayer
//
//  Created by tiff on 11. 6. 27..
//  Copyright 2011 ICONDB.COM. All rights reserved.
//


#include "../Frame/MediaFrame.h"
#import "MediaView.h"

@interface MediaViewContext : NSObject
{
	DKObject<DKApplication> _app;
	DKObject<MediaFrame>	_frame;
	DKObject<DKScreen>		_screen;
	DKObject<DKWindow>		_window;		
}
@property (nonatomic, readwrite, assign) MediaFrame* frame;
@property (nonatomic, readwrite, assign) DKScreen* screen;
@property (nonatomic, readwrite, assign) DKWindow* window;
@end
@implementation MediaViewContext
@dynamic frame;
@dynamic screen;
@dynamic window;

- (void)setFrame:(MediaFrame*)frame
{
	_frame = frame;
}

- (MediaFrame*)frame
{
	return (MediaFrame*)_frame;
}

- (void)setScreen:(DKScreen*)screen
{
	_screen = screen;
}

- (DKScreen*)screen
{
	return (DKScreen*)_screen;
}

- (void)setWindow:(DKWindow*)window
{
	_window = window;
}

- (DKWindow*)window
{
	return (DKWindow*)_window;
}

- (id)initWithView:(UIView*)view
{
	self = [super init];
	if (self)
	{
		_app = DKObject<DKApplication>::New();
		
		CGRect bounds = view.bounds;
		DKRect rcWindow(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
		DKObject<DKWindow> tmpWindow = DKWindow::Create("MediaPlayer", rcWindow, true, true);
		
		if (tmpWindow)
		{
			DKLog("Proxy Window created.\n");
			
			DKObject<MediaFrame> tmpFrame = DKObject<MediaFrame>::New();
			// 렌더러 생성 및 연계
			DKObject<DKScreen> tmpScreen = DKScreen::Create(tmpWindow, tmpFrame);
			if (tmpScreen)
			{
				DKOpenGLContext::SharedInstance()->Bind();
				
				tmpWindow->Activate();
				UIView* playerView = (UIView*)tmpWindow->PlatformHandle();
				if ([playerView isKindOfClass:[UIView class]] == NO)
					[NSException raise:@"DKWindow Failed" format:@"DKWindow:%p is not UIView", playerView];
				
				[view addSubview:playerView];
				self.window = (DKWindow*)tmpWindow;
				self.screen = (DKScreen*)tmpScreen;
				self.frame = (MediaFrame*)tmpFrame;
				return self;
			}
			else
			{
				[NSException raise:@"DKScreen Failed" format:@"DKScreen failed to initialize."];
			}
		}
		else
		{
			[NSException raise:@"DKWindow Failed" format:@"DKWindow failed to initialize."];
		}
	}
	return self;
}

- (void)dealloc
{
	if (self.screen)
	{
		DKOpenGLContext::SharedInstance()->Unbind();
		self.screen->Terminate(true);
	}
	self.frame = NULL;
	self.screen = NULL;
	self.window = NULL;	
	
    [super dealloc];
}
@end

#pragma mark -
@interface MediaView ()
{
	MediaViewContext* context;
}
@property (nonatomic, readwrite, retain) MediaViewContext* context;
@end

@implementation MediaView
@synthesize context;

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
		self.context = [[MediaViewContext alloc] initWithView:self];
	}
    return self;
}

- (id)initWithCoder:(NSCoder *)decoder
{
	self = [super initWithCoder:decoder];
    if (self) {
        // Initialization code
		self.context = [[MediaViewContext alloc] initWithView:self];
	}
    return self;	
}

- (void)dealloc
{
	self.context = nil;	
    [super dealloc];
}

- (void)openMediaURL:(NSURL*)url
{
	if (self.context == nil)
	{
		NSLog(@"DK not initialized.\n");
		return;
	}
	if (url == nil)
	{
		NSLog(@"URL is empty!\n");
		return;
	}
	// url 열기
	NSString* urlString = nil;
	if ([url isFileURL])
		urlString = [url path];
	else
		urlString = [url absoluteString];

	if (urlString == nil)
	{
		NSLog(@"URL is empty!\n");
		return;
	}
	
	NSLog(@"Openning URL:%@\n", urlString);	
	DKString filepath = DKString((const DKUniChar8*)[urlString UTF8String]);		
	if (self.context.frame->OpenMedia(filepath))
	{
		NSLog(@"URL(%@) open succeed.\n", urlString);
	}
	else
	{
		NSLog(@"URL(%@) open failed.\n", urlString);
	}
}

@end
