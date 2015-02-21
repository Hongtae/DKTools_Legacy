#include "FrameButton.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;

FrameButton::FrameButton(void)
	: state(ButtonStateNormal)
	, capturedMouseId(-1)
{
	buttonStyleNormal.textColor = DKColor(0.1, 0.1, 0.1, 1.0);
	buttonStyleHighlighted.textColor = DKColor(0.1, 0.1, 0.1, 1.0);
	buttonStyleDisabled.textColor = DKColor(0.6, 0.6, 0.6, 1.0);

	buttonStyleNormal.outlineColor = DKColor(0.0, 0.0, 0.0, 0.3);
	buttonStyleHighlighted.outlineColor = DKColor(0.0, 0.0, 0.0, 0.3);
	buttonStyleDisabled.outlineColor = DKColor(0.0, 0.0, 0.0, 0.3);

	buttonStyleNormal.backgroundColor = DKColor(1.0, 1.0, 1.0, 1.0);
	buttonStyleHighlighted.backgroundColor = DKColor(0.8, 0.8, 1.0, 1.0);
	buttonStyleDisabled.backgroundColor = DKColor(1.0, 1.0, 1.0, 1.0);
	
	SetDepthFormat(DKRenderTarget::DepthFormatNone);
}

FrameButton::~FrameButton(void)
{
	Unload();
}

void FrameButton::OnRender(DKRenderer& renderer) const
{
	const ButtonStyle* style = NULL;

	switch (this->state)
	{
	case ButtonStateNormal:
		style = &buttonStyleNormal;
		break;
	case ButtonStateHighlighted:
		style = &buttonStyleHighlighted;
		break;
	case ButtonStateDisabled:
		style = &buttonStyleDisabled;
		break;
	}

	if (style)
	{
		if (style->texture)
		{
			renderer.RenderTexturedRect(DKRect(0,0,1,1), DKMatrix3::identity, DKRect(0,0,1,1), DKMatrix3::identity, style->texture, NULL, style->backgroundColor, DKBlendState::defaultOpaque);
		}
		else
		{
			renderer.Clear(style->backgroundColor);
		}

		if (style->text.Length() > 0)
		{
			if (style->textFont)
			{
				const DKSize scale = ContentScale();
				DKSize textSize = PixelToLocal(DKSize(style->textFont->LineWidth(style->text), style->textFont->Height() - style->textFont->Baseline()));
				DKPoint lineBegin = DKPoint((scale.width  - textSize.width) * 0.5, (scale.height - textSize.height) * 0.5);
				DKPoint lineEnd = DKPoint(lineBegin.x + textSize.width, lineBegin.y);

				if (style->outlineFont)
					renderer.RenderText(lineBegin, lineEnd, style->text, style->outlineFont, style->outlineColor);

				renderer.RenderText(lineBegin, lineEnd, style->text, style->textFont, style->textColor);
			}
		}
	}
}

void FrameButton::OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
	if (this->IsEnabled())
	{
		if (this->state == ButtonStateDisabled)
		{
			this->state = ButtonStateNormal;
			SetRedraw();
		}
		if (this->IsHidden())
		{
			if (this->capturedMouseId != -1)		// 마우스 잡고 있는거 해제
			{
				this->ReleaseMouse(this->capturedMouseId);
				this->capturedMouseId = -1;
			}
			if (this->state == ButtonStateHighlighted) // highlighted -> cancelled
			{
				this->state = ButtonStateNormal;
				PostEvent(ButtonEventCancelled);
				SetRedraw();
			}
		}
	}
	else		// 프레임이 비활성화 상태임 (ButtonStateDisabled 여야함)
	{
		if (this->capturedMouseId != -1)		// 마우스 잡고 있는거 해제
		{
			this->ReleaseMouse(this->capturedMouseId);
			this->capturedMouseId = -1;
		}

		if (this->state == ButtonStateHighlighted)	// highlighted -> cancelled
		{
			this->state = ButtonStateDisabled;
			PostEvent(ButtonEventCancelled);
			SetRedraw();
		}
		else if (this->state != ButtonStateDisabled)
		{
			this->state = ButtonStateDisabled;
			SetRedraw();
		}
	}
}

void FrameButton::OnLoaded(void)
{
	this->state = ButtonStateNormal;
}

void FrameButton::OnUnload(void)
{
	if (this->capturedMouseId != -1)
	{
		this->ReleaseMouse(this->capturedMouseId);
		this->capturedMouseId = -1;
	}
	this->state = ButtonStateNormal;

	buttonStyleNormal.textFont = NULL;
	buttonStyleDisabled.textFont = NULL;
	buttonStyleHighlighted.textFont = NULL;

	buttonStyleNormal.outlineFont = NULL;
	buttonStyleDisabled.outlineFont = NULL;
	buttonStyleHighlighted.outlineFont = NULL;

	buttonStyleNormal.texture = NULL;
	buttonStyleDisabled.texture = NULL;
	buttonStyleHighlighted.texture = NULL;
}

void FrameButton::OnContentResized(void)
{
}

