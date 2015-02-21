//
//  DKMediaPlayerAppDelegate.h
//  DKMediaPlayer
//
//  Created by tiff on 11. 6. 25..
//  Copyright 2011 ICONDB.COM. All rights reserved.
//

#import <UIKit/UIKit.h>

@class MediaPlayerViewController;

@interface MediaPlayerAppDelegate : NSObject <UIApplicationDelegate>
{
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet MediaPlayerViewController *viewController;

@end
