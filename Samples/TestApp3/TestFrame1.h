#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class TestFrame1 : public DKFrame
{
public:
	TestFrame1(void);
	~TestFrame1(void);

protected:
	void OnRender(DKRenderer& renderer) const;									// 프레임이 그려질때 호출됨
	void OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate);		// 프레임이 갱신될때 호출됨
	void OnLoaded(void);																// 프레임이 로딩되면 호출됨
	void OnUnload(void);																// 프레임이 언로드 되어야할때 호출됨
	
	void OnMouseDown(int deviceId, int buttonId, const DKPoint& pos);					// 마우스 버튼이 눌려짐
	void OnMouseUp(int deviceId, int buttonId, const DKPoint& pos);						// 마우스 버튼이 올라감
	void OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta);			// 마우스가 이동할때 호출됨.
	void OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta);		// 마우스 휠이 움직임

private:
	DKObject<DKFont> font;
	DKObject<DKFrame> subFrame1;

	DKPoint			mousePos;
	double			tickDelta;
	DKDateTime		tickDate;
};
