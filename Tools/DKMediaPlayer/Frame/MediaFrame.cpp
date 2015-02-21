#include "MediaFrame.h"

static const double menuInactivateDelay = 2.5;			// 메뉴가 사라지기전 입력 대기 시간
static const double menuTranslucentDuration = 0.25;		// 메뉴가 사라지는 시간

static const float menuBarOpacity = 0.5;				// 바 투명도
static const float menuButtonOpacity = 0.8;				// 버튼 투명도

MediaFrame::MediaFrame(void)
	: menuOpacity(1.0f)
	, menuActivated(0)
	, displayInfo(false)
	, keepRatio(false)
{
}

MediaFrame::~MediaFrame(void)
{
}

void MediaFrame::OnLoaded(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	fontRegular = DKFont::Create(DKApplication::Instance()->LoadResource("default_font.ttf"));
	fontRegular->SetStyle(18, 0, 0, DKPoint(72,72));
	fontOutline = DKFont::Create(DKApplication::Instance()->LoadResource("default_font.ttf"));
	fontOutline->SetStyle(18, 0, 2, DKPoint(72,72));

	DKObject<FrameButton> playButton = DKObject<FrameButton>::New();
	playButton->SetText(L">");
	playButton->SetCallback(DKFunction(this, &MediaFrame::OnButtonPlay), DKRunLoop::CurrentRunLoop(), this);

	DKObject<FrameButton> fasterButton = DKObject<FrameButton>::New();
	fasterButton->SetText(L"+");
	fasterButton->SetCallback(DKFunction(this, &MediaFrame::OnButtonFaster), DKRunLoop::CurrentRunLoop(), this);

	DKObject<FrameButton> slowerButton = DKObject<FrameButton>::New();
	slowerButton->SetText(L"-");
	slowerButton->SetCallback(DKFunction(this, &MediaFrame::OnButtonSlower), DKRunLoop::CurrentRunLoop(), this);

	DKObject<FrameButton> ratioButton = DKObject<FrameButton>::New();
	ratioButton->SetText(L"R");
	ratioButton->SetCallback(DKFunction(this, &MediaFrame::OnButtonRatio), DKRunLoop::CurrentRunLoop(), this);

	DKObject<FrameButton> infoButton = DKObject<FrameButton>::New();
	infoButton->SetText(L"i");
	infoButton->SetCallback(DKFunction(this, &MediaFrame::OnButtonInfo), DKRunLoop::CurrentRunLoop(), this);

	uiButtons.Clear();
	uiButtons.Add(playButton);
	uiButtons.Add(fasterButton);
	uiButtons.Add(slowerButton);
	uiButtons.Add(ratioButton);
	uiButtons.Add(infoButton);

	for (size_t i = 0; i < uiButtons.Count(); ++i)
	{
		FrameButton* btn = uiButtons.Value(i);
		btn->SetTextFont(fontRegular);
		btn->SetOutlineFont(fontOutline);
		btn->SetTextColor(DKColor(1,1,1,1));
		btn->SetOutlineColor(DKColor(0,0,0,1));
		btn->SetBlendState(DKBlendState::defaultAlpha);
		btn->SetBackgroundColor(DKColor(1.0,1.0,1.0, menuButtonOpacity), FrameButton::ButtonStateNormal);
		btn->SetBackgroundColor(DKColor(0, 0, 0.5), FrameButton::ButtonStateHighlighted);
		this->AddSubframe(btn);
	}

	if (DKPropertySet::DefaultSet().Value(L"MediaFrame.DisplayInfo").ValueType() == DKVariant::TypeInteger)
		this->displayInfo = DKPropertySet::DefaultSet().Value(L"MediaFrame.DisplayInfo").Integer() != 0;
	else
		this->displayInfo = false;

	if (DKPropertySet::DefaultSet().Value(L"MediaFrame.KeepRatio").ValueType() == DKVariant::TypeInteger)
		this->keepRatio = DKPropertySet::DefaultSet().Value(L"MediaFrame.KeepRatio").Integer() != 0;
	else
		this->keepRatio = false;

	// 수직동기화 활성화.
	DKObject<DKOpenGLContext> glContext = DKOpenGLContext::SharedInstance();
	glContext->SetSwapInterval(true);
	DKLog("OpenGL swapInterval:%d\n", (int)glContext->GetSwapInterval());
}

