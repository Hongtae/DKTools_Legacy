//
//  FileViewController.h
//  DKMediaPlayer
//
//  Created by tiff on 11. 6. 28..
//  Copyright 2011 ICONDB.COM. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface FileViewController : UITableViewController
@property (nonatomic, readwrite, assign) id delegate;
- (id)initWithURL:(NSURL*)baseURL;
@end


@protocol FileViewControllerDelegate <NSObject>
- (void)fileView:(FileViewController*)fileView didSelectURL:(NSURL*)url;
@end
