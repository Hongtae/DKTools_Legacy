#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// FrameButton
//
// DKFrame 형태로 사용할 수 있는 버튼
//
////////////////////////////////////////////////////////////////////////////////

namespace DKUtils
{
	class FrameButton : public DKFramework::DKFrame
	{
	public:
		enum ButtonState
		{
			ButtonStateNormal = 0,
			ButtonStateHighlighted,
			ButtonStateDisabled,
			ButtonStateAll,				// 값을 세팅할 때만 사용된다. (존재하지 않는 state 임)
		};
		enum ButtonEvent
		{
			ButtonEventHighlighted,		// 버튼이 눌려짐
			ButtonEventActivated,		// 버튼을 누른후 뗐음
			ButtonEventCancelled,		// 버튼을 누른후 취소되었음
		};
		struct ButtonStyle
		{
			DKFoundation::DKString								text;
			DKFoundation::DKObject<DKFramework::DKFont>			textFont;
			DKFramework::DKColor								textColor;
			DKFoundation::DKObject<DKFramework::DKFont>			outlineFont;
			DKFramework::DKColor								outlineColor;
			DKFoundation::DKObject<DKFramework::DKTexture2D>	texture;
			DKFramework::DKColor								backgroundColor;
		};

		typedef void CallbackProc(ButtonEvent, DKFoundation::DKObject<FrameButton>);
		typedef DKFoundation::DKFunctionSignature<CallbackProc> Callback;

		FrameButton(void);
		~FrameButton(void);

		ButtonState State(void) const	{return state;}

		ButtonStyle& Style(ButtonState s);
		const ButtonStyle& Style(ButtonState s) const;
		void SetStyle(const ButtonStyle& style, ButtonState s = ButtonStateAll);

		void SetText(const DKFoundation::DKString& text, ButtonState s = ButtonStateAll);
		void SetTextColor(const DKFramework::DKColor& c, ButtonState s = ButtonStateAll);
		void SetTextFont(DKFramework::DKFont* font, ButtonState s = ButtonStateAll);
		void SetOutlineColor(const DKFramework::DKColor& c, ButtonState s = ButtonStateAll);
		void SetOutlineFont(DKFramework::DKFont* font, ButtonState s = ButtonStateAll);
		void SetBackgroundColor(const DKFramework::DKColor& c, ButtonState s = ButtonStateAll);
		void SetTexture(DKFramework::DKTexture2D* texture, ButtonState s = ButtonStateAll);

		void SetCallback(Callback* func, DKFoundation::DKRunLoop* runLoop, void* context);
		void RemoveCallback(void* context);

	protected:
		void OnRender(DKFramework::DKRenderer& renderer) const;
		void OnUpdate(double tickDelta, DKFoundation::DKTimeTick tick, const DKFoundation::DKDateTime& tickDate);
		void OnLoaded(void);
		void OnUnload(void);
		void OnContentResized(void);

		void OnMouseDown(int deviceId, int buttonId, const DKFramework::DKPoint& pos);
		void OnMouseUp(int deviceId, int buttonId, const DKFramework::DKPoint& pos);
		void OnMouseMove(int deviceId, const DKFramework::DKPoint& pos, const DKFramework::DKVector2& delta);
		void OnMouseWheel(int deviceId, const DKFramework::DKPoint& pos, const DKFramework::DKVector2& delta);
		void OnMouseHover(int deviceId);
		void OnMouseLeave(int deviceId);

	private:
		ButtonStyle buttonStyleNormal;
		ButtonStyle buttonStyleDisabled;
		ButtonStyle buttonStyleHighlighted;

		DKFoundation::DKCallback<CallbackProc, void*> callback;
		int capturedMouseId;

		ButtonState state;
		void PostEvent(ButtonEvent e);
	};
}