void MediaFrame::OnUnload(void)
{
	DKCriticalSection<DKSpinLock> guard(mediaLock);
	uiButtons.Clear();
	fontRegular = NULL;
	fontOutline = NULL;
	mediaSource = NULL;
}

void MediaFrame::OnContentResized(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	const DKSize& contentSize = this->ContentResolution();

	if (uiButtons.Count() > 0)
	{
		int numButtons = uiButtons.Count();
		const float buttonWidth = 55;
		const float buttonHeight = 40;
		const float buttonMargin = 1;

		DKSize allButtonsSize = DKSize(buttonWidth * numButtons + buttonMargin * (numButtons-1), buttonHeight);

		float buttonOffsetX = (contentSize.width - allButtonsSize.width) * 0.5;
		float buttonOffsetY = 50;

		for (int i = 0; i < numButtons; ++i)
		{
			DKRect btnRect = this->PixelToLocal(DKRect(buttonOffsetX + (buttonWidth + buttonMargin)* i, buttonOffsetY, buttonWidth, buttonHeight));
			uiButtons.Value(i)->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(btnRect.size.Vector()), btnRect.origin.Vector()).Matrix3());
		}
	}
	const float progressBarHeight = 30;
	progressRect = this->PixelToLocal(DKRect(0, 0, contentSize.width, progressBarHeight));
}

void MediaFrame::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	if (this->menuOpacity > 0 && this->progressRect.IsPointInside(pos))
	{
		double p = (pos.x - this->progressRect.origin.x) / this->progressRect.size.width;

		DKCriticalSection<DKSpinLock> guard(mediaLock);
		if (mediaSource)
		{
			DKAVMediaPlayer::MediaState st = mediaSource->State();
			if (st.mediaDuration > 0)
			{
				mediaSource->SetPosition(p * st.mediaDuration);
			}			
		}
	}
	this->ActivateMenu();
}

void MediaFrame::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
}

void MediaFrame::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	mousePos = pos;

	this->ActivateMenu();
}	

void MediaFrame::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
}	

void MediaFrame::OnMouseHover(int deviceId)
{
}

void MediaFrame::OnMouseLeave(int deviceId)
{
}											

void MediaFrame::OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent)
{ 
	this->timeElapsed = timeDelta;
	this->timeCurrent = timeCurrent;

	mediaLock.Lock();
	if (this->mediaSource)
		this->menuActivated += timeDelta;
	else
		this->menuActivated = 0;
	mediaLock.Unlock();

	if (menuActivated > menuInactivateDelay)
	{
		double trans = (menuActivated - menuInactivateDelay) / menuTranslucentDuration;
		if (trans > 1.0)
		{
			this->menuOpacity = 0.0;
		}
		else
		{
			this->menuOpacity = 1.0 - trans;
		}
	}
	else
		this->menuOpacity = 1.0;


	for (size_t i = 0; i < uiButtons.Count(); ++i)
	{
		FrameButton* btn = uiButtons.Value(i);
		btn->SetBackgroundColor(DKColor(1.0,1.0,1.0, menuButtonOpacity * this->menuOpacity), FrameButton::ButtonStateNormal);
		btn->SetHidden(this->menuOpacity <= 0.0);
	}

	SetRedraw();
}

void MediaFrame::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(0,0,0,1));

	DKSize contentSize = ContentResolution();

	DKArray<DrawText> drawText;
	drawText.Reserve(20);

	// 시간, 화면 크기 출력
	drawText.Add(DrawText(DKString::Format("FPS:%.2f (%d x %d)", 1 / timeElapsed, (int)contentSize.width, (int)contentSize.height), fontRegular, fontOutline, DKColor(1,1,1,1), DKColor(0,0,0,1)));

	double progress = 0;
	double buffered = 0;
	double duration = 0;

	mediaLock.Lock();
	if (mediaSource)
	{
		const DKTexture2D* tex = mediaSource->VideoTexture();
		if (tex)
		{
			int texWidth = tex->Width();
			int texHeight = tex->Height();
			DKSize frameSize = renderer.Viewport().size;

			if (texWidth > 0 && texHeight > 0 && frameSize.width > 0 && frameSize.height > 0)
			{
				DKRect mediaRect(0,0,1,1);
				if (this->keepRatio)
				{
					float r1 = frameSize.width / frameSize.height;
					float r2 = static_cast<float>(texWidth) / static_cast<float>(texHeight);

					if (r1 > r2)
					{
						mediaRect.size.width = r2 / r1;
						mediaRect.origin.x = (1.0 - mediaRect.size.width) * 0.5;
					}
					else
					{
						mediaRect.size.height = r1 / r2;
						mediaRect.origin.y = (1.0 - mediaRect.size.height) * 0.5;
					}
				}

				renderer.RenderTexturedRect(mediaRect, DKMatrix3::identity, DKRect(0,0,1,1), DKMatrix3::identity, tex, NULL, DKColor(1,1,1,1));
			}
			drawText.Add(DrawText(DKString::Format("Media %d x %d (Keep-Ratio:%d)", (int)tex->Width(), (int)tex->Height(), (int)this->keepRatio),
				fontRegular, fontOutline, DKColor(1,1,1,1), DKColor(0,0,0,1)));
		}

		DKAVMediaPlayer::MediaState st = mediaSource->State();

		duration = st.mediaDuration;
		progress = st.progress;
		buffered = Max(st.bufferedAudio, st.bufferedVideo);


		DKString controlState = L"N/A";
		switch (st.controlState)
		{
		case DKAVMediaPlayer::MediaState::ControlStateError:	controlState = L"Error";break;
		case DKAVMediaPlayer::MediaState::ControlStatePlaying:	controlState = L"Playing";break;
		case DKAVMediaPlayer::MediaState::ControlStateStopped:	controlState = L"Stopped";break;
		}

		DKString renderState = L"N/A";
		switch (st.renderState)
		{
		case DKAVMediaPlayer::MediaState::RenderStateStopped: renderState = L"Stopped";break;
		case DKAVMediaPlayer::MediaState::RenderStatePlaying: renderState = L"Playing";break;
		case DKAVMediaPlayer::MediaState::RenderStateWaiting: renderState = L"Buffering";break;
		}

		DKString bufferState = L"N/A";
		switch (st.bufferState)
		{
		case DKAVMediaPlayer::MediaState::BufferStateStopped:	bufferState = L"Stopped";break;
		case DKAVMediaPlayer::MediaState::BufferStateFeeding:	bufferState = L"Feeding";break;
		}

		double audioDiff = st.audio - st.progress;
		double videoDiff = st.video - st.progress;

		drawText.Add(DrawText(DKString::Format("Control:%ls %.1f/%.1f Speed:%.2f Repeat:%d",
			(const wchar_t*)controlState,
			st.progress, st.mediaDuration, st.speed, st.playCount - 1),
			fontRegular, fontOutline, DKColor(1,1,1,1), DKColor(0,0,1,1)));

		drawText.Add(DrawText(DKString::Format("Render:%ls A:%.1f, V:%.1f A-Sync:%+.1f, V-Sync:%+.1f",
			(const wchar_t*)renderState,
			st.audio, st.video, audioDiff, videoDiff),
			fontRegular, fontOutline, DKColor(1,1,1,1), DKColor(0,0,1,1)));

		drawText.Add(DrawText(DKString::Format("Buffer:%ls(%.1f) A:%.1f V:%.1f Range:%.1f-%.1f",
			(const wchar_t*)bufferState, Max(st.bufferedAudio, st.bufferedVideo),
			st.bufferedAudio, st.bufferedVideo, st.minBufferLength, st.maxBufferLength),
			fontRegular, fontOutline, DKColor(1,1,1,1), DKColor(0,0,1,1)));

	}
	mediaLock.Unlock();

	if (menuOpacity > 0)
	{
		// progress bar
		renderer.RenderSolidRect(this->progressRect, DKMatrix3::identity, DKColor(1,1,1, menuBarOpacity * menuOpacity));
	}

	if (duration > 0)
	{
		if (buffered > 0 && menuOpacity > 0)
		{
			DKRect rc = this->progressRect;
			rc.size.width *= (buffered / duration);
			renderer.RenderSolidRect(rc, DKMatrix3::identity, DKColor(0,1,0, menuBarOpacity * menuOpacity));
		}
		if (progress > 0 && menuOpacity > 0)
		{
			DKRect rc = this->progressRect;
			rc.size.width *= (progress / duration);
			renderer.RenderSolidRect(rc, DKMatrix3::identity, DKColor(0,0,1, menuBarOpacity * menuOpacity));
		}
		if (displayInfo)
		{
			DKString str = DKString::Format("%.1f / %.1f", progress, duration);

			DKSize textSize = PixelToLocal(DKSize(fontRegular->LineWidth(str), fontRegular->Height() - fontRegular->Baseline()));
			DKPoint lineBegin = progressRect.origin + DKPoint((progressRect.size.width - textSize.width) * 0.5, (progressRect.size.height - textSize.height) * 0.5);
			DKPoint lineEnd = DKPoint(lineBegin.x + textSize.width, lineBegin.y);
			renderer.RenderText(lineBegin, lineEnd, str, fontOutline, DKColor(0,0,0,1));
			renderer.RenderText(lineBegin, lineEnd, str, fontRegular, DKColor(1,1,1,1));
		}
	}
	else
	{
		if (displayInfo)
		{
			DKString str = L"Unknown";

			DKSize textSize = PixelToLocal(DKSize(fontRegular->LineWidth(str), fontRegular->Height() - fontRegular->Baseline()));
			DKPoint lineBegin = progressRect.origin + DKPoint((progressRect.size.width - textSize.width) * 0.5, (progressRect.size.height - textSize.height) * 0.5);
			DKPoint lineEnd = DKPoint(lineBegin.x + textSize.width, lineBegin.y);
			renderer.RenderText(lineBegin, lineEnd, str, fontOutline, DKColor(0,0,0,1));
			renderer.RenderText(lineBegin, lineEnd, str, fontRegular, DKColor(1,1,1,1));
		}		
	}

	if (displayInfo)
	{
		// 텍스트 출력.
		int heightOffset = 0;
		for (int i = 0; i < drawText.Count(); i++)
		{
			DrawText& dt = drawText.Value(i);
			if (dt.fontRegular == NULL)
				continue;

			int lineHeight = dt.fontRegular->LineHeight();
			heightOffset += lineHeight;

			DKPoint baseline(5, contentSize.height - heightOffset);

			if (dt.fontOutline)
				renderer.RenderText(PixelToLocal(baseline), PixelToLocal(DKPoint(fontOutline->LineWidth(dt.text) + baseline.x, baseline.y)), dt.text, dt.fontOutline, dt.colorOutline);
			renderer.RenderText(PixelToLocal(baseline), PixelToLocal(DKPoint(fontRegular->LineWidth(dt.text) + baseline.x, baseline.y)), dt.text, dt.fontRegular, dt.colorText);

			//	renderer.RenderSolidLine(PixelToLocal(baseline), PixelToLocal(DKPoint(fontRegular->LineWidth(dt.text) + baseline.x, baseline.y)), DKColor(1,0,0,1));
		}
	}
}

