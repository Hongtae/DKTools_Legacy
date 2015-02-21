#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

class FontTest : public DKFrame
{
public:
	FontTest(void);
	~FontTest(void);

	void OnRender(DKRenderer& renderer) const;											// 프레임이 그려질때 호출됨
	void OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate);	// 프레임이 갱신될때 호출됨
	void OnLoaded(void);																		// 프레임이 로딩되면 호출됨
	void OnUnload(void);																		// 프레임이 언로드 되어야할때 호출됨
	void OnContentResized(void);

	void OnMouseDown(int deviceId, int buttonId, const DKPoint& pos);
	DKSize QueryContentResolution(void);
	bool renderTextBaseline;
private:
	DKObject<DKFont> font;
	DKPoint mouseLDown;
	DKPoint mouseRDown;

	double rotate;
};

