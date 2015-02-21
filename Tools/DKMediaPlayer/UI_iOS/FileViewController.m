//
//  FileViewController.m
//  DKMediaPlayer
//
//  Created by tiff on 11. 6. 28..
//  Copyright 2011 ICONDB.COM. All rights reserved.
//

#import "FileViewController.h"

@interface FileViewController ()
{
    NSFileManager *fileManager;
	NSURL *baseURL;
	NSArray* files;
	UIBarButtonItem *cancelButton;
	id delegate;
}
@property (nonatomic, retain) IBOutlet UIBarButtonItem *cancelButton;
@property (nonatomic, readwrite, retain) NSFileManager* fileManager;
@property (nonatomic, readwrite, retain) NSURL* baseURL;
@property (nonatomic, readwrite, retain) NSArray* files;
@end

@implementation FileViewController
@synthesize cancelButton;
@synthesize fileManager;
@synthesize baseURL;
@synthesize files;
@synthesize delegate;

- (id)initWithURL:(NSURL*)url
{
	self = [super initWithNibName:@"FileViewController" bundle:nil];
	if (self)
	{
		self.fileManager = [[NSFileManager alloc] init];

		if (url)
		{
			self.baseURL = url;	
		}
		else
		{
			self.baseURL = [fileManager URLForDirectory:NSDocumentDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
		}
		
		self.title = [self.baseURL lastPathComponent];
	}
	return self;
}

- (id)init
{
	return [self initWithURL:nil];
}

- (void)dealloc
{
	self.baseURL = nil;
	self.fileManager = nil;
    [cancelButton release];
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
 
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
	self.navigationItem.rightBarButtonItem = self.cancelButton;
	self.files = [self.fileManager contentsOfDirectoryAtPath:[self.baseURL path] error:nil];
}

- (void)viewDidUnload
{
    [self setCancelButton:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    //return (interfaceOrientation == UIInterfaceOrientationPortrait);
	return YES;
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Return the number of sections.
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Return the number of rows in the section.
    return self.files.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier] autorelease];
    }
    
    // Configure the cell...
	NSString* file = [self.files objectAtIndex:indexPath.row];
    NSString* filePath = [[self.baseURL path] stringByAppendingPathComponent:file];

	cell.textLabel.text = file;
	
	NSDictionary* fileAttr = [self.fileManager attributesOfItemAtPath:filePath error:nil];
	if ([[fileAttr valueForKey:NSFileType] isEqualToString:NSFileTypeDirectory])
	{
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		cell.detailTextLabel.text = nil;
	}
	else 
	{
		cell.accessoryType = UITableViewCellAccessoryNone;
		cell.detailTextLabel.text = [NSNumberFormatter localizedStringFromNumber:[fileAttr valueForKey:NSFileSize] numberStyle:NSNumberFormatterDecimalStyle];
	}
    return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	NSString* file = [self.files objectAtIndex:indexPath.row];
    NSString* filePath = [[self.baseURL path] stringByAppendingPathComponent:file];

	NSURL* selectedURL = [NSURL fileURLWithPath:filePath];
	NSLog(@"User selected url: %@\n", selectedURL);
	
	NSDictionary* fileAttr = [self.fileManager attributesOfItemAtPath:filePath error:nil];
	if ([[fileAttr valueForKey:NSFileType] isEqualToString:NSFileTypeDirectory])
	{
		FileViewController* viewController = [[FileViewController alloc] initWithURL:selectedURL];
		viewController.delegate = self;
		[self.navigationController pushViewController:viewController animated:YES];
		[viewController release];
	}
	else 
	{
		[(id)self fileView:self didSelectURL:selectedURL];
	}
}

#pragma mark - IBActions
- (IBAction)cancelAction:(id)sender
{
	[self.navigationController dismissModalViewControllerAnimated:YES];
}

#pragma mark - FileViewControllerDelegate
- (void)fileView:(FileViewController*)fileView didSelectURL:(NSURL*)url
{
	if (self.delegate && [self.delegate respondsToSelector:@selector(fileView:didSelectURL:)])
	{
		[self.delegate fileView:self didSelectURL:url];
	}
}

@end
