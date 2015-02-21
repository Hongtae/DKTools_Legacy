#include "StdAfx.h"
#include "FontTest.h"

extern DKObject<DKResourcePool>	resourcePool;

FontTest::FontTest(void)
	: rotate(0)
	, renderTextBaseline(false)
	, mouseLDown(0,0)
	, mouseRDown(1,1)
{
}

FontTest::~FontTest(void)
{
}

void FontTest::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(0,0,0,0.7));

	DKString text = L"ASDFqqggpprrjj";
	DKSize fontSize = PixelToLocal(DKSize(font->LineWidth(text), font->LineHeight()));
	DKVector2 frameCenter = this->ContentScale().Vector() * 0.5f;
	DKMatrix3 fontTM = DKAffineTransform2(DKLinearTransform2().Scale(fontSize.width, fontSize.height).Rotate(-rotate), frameCenter).Matrix3();

	if (renderTextBaseline)
	{
		DKVector2 lineStart = this->mouseLDown.Vector();
		DKVector2 lineEnd = this->mouseRDown.Vector();

		renderer.RenderText(lineStart, lineEnd, text, font, DKColor(1,1,1,1));
		renderer.RenderSolidLine(lineStart, lineEnd, DKColor(1,0,0,1));
	}
	else
	{
		renderer.RenderSolidRect(DKRect(-0.5,-0.5,1,1), fontTM, DKColor(0,0,1,1));
		renderer.RenderText(DKRect(-0.5,-0.5,1,1), fontTM, text, font, DKColor(1,1,1,1));
	}
}

void FontTest::OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
	rotate += tickDelta;

	DKPoint framePos = DKPoint(0.45, 0.8);
	DKSize frameSize = DKSize(0.5, 0.19);
	SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(frameSize.Vector()), DKVector2(framePos.Vector())).Matrix3());

	SetRedraw();
}

void FontTest::OnLoaded(void)
{
	font = DKFont::Create(resourcePool->LoadResourceData(L"NanumGothic.ttf"));
	font->SetStyle(60);

	SetBlendState(DKBlendState::defaultAlpha);

	DKPoint framePos = DKPoint(0.45, 0.8);
	DKSize frameSize = DKSize(0.5, 0.19);
	SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(frameSize.Vector()), DKVector2(framePos.Vector())).Matrix3());
	//SetTransform(DKAffineTransform2().Scale(frameSize.Vector()).Translate(-frameSize.Vector()/2).Rotate(rotate).Translate(0.5,0.5));
}

void FontTest::OnUnload(void)
{
	font = NULL;
}

void FontTest::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	if (buttonId == 0)
		this->mouseLDown = pos;
	else if (buttonId == 1)
		this->mouseRDown = pos;
	else
		this->renderTextBaseline = !renderTextBaseline;
}

void FontTest::OnContentResized(void)
{
	//this->SetContentScale(this->ContentResolution());
	DKLog("%s (%dx%d)\n", DKLIB_FUNCTION_NAME, static_cast<int>(ContentResolution().width), static_cast<int>(ContentResolution().height));
}

DKSize FontTest::QueryContentResolution(void)
{
	return DKFrame::QueryContentResolution();
}