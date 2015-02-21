#include "StdAfx.h"
#include "TestFrame1.h"
#include <math.h>

extern DKString g_resDir;

TestFrame1::TestFrame1(void)
{
}

TestFrame1::~TestFrame1(void)
{
	Unload();
}

void TestFrame1::OnLoaded(void)
{
	CaptureKeyboard(0);
	Screen()->Window()->SetTextInputEnabled(0, false);

	equalizer.Load(Screen(), DKSize(600,200));
	equalizer.SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(0.8, 0.5), DKVector2(0.1,0.2)).Matrix3());

	AddSubframe(&equalizer);

	DKRect eqRect;
	eqRect.origin = DKVector2(0,0).Transform(equalizer.Transform());
	eqRect.size = DKVector2(1,1).Transform(equalizer.Transform());

	DKResourcePool	pool;
	pool.AddSearchPath(g_resDir);
	pool.AddSearchPath(g_resDir + "/Fonts");
	pool.AddSearchPath(g_resDir + "/Ogg");

	equalizer.Open(pool.ResourceFilePath("S8_faded.ogg"));

	textureBG = pool.LoadResource("Aurora.jpg").SafeCast<DKTexture2D>();

	font = DKFont::Create(pool.ResourceFilePath("NanumGothic.ttf"));
	font->SetStyle(18);
}

void TestFrame1::OnUnload(void)
{
	textureBG = NULL;
	font = NULL;
	texture = NULL;

	DKLog("[TestFrame1::OnDestroy] end.\n");

	DKApplication::Instance()->Terminate(0);
}

void TestFrame1::OnUpdate(double delta, DKTimeTick tick, const DKDateTime& current)
{
	tickDelta = delta;
	timeCurrent = current;

//	equalizer.SetPosition(vec.x, vec.y, vec.z);
//	DKLog("Audio pos: %f, %f, %f\n", vec.x, vec.y, vec.z);
}

void TestFrame1::OnRender(DKRenderer& renderer) const
{
	// 바탕 이미지 찍기
	renderer.RenderTexturedRect(DKRect(0,0,1,1),DKMatrix3::identity, DKRect(0,0,1,1), DKMatrix3::identity, textureBG, NULL, DKColor(1,1,1,1));

	if (equalizer.audioSource)
	{
		double duration = equalizer.audioSource->Duration();
		double position = equalizer.audioSource->TimePosition();
		DKRect progressRect(0.2, 0.8, 0.6, 0.1);
		renderer.RenderSolidRect(progressRect, DKMatrix3::identity, DKColor(0,0,0,0.5));
		progressRect.size.width *= position / duration;
		renderer.RenderSolidRect(progressRect, DKMatrix3::identity, DKColor(1,0,0,0.5));
	}

	// 글자 찍기
	DKString str1 = DKString::Format("Elapsed: %.6f (%.2f fps), Current:%ls",
		tickDelta, 1 / tickDelta, (const wchar_t*)timeCurrent.Format(DKDateTime::DateFormatWithWeekday, DKDateTime::TimeFormat24HourWithMicrosecondTimezone));
	DKString str2 = (equalizer.audioSource != NULL) ? DKString::Format("Audio: %.2f / %.2f", equalizer.audioSource->TimePosition(), equalizer.audioSource->Duration()) :	L"Audio Not Loaded";
	DKString str3 = DKString::Format("Mouse: (%.3f, %.3f) %s", mousePos.x, mousePos.y, IsMouseHover(0) ? "Hover" : "Leave");
	DKString str4 = DKString("Input: ") + inputTextString + inputCandidateString;

	float lineHeight = font->LineHeight();
	float str1Length = font->LineWidth(str1);
	float str2Length = font->LineWidth(str2);
	float str3Length = font->LineWidth(str3);
	float str4Length = font->LineWidth(str4);
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 0)), PixelToLocal(DKPoint(str1Length, lineHeight * 0)), str1, font, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 1)), PixelToLocal(DKPoint(str2Length, lineHeight * 1)), str2, font, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 2)), PixelToLocal(DKPoint(str3Length, lineHeight * 2)), str3, font, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 3)), PixelToLocal(DKPoint(str4Length, lineHeight * 3)), str4, font, DKColor(1,1,1,1));
}

