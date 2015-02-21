#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class SplineTest : public DKFrame
{
public:
	SplineTest(void);
	~SplineTest(void);

protected:
	void OnRender(DKRenderer& renderer) const;											// 프레임이 그려질때 호출됨
	void OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate);	// 프레임이 갱신될때 호출됨
	void OnLoaded(void);																		// 프레임이 로딩되면 호출됨
	void OnUnload(void);																		// 프레임이 언로드 되어야할때 호출됨
	void OnMouseUp(int deviceId, int buttonId, const DKPoint& pos);					// 마우스 버튼이 올라감

private:
	void ResetPoints(void);
	void NextSpline(void);
	DKSpline::Type	splineType;
	DKObject<DKFont> font;
	DKOrderedArray<DKVector3> splinePoints;
};
