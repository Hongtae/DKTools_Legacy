#include "StdAfx.h" 
#include "SmallFrame.h"

extern DKObject<DKResourcePool>	resourcePool;

SmallFrame::SmallFrame(void)
{
}

SmallFrame::~SmallFrame(void)
{
}

void SmallFrame::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(1,1,1,1));

	DKPoint pt = MousePosition(0);
	DKString text = DKString::Format("Content: %dx%d, Scale(%.2f,%.2f), Mouse:(%.2f,%.2f),(%.2f,%.2f), Hover:%d",
		(int)ContentResolution().width, (int)ContentResolution().height,
		ContentScale().width, ContentScale().height,
		pt.x, pt.y,
		mousePt.x, mousePt.y,
		IsMouseHover(0));

//	DKRect rect = DKRect(DKPoint::zero, PixelToLocal(DKSize(font->LineWidth(text), font->LineHeight())));
	DKRect rect = PixelToLocal(font->Bounds(text));
	renderer.RenderSolidRect(rect, DKMatrix3::identity, DKColor(0,0,0,0.5));
	renderer.RenderText(rect, DKMatrix3::identity, text, font, DKColor(1,1,1,1));
}

void SmallFrame::OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
	SetRedraw();
}

void SmallFrame::OnLoaded(void)
{
	font = DKFont::Create(resourcePool->LoadResourceData(L"NanumGothic.ttf"));
	font->SetStyle(15);
	SetContentScale(DKSize(1.2,1.2));
}

void SmallFrame::OnUnload(void)
{
	font = NULL;
}

void SmallFrame::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	CaptureMouse(deviceId);
	DKLog("Mouse(%d) captured by %x\n", deviceId, this);
}

void SmallFrame::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	ReleaseMouse(deviceId);
	DKLog("Mouse(%d) released by %x\n", deviceId, this);
}

void SmallFrame::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	mousePt = pos;
}

void SmallFrame::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	DKSize scale = ContentScale();

	scale.width = Max<float>(scale.width + delta.x, 0.1);
	scale.height = Max<float>(scale.height + delta.y, 0.1);

	SetContentScale(scale);
}
