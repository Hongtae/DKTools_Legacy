#include "StdAfx.h" 
#include <math.h>
#include "TestFrame1.h"
#include "TestFrame2.h"
#include "SmallFrame.h"
#include "FontTest.h"
#include "SplineTest.h"


extern DKObject<DKResourcePool>	resourcePool;

TestFrame1::TestFrame1(void)
{
}

TestFrame1::~TestFrame1(void)
{
}

void TestFrame1::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(1,0,0));

	DKString text1 = DKString::Format("Delta: %.6f (%.2f fps), Current:%ls",
									 tickDelta, 1 / tickDelta,
									 (const wchar_t*)tickDate.Format(DKDateTime::DateFormatWithWeekday, DKDateTime::TimeFormat24HourWithMicrosecondTimezone));

	DKPoint pt = MousePosition(0);
	DKString text2 = DKString::Format("Content: %dx%d, Scale(%.2f,%.2f), Mouse:(%.3f,%.3f), Hover:%d",
		(int)ContentResolution().width, (int)ContentResolution().height,
		ContentScale().width, ContentScale().height,
		pt.x, pt.y, IsMouseHover(0));

	float lineHeight = font->LineHeight();
	float text1Length = font->LineWidth(text1);
	float text2Length = font->LineWidth(text2);
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 0)), PixelToLocal(DKPoint(text1Length, lineHeight * 0)), text1, font, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 1)), PixelToLocal(DKPoint(text2Length, lineHeight * 1)), text2, font, DKColor(1,1,1,1));
}

static bool rotate = false;

void TestFrame1::OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
	this->tickDelta = tickDelta;
	this->tickDate = tickDate;

	static float r = 0;
	if (rotate)
		r += tickDelta * 0.1;

	DKSize scale = ContentScale();

	SetContentTransform(
		DKAffineTransform2(scale.Vector() * -0.5)
		.Multiply(DKAffineTransform2(DKLinearTransform2().Rotate(r)))
		.Translate(scale.Vector() * 0.5).Matrix3());

	SetRedraw();
}

void TestFrame1::OnLoaded(void)
{
	font = DKFont::Create(resourcePool->LoadResourceData(L"NanumGothic.ttf"));
	font->SetStyle(18);

	subFrame1 = DKObject<TestFrame2>::New().SafeCast<DKFrame>();

	this->AddSubframe(subFrame1);

	DKObject<DKFrame> small1 = DKObject<SmallFrame>::New().SafeCast<DKFrame>();
	small1->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(0.8,0.4).Rotate(-DKL_PI/100), DKVector2(0.1,0.1)).Matrix3());

	DKObject<DKFrame> small1_1 = DKObject<SmallFrame>::New().SafeCast<DKFrame>();
	small1_1->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(0.85,0.8).Rotate(DKL_PI/10), DKVector2(0.2,0.1)).Matrix3());
	small1_1->SetColor(DKColor(0.5,0.5,0.5));
	small1->AddSubframe(small1_1);

	DKObject<DKFrame> small1_1_1 = DKObject<SmallFrame>::New().SafeCast<DKFrame>();
	small1_1_1->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(0.95,0.6).Rotate(-DKL_PI/5), DKVector2(0.01,0.8)).Matrix3());
	small1_1_1->SetColor(DKColor(0,1,1));
	small1_1->AddSubframe(small1_1_1);

	AddSubframe(small1);

	DKObject<DKFrame> fontTest = DKObject<FontTest>::New().SafeCast<DKFrame>();
	AddSubframe(fontTest);
	this->BringSubframeToFront(fontTest);
	
	DKObject<DKFrame> splineTest = DKObject<SplineTest>::New().SafeCast<DKFrame>();
	splineTest->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(0.55,0.25).Rotate(0), DKVector2(0.4,0.5)).Matrix3());
	AddSubframe(splineTest);
}

void TestFrame1::OnUnload(void)
{
	font = NULL;
	
	DKApplication::Instance()->Terminate(0);
}

void TestFrame1::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	CaptureMouse(deviceId);
	DKLog("Mouse(%d) captured by %x\n", deviceId, this);
}

void TestFrame1::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	ReleaseMouse(deviceId);
	DKLog("Mouse(%d) released by %x\n", deviceId, this);
	if (buttonId == 1)
		this->UpdateContentResolution();
	if (buttonId == 2)
		rotate = !rotate;
}

void TestFrame1::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
}

void TestFrame1::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	DKSize scale = ContentScale();

	scale.width = Max<float>(scale.width + (delta.x * 10), 0.1);
	scale.height = Max<float>(scale.height + (delta.y * 10), 0.1);

	SetContentScale(scale);

	DKLog("contentScale: (%f, %f)\n", scale.width, scale.height);
}