void MediaFrame::OnKeyDown(int deviceId, DKVirtualKey key)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void MediaFrame::OnKeyUp(int deviceId, DKVirtualKey key)
{  
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void MediaFrame::OnTextInput(int deviceId, const DKString& str)
{   
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void MediaFrame::OnTextInputCandidate(int deviceId, const DKString& str)
{   
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

bool MediaFrame::OpenMedia(const DKString& url)
{
	// 삭제후 재생성을 막기 위해 임시 저장.
	DKObject<DKAVMediaRenderer> renderer = DKAVMediaRenderer::SharedInstance();

	SetMedia(NULL);

	DKObject<DKAVMediaPlayer> media = DKAVMediaPlayer::Create(url, true, true);
	if (media)
	{
		return SetMedia(media);
	}
	return false;
}

bool MediaFrame::OpenMedia(DKStream* stream)
{
	// 삭제후 재생성을 막기 위해 임시 저장.
	DKObject<DKAVMediaRenderer> renderer = DKAVMediaRenderer::SharedInstance();

	SetMedia(NULL);

	DKObject<DKAVMediaPlayer> media = DKAVMediaPlayer::Create(stream, true, true);
	if (media)
	{
		return SetMedia(media);
	}
	return false;
}

bool MediaFrame::SetMedia(DKAVMediaPlayer* media)
{
	DKCriticalSection<DKSpinLock> guard(mediaLock);

	if (mediaSource == media)
		return true;

	// 삭제후 재생성을 막기 위해 임시 저장.
	DKObject<DKAVMediaRenderer> renderer = DKAVMediaRenderer::SharedInstance();

	if (mediaSource)
	{
		mediaSource->Stop();
		mediaSource = NULL;
	}

	mediaSource = media;

	if (mediaSource)
	{
		mediaSource->ResizeBuffer(1, 60);
		mediaSource->Play(1);

		this->ActivateMenu();
		return true;
	}
	return false;
}

bool MediaFrame::GetMediaSize(int& width, int& height) const
{
	DKCriticalSection<DKSpinLock> guard(mediaLock);
	if (mediaSource)
	{
		const DKTexture2D* tex = mediaSource->VideoTexture();
		if (tex)
		{
			width = tex->Width();
			height = tex->Height();
			return true;
		}
	}
	return false;
}

void MediaFrame::OnButtonPlay(FrameButton::ButtonEvent e, DKObject<FrameButton>)
{
	if (e == FrameButton::ButtonEventActivated)
	{
		DKCriticalSection<DKSpinLock> guard(mediaLock);
		if (mediaSource)
		{
			DKAVMediaPlayer::MediaState st = mediaSource->State();
			if (st.controlState == DKAVMediaPlayer::MediaState::ControlStateStopped)
			{
				mediaSource->Play(1);
			}
			else
			{
				mediaSource->Stop();
				mediaSource->SetPosition(0);
			}
		}
	}
}

void MediaFrame::OnButtonFaster(FrameButton::ButtonEvent e, DKObject<FrameButton>)
{
	if (e == FrameButton::ButtonEventActivated)
	{
		DKCriticalSection<DKSpinLock> guard(mediaLock);
		if (mediaSource)
		{
			DKAVMediaPlayer::MediaState st = mediaSource->State();
			if (st.speed < 4.0)
			{
				mediaSource->SetSpeed(Clamp<double>(st.speed + 0.5, 0.0, 4.0));
			}
		}
	}
}

void MediaFrame::OnButtonSlower(FrameButton::ButtonEvent e, DKObject<FrameButton>)
{
	if (e == FrameButton::ButtonEventActivated)
	{
		DKCriticalSection<DKSpinLock> guard(mediaLock);
		if (mediaSource)
		{
			DKAVMediaPlayer::MediaState st = mediaSource->State();
			if (st.speed > 0.0)
			{
				mediaSource->SetSpeed(Clamp<double>(st.speed - 0.5, 0.0, 4.0));
			}
		}
	}
}

void MediaFrame::OnButtonRatio(FrameButton::ButtonEvent e, DKObject<FrameButton>)
{
	if (e == FrameButton::ButtonEventActivated)
	{
		this->keepRatio = !this->keepRatio;

		DKPropertySet::DefaultSet().SetValue(L"MediaFrame.KeepRatio", (DKVariant::VInteger)this->keepRatio);
	}
}

void MediaFrame::OnButtonInfo(FrameButton::ButtonEvent e, DKObject<FrameButton>)
{
	if (e != FrameButton::ButtonEventActivated)
		return;

	displayInfo = !displayInfo;

	DKPropertySet::DefaultSet().SetValue(L"MediaFrame.DisplayInfo", (DKVariant::VInteger)this->displayInfo);
}

void MediaFrame::ActivateMenu(void)
{
	this->menuActivated = 0;
	this->menuOpacity = 1.0;

	for (size_t i = 0; i < uiButtons.Count(); ++i)
	{
		FrameButton* btn = uiButtons.Value(i);
		btn->SetBackgroundColor(DKColor(1.0,1.0,1.0, menuButtonOpacity), FrameButton::ButtonStateNormal);
		btn->SetHidden(false);
	}

	SetRedraw();
}