void TestFrame1::OnMouseDown(int deviceId, int nButtonId, const DKPoint& pos)	// 마우스 버튼이 눌려짐
{
	DKLog("TestFrame1::OnMouseDown(%d, %.2f, %.2f)\n", nButtonId, pos.x, pos.y);
	switch (nButtonId)
	{
	case 0:
		DKLog("Hide mouse\n");
		Screen()->Window()->ShowMouse(0, false);
		break;
	case 1:
		DKLog("Hold Mouse\n");
		Screen()->Window()->HoldMouse(0, true);
		Screen()->SetFocusFrame(0, this);		// 트래킹 메시지를 받을 프레임 설정.
		break;
	case 2:
		break;
	}
}

void TestFrame1::OnMouseUp(int deviceId, int nButtonId, const DKPoint& pos)		// 마우스 버튼이 올라감
{
	DKLog("TestFrame1::OnMouseUp(%d, %.2f, %.2f)\n", nButtonId, pos.x, pos.y);
	switch (nButtonId)
	{
		case 0:
			DKLog("Show Mouse\n");
			Screen()->Window()->ShowMouse(0, true);
			break;
		case 1:
			DKLog("Free Mouse\n");
			Screen()->Window()->HoldMouse(0, false);
			Screen()->SetFocusFrame(0, NULL);
			break;
		case 2:
			if (equalizer.State() == DKAudioPlayer::AudioState::StatePlaying)
				equalizer.Pause();
			else
				equalizer.Play();
			break;
	}

}

void TestFrame1::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)					// 마우스가 이동할때 호출됨.
{
	if (deviceId == 0)
		mousePos = pos;

//	DKLog("%s pos(%.3f, %.3f) delta(%.3f, %.3f)\n", DKLIB_FUNCTION_NAME, pos.x, pos.y, delta.x, delta.y);
}

void TestFrame1::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)		// 마우스 휠이 움직임
{
//	DKLog("TestFrame1::OnMouseWheel(%d, %f, %f)\n", nDelta, pos.x, pos.y);
}

void TestFrame1::OnMouseHover(int deviceId)												// 마우스가 프레임 영역에 들어옴
{
//	DKLog("TestFrame1::OnMouseHover()\n");
}

void TestFrame1::OnMouseLeave(int deviceId)												// 마우스가 프레임 영역에서 벗어남
{
//	DKLog("TestFrame1::OnMouseLeave()\n");
}

void TestFrame1::OnKeyDown(int deviceId, DKVirtualKey lKey)					// 키가 눌려짐
{
	DKLog("TestFrame1::OnKeyDown(%x) = '%ls'\n", lKey, (const wchar_t*)DKWindow::GetVKName(lKey));
	if (lKey == DKVK_ESCAPE)
	{
	}
	if (lKey == DKVK_ENTER || lKey == DKVK_RETURN)
	{
		CaptureKeyboard(0);
		Screen()->Window()->SetTextInputEnabled(0, true);
		DKLog("Command input mode.\n");
		inputTextString = L"";
		inputCandidateString = L"";
	}
}

void TestFrame1::OnKeyUp(int deviceId, DKVirtualKey lKey)					// 키가 올라감
{
	DKLog("TestFrame1::OnKeyUp(%x) = '%ls'\n", lKey, (const wchar_t*)DKWindow::GetVKName(lKey));
}

void TestFrame1::OnTextInput(int deviceId, const DKString& str)			// 글자가 입력됨.
{
	if (str[0] == 0x1b)		// ESC
	{
		DKLog("Command cancelled.\n");
		CaptureKeyboard(0);
		Screen()->Window()->SetTextInputEnabled(0, false);
	}
	else if (str[0] == L'\n' || str[0] == L'\r')
	{
		DKLog(DKString(L"Command: \"") + inputTextString + L"\"\n");
		inputTextString = L"";
		inputCandidateString = L"";
	}
	else
	{
		DKLog("TestFrame1::OnTextInput('%ls')\n", (const wchar_t*)str);
		inputTextString += str;
	}
}

void TestFrame1::OnTextInputCandidate(int deviceId, const DKString& str)	// 글자 입력중
{
	DKLog("TestFrame1::OnTextInputCandidate('%ls')\n", (const wchar_t*)str);
	inputCandidateString = str;
}

