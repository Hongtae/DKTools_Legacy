#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#import <CoreLocation/CoreLocation.h>

@interface GyroScopeDelegate : NSObject <CLLocationManagerDelegate, UIAccelerometerDelegate>
{
	CLLocationManager*			locationManager;
	UIAccelerationValue			accelerationX;
	UIAccelerationValue			accelerationY;
	UIAccelerationValue			accelerationZ;
	CLHeadingComponentValue		headingX;
	CLHeadingComponentValue		headingY;
	CLHeadingComponentValue		headingZ;
	double						lowpassFilter;	
}
@property (retain, nonatomic)		CLLocationManager*			locationManager;
@property (readonly)				CLHeadingComponentValue		headingX, headingY, headingZ;
@property (readonly)				UIAccelerationValue			accelerationX, accelerationY, accelerationZ;
@end
@implementation GyroScopeDelegate
@synthesize locationManager;
@synthesize headingX, headingY, headingZ;
@synthesize accelerationX, accelerationY, accelerationZ;

- (void)initialize
{	
	if ([CLLocationManager headingAvailable])
	{
		self.locationManager = [[[CLLocationManager alloc] init] autorelease];
		[self.locationManager setDelegate:self];
	
		[self.locationManager setHeadingFilter:kCLHeadingFilterNone];
		[self.locationManager startUpdatingHeading];
		
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval: 1.0/50.0];	// 50 Hz		
	}
}

+ (void)deinitializeLocationManager:(CLLocationManager*)manager
{
	[manager stopUpdatingHeading];
	[manager stopUpdatingLocation];
}

- (id)init
{
	self = [super init];
	if (self)
	{
		headingX = 0;
		headingY = 0;
		headingZ = -1;
		accelerationX = 0;
		accelerationY = -1;
		accelerationZ = 0;
		lowpassFilter = 0.1f;

		// locationManager 는 메인쓰레드에서 초기화 되어야 한다.
		[self performSelectorOnMainThread:@selector(initialize) withObject:nil waitUntilDone:NO];
	}
	return self;
}

- (void)dealloc
{
	// locationManager 는 메인 쓰레드에서 작업중지하고 제거되도록 한다.
	if (self.locationManager)
	{
		[GyroScopeDelegate performSelectorOnMainThread:@selector(deinitializeLocationManager:) withObject:self.locationManager waitUntilDone:NO];
		self.locationManager = nil;
	}
	[super dealloc];
}

#pragma mark - CLLocationManagerDelegate
- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
	NSLog(@"CLLocationManager failed:%@\n", [error localizedDescription]);
}

- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)newHeading
{
	if (newHeading.headingAccuracy > 0)
	{
		headingX = headingX * (1.0f - lowpassFilter) + newHeading.x * lowpassFilter;
		headingY = headingY * (1.0f - lowpassFilter) + newHeading.y * lowpassFilter;
		headingZ = headingZ * (1.0f - lowpassFilter) + newHeading.z * lowpassFilter;
	}
}

- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager *)manager
{
	return NO;
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	accelerationX = accelerationX * (1.0f - lowpassFilter) + acceleration.x * lowpassFilter;
	accelerationY = accelerationY * (1.0f - lowpassFilter) + acceleration.y * lowpassFilter;
	accelerationZ = accelerationZ * (1.0f - lowpassFilter) + acceleration.z * lowpassFilter;
}

@end

#pragma mark - GyroScope implementation
#include "GyroScope.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;

GyroScope::GyroScope(void)
: context(NULL)
{
	context = reinterpret_cast<void*>([[GyroScopeDelegate alloc] init]);
}

GyroScope::~GyroScope(void)
{
	[reinterpret_cast<GyroScopeDelegate*>(context) release];
}

DKMatrix3 GyroScope::Orientation(void) const
{
	GyroScopeDelegate* gyro = (GyroScopeDelegate*)context;
	if ([gyro isKindOfClass:[GyroScopeDelegate class]])
	{
		DKVector3 deviceHeading = DKVector3(gyro.headingX, gyro.headingY, gyro.headingZ).Normalize();
		DKVector3 deviceGravity = DKVector3(gyro.accelerationX, gyro.accelerationY, gyro.accelerationZ).Normalize();
		
		// 로컬 좌표를 gravity 로 변환하기 위한 타겟 (중력과 가까운 y 축 방향으로 설정한다)
		DKVector3 rotationTarget = DKVector3::Dot(DKVector3(0,1,0), deviceGravity) > 0.0 ? DKVector3(0,1,0) : DKVector3(0,-1,0);
		// 로컬좌표가 gravity 로 이동하는 회전을 구한다.
		DKQuaternion localToGravity(DKVector3::Cross(rotationTarget, deviceGravity), acos(DKVector3::Dot(rotationTarget, deviceGravity)));
		
		// rightAngledHeading: 중력방향과 직각을 이루는 북쪽(heading)
		DKVector3 rightAngledHeading = deviceHeading * DKQuaternion(localToGravity).Inverse();	// heading 을 로컬 좌표로 변환한다.
		rightAngledHeading.y = 0;								// 로컬 heading 을 x,z 평면으로 프로젝션
		rightAngledHeading.Rotate(localToGravity);				// 다시 현실 좌표로 원위치 (이제 중력과 직각이 된다)
		
		DKVector3 worldAxisY = DKVector3(-deviceGravity);
		DKVector3 worldAxisZ = DKVector3(-rightAngledHeading).Normalize();
		DKVector3 worldAxisX = DKVector3::Cross(worldAxisY, worldAxisZ).Normalize();	// X 축 좌표는 Y 축과 Z 축의 외적
		
		DKMatrix3 deviceRotate = DKMatrix3::identity;
		UIInterfaceOrientation interfaceOrientation = UIInterfaceOrientationPortraitUpsideDown;
		
		UIWindow* keyWindow = [[UIApplication sharedApplication] keyWindow];
		if ([keyWindow respondsToSelector:@selector(rootViewController)])
		{
			UIViewController* viewController = [keyWindow rootViewController];
			if (viewController)
				interfaceOrientation = viewController.interfaceOrientation;
		}
		
		switch (interfaceOrientation)
		{
			case UIInterfaceOrientationPortraitUpsideDown:
				deviceRotate *= DKLinearTransform3().Identity().Rotate(DKVector3(0,0,1), DKL_PI).Matrix3();
				break;
			case UIInterfaceOrientationLandscapeLeft:
				deviceRotate *= DKLinearTransform3().Identity().Rotate(DKVector3(0,0,-1), DKL_PI/2).Matrix3();
				break;
			case UIInterfaceOrientationLandscapeRight:
				deviceRotate *= DKLinearTransform3().Identity().Rotate(DKVector3(0,0,1), DKL_PI/2).Matrix3();
				break;
			default:
				break;
		}
		
		return DKMatrix3(worldAxisX, worldAxisY, worldAxisZ) * deviceRotate;		
	}
	return DKMatrix3::identity;
}

#endif // if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
