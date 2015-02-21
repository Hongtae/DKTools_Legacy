#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// GyroScope
//
// iOS 용 나침반과 중력센서를 이용하여 구현된 자이로 스코프
//
////////////////////////////////////////////////////////////////////////////////

namespace DKUtils
{
	class GyroScope : public DKFoundation::DKSharedInstance<GyroScope>
	{
	public:
		~GyroScope(void);
		DKFramework::DKMatrix3 Orientation(void) const;

	private:
		GyroScope(void);
		GyroScope(GyroScope&);
		GyroScope& operator = (GyroScope&);

		void* context;
		friend class DKFoundation::DKObject<GyroScope>;
		friend class DKFoundation::DKSharedInstance<GyroScope>;
	};
}
