#include "GyroScope.h"

#if defined(__APPLE__) && defined(__MACH__)
#import <TargetConditionals.h>
#endif

#if !TARGET_OS_IPHONE

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;

GyroScope::GyroScope(void)
	: context(NULL)
{
}

GyroScope::~GyroScope(void)
{
}

DKMatrix3 GyroScope::Orientation(void) const
{
	return DKMatrix3::identity;
}

#endif
