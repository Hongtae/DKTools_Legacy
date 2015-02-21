#include "StdAfx.h" 
#include <math.h>
#include "TestFrame2.h"

extern DKObject<DKResourcePool>	resourcePool;

TestFrame2::TestFrame2(void)
{
}

TestFrame2::~TestFrame2(void)
{
}

void TestFrame2::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(0,0,1));
	renderer.RenderTexturedEllipse(DKRect(0.1,0.5,0.6,0.4), DKRect(0,0,1,1), tex, NULL, DKColor(1,1,1));
	renderer.RenderSolidLine(DKPoint(0.1,0.1), DKPoint(0.9,0.9), DKColor(1,1,1));
	renderer.RenderTexturedRect(DKRect(0.5, 0.05, 0.2, 0.2), DKAffineTransform2(DKLinearTransform2().Rotate(DKL_PI/10)).Matrix3(), DKRect(0,0,1,1), DKMatrix3::identity, tex, NULL, DKColor(1,1,0));

	DKRect textRect(0,0,0.5,0.5);
	DKMatrix3 textTransform = DKAffineTransform2(DKLinearTransform2().Rotate(DKL_PI/10), DKVector2(0.2,0.1)).Matrix3();
	renderer.RenderSolidRect(textRect, textTransform, DKColor(0,0,0,0.5));
	renderer.RenderText(textRect, textTransform, L"ABC", font, DKColor(1,1,1));

	DKString text = DKString::Format("Content: %dx%d", (int)ContentResolution().width, (int)ContentResolution().height);
	DKRect rect = DKRect(DKPoint::zero, PixelToLocal(DKSize(font->LineWidth(text), font->LineHeight())));
	renderer.RenderSolidRect(rect, DKMatrix3::identity, DKColor(0,0,0,0.5));
	renderer.RenderText(rect, DKMatrix3::identity, text, font, DKColor(1,1,1,1));
}

void TestFrame2::OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
}

void TestFrame2::OnLoaded(void)
{
	font = DKFont::Create(resourcePool->LoadResourceData(L"NanumGothic.ttf"));
	font->SetStyle(30);
	tex = resourcePool->LoadResource(L"koo.jpg").SafeCast<DKTexture>();
	
	SetTransform(DKAffineTransform2(DKLinearTransform2().Rotate(DKL_PI/15).Scale(0.25), DKVector2(0.1,0.7)).Matrix3());
}

void TestFrame2::OnUnload(void)
{
	resourcePool->RemoveAllResources();
	resourcePool->RemoveAllResourceData();
	font = NULL;
	tex = NULL;
}