void FrameButton::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	if (this->state == ButtonStateNormal)		// 다른 마우스가 눌려져 있으며, 버튼 위에 있을 경우엔 무시함.
	{
		if (this->capturedMouseId != -1)			// 현재 영역을 벗어난, 기존에 먼저 누르고 있던 마우스가 있음
		{
			this->ReleaseMouse(this->capturedMouseId);	// 먼저 마우스 해제
			this->capturedMouseId = -1;
		}

		if (CaptureMouse(deviceId))
		{
			this->capturedMouseId = deviceId;
			this->state = ButtonStateHighlighted;
			PostEvent(ButtonEventHighlighted);
			SetRedraw();
		}
	}
}

void FrameButton::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	if (IsMouseCapturedBySelf(deviceId))
	{
		this->ReleaseMouse(deviceId);
		this->capturedMouseId = -1;

		if (this->state == ButtonStateHighlighted)	// 마우스가 버튼 위에 있었음. (up-inside)
		{
			PostEvent(ButtonEventActivated);
		}

		this->state = ButtonStateNormal;
		SetRedraw();
	}
}

void FrameButton::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	if (this->capturedMouseId == deviceId)
	{
		ButtonState newState = this->IsMouseHover(deviceId) ? ButtonStateHighlighted : ButtonStateNormal;

		if (newState != this->state)
		{
			if (this->state == ButtonStateHighlighted)		// 마우스가 영역을 벗어났음, highlighted->cancelled
			{
				this->state = newState;
				PostEvent(ButtonEventCancelled);
			}
			else if (newState == ButtonStateHighlighted)	// 마우스가 영역으로 되돌아왔음. normal->hightlighted
			{
				this->state = newState;
				PostEvent(ButtonEventHighlighted);
			}
			else
			{
				this->state = newState;
			}
			SetRedraw();
		}
	}
}

void FrameButton::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	//SetRedraw();
}

void FrameButton::OnMouseHover(int deviceId)
{
	//SetRedraw();
}

void FrameButton::OnMouseLeave(int deviceId)
{
	//SetRedraw();
}

FrameButton::ButtonStyle& FrameButton::Style(ButtonState s)
{
	SetRedraw();

	if (s == ButtonStateHighlighted)
		return buttonStyleHighlighted;
	else if (s == ButtonStateDisabled)
		return buttonStyleDisabled;
	return buttonStyleNormal;
}

const FrameButton::ButtonStyle& FrameButton::Style(ButtonState s) const
{
	if (s == ButtonStateHighlighted)
		return buttonStyleHighlighted;
	else if (s == ButtonStateDisabled)
		return buttonStyleDisabled;
	return buttonStyleNormal;
}

void FrameButton::SetStyle(const ButtonStyle& style, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted = style;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled = style;
		break;
	case ButtonStateNormal:
		buttonStyleNormal = style;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted = style;
		buttonStyleDisabled = style;
		buttonStyleNormal = style;
		break;
	}
	SetRedraw();
}

void FrameButton::SetText(const DKString& text, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.text = text;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.text = text;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.text = text;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.text = text;
		buttonStyleDisabled.text = text;
		buttonStyleNormal.text = text;
		break;
	}
	SetRedraw();
}

void FrameButton::SetTextColor(const DKColor& c, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.textColor = c;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.textColor = c;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.textColor = c;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.textColor = c;
		buttonStyleDisabled.textColor = c;
		buttonStyleNormal.textColor = c;
		break;
	}
	SetRedraw();
}

void FrameButton::SetTextFont(DKFont* font, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.textFont = font;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.textFont = font;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.textFont = font;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.textFont = font;
		buttonStyleDisabled.textFont = font;
		buttonStyleNormal.textFont = font;
		break;
	}
	SetRedraw();
}

void FrameButton::SetOutlineColor(const DKColor& c, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.outlineColor = c;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.outlineColor = c;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.outlineColor = c;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.outlineColor = c;
		buttonStyleDisabled.outlineColor = c;
		buttonStyleNormal.outlineColor = c;
		break;
	}
	SetRedraw();
}

void FrameButton::SetOutlineFont(DKFont* font, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.outlineFont = font;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.outlineFont = font;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.outlineFont = font;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.outlineFont = font;
		buttonStyleDisabled.outlineFont = font;
		buttonStyleNormal.outlineFont = font;
		break;
	}
	SetRedraw();
}

void FrameButton::SetBackgroundColor(const DKColor& c, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.backgroundColor = c;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.backgroundColor = c;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.backgroundColor = c;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.backgroundColor = c;
		buttonStyleDisabled.backgroundColor = c;
		buttonStyleNormal.backgroundColor = c;
		break;
	}
	SetRedraw();
}

void FrameButton::SetTexture(DKTexture2D* texture, ButtonState s)
{
	switch (s)
	{
	case ButtonStateHighlighted:
		buttonStyleHighlighted.texture = texture;
		break;
	case ButtonStateDisabled:
		buttonStyleDisabled.texture = texture;
		break;
	case ButtonStateNormal:
		buttonStyleNormal.texture = texture;
		break;
	case ButtonStateAll:
		buttonStyleHighlighted.texture = texture;
		buttonStyleDisabled.texture = texture;
		buttonStyleNormal.texture = texture;
		break;
	}
	SetRedraw();
}

void FrameButton::SetCallback(Callback* func, DKRunLoop* runLoop, void* context)
{
	callback.SetCallback(func, runLoop, context);
}

void FrameButton::RemoveCallback(void* context)
{
	callback.Remove(context);
}

void FrameButton::PostEvent(ButtonEvent e)
{
	callback.PostInvocation(e, this);
}
