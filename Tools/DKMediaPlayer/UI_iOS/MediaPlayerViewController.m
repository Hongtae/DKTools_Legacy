//
//  DKMediaPlayerViewController.m
//  DKMediaPlayer
//
//  Created by tiff on 11. 6. 25..
//  Copyright 2011 ICONDB.COM. All rights reserved.
//

#import "MediaPlayerViewController.h"
#import "FileViewController.h"
#import "MediaView.h"

@interface MediaPlayerViewController ()
{    
	MediaView *mediaView;
}
@property (nonatomic, retain) IBOutlet MediaView *mediaView;
@end

@implementation MediaPlayerViewController
@synthesize mediaView;

- (void)dealloc
{
    [mediaView release];
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
	
	[self performSelector:@selector(showFileList:) withObject:nil afterDelay:0];
}

- (void)viewDidUnload
{
    [self setMediaView:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotate
{
	return YES;
}

- (NSUInteger)supportedInterfaceOrientations
{
	return UIInterfaceOrientationMaskAll;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    //return (interfaceOrientation == UIInterfaceOrientationPortrait);
	return YES;
}

#pragma mark - IB Actions
- (IBAction)showFileList:(id)sender
{	
	FileViewController* viewController = [[FileViewController alloc] init];
	viewController.delegate = self;
	UINavigationController* navController = [[UINavigationController alloc] initWithRootViewController:viewController];
	[self presentModalViewController:navController animated:YES];
	[viewController release];
	[navController release];
}

#pragma mark - FileViewControllerDelegate
- (void)fileView:(FileViewController*)fileView didSelectURL:(NSURL*)url
{
	[self dismissModalViewControllerAnimated:YES];
	NSLog(@"User Selected URL: %@\n", url);

	[self.mediaView performSelector:@selector(openMediaURL:) withObject:url afterDelay:0];
}

@end
