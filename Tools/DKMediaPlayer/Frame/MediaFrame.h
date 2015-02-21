#pragma once

#include <DK.h>
#include "../../../Extras/DKAVPlayer/DKAVPlayer.h"
#include "../../../Extras/DKUtils/DKUtils.h"
using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;

class MediaFrame :
	public DKFrame
{
public:
	MediaFrame(void);
	~MediaFrame(void);

	bool SetMedia(DKAVMediaPlayer* media);
	bool OpenMedia(const DKString& url);
	bool OpenMedia(DKStream* stream);
	bool GetMediaSize(int& width, int& height) const;

protected:
	void OnLoaded(void);
	void OnUnload(void);
	void OnContentResized(void);

	void OnMouseDown(int deviceId, int buttonId, const DKPoint& pos);				// 마우스 버튼이 눌려짐
	void OnMouseUp(int deviceId, int buttonId, const DKPoint& pos);					// 마우스 버튼이 올라감
	void OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta);		// 마우스가 이동할때 호출됨.
	void OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta);	// 마우스 휠이 움직임
	void OnMouseHover(int deviceId);												// 마우스가 프레임 영역에 들어옴
	void OnMouseLeave(int deviceId);												// 마우스가 프레임 영역에서 벗어남

	void OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent);
	void OnRender(DKRenderer& renderer) const;

	void OnKeyDown(int deviceId, DKVirtualKey key);									// 키가 눌려짐
	void OnKeyUp(int deviceId, DKVirtualKey key);									// 키가 올라감
	void OnTextInput(int deviceId, const DKFoundation::DKString& str);				// 글자가 입력됨.
	void OnTextInputCandidate(int deviceId, const DKFoundation::DKString& str);		// 글자 입력중
private:
	// 정보 찍기
	DKPoint						mousePos;
	double						timeElapsed;
	DKDateTime					timeCurrent;	
	DKObject<DKFont>			fontRegular;
	DKObject<DKFont>			fontOutline;
	DKObject<DKAVMediaPlayer>	mediaSource;

	struct DrawText
	{
		DrawText(const DKString& t, const DKFont* r, const DKFont* o, const DKColor& c1, const DKColor& c2)
			: text(t), fontRegular(r), fontOutline(o), colorText(c1), colorOutline(c2)
		{
		}
		DKString			text;
		const DKFont*		fontRegular;
		const DKFont*		fontOutline;
		DKColor				colorText;
		DKColor				colorOutline;
	};

	DKRect progressRect;

	DKSpinLock	mediaLock;
	bool		keepRatio;
	bool		displayInfo;
	float		menuOpacity;
	double		menuActivated;

	DKArray<DKObject<FrameButton>> uiButtons;

	void OnButtonPlay(FrameButton::ButtonEvent, DKObject<FrameButton>);
	void OnButtonFaster(FrameButton::ButtonEvent, DKObject<FrameButton>);
	void OnButtonSlower(FrameButton::ButtonEvent, DKObject<FrameButton>);
	void OnButtonRatio(FrameButton::ButtonEvent, DKObject<FrameButton>);
	void OnButtonInfo(FrameButton::ButtonEvent, DKObject<FrameButton>);

	void ActivateMenu(void);
};
