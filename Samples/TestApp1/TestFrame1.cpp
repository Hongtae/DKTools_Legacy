#include "StdAfx.h"
#include "TestFrame1.h"
#include <math.h>

extern DKString g_resDir;

TestFrame1::TestFrame1(void)
	: timeTotal(0)
	, outlineBitmap(false)
{
}

TestFrame1::~TestFrame1(void)
{
}

void TestFrame1::OnLoaded(void)
{
	CaptureKeyboard(0);
	Screen()->Window()->SetTextInputEnabled(0, true);

	DKResourcePool	pool;
	pool.AddSearchPath(g_resDir);
	pool.AddSearchPath(g_resDir + "/Fonts");
 
	texture1 = pool.LoadResource("Aurora.jpg").SafeCast<DKTexture2D>();
	texture2 = pool.LoadResource("koo.jpg").SafeCast<DKTexture2D>();

	DKString defaultFont = pool.ResourceFilePath("NanumGothic.ttf");
	DKString testFont = pool.ResourceFilePath("SeoulNamsanM.otf");

	fontDefault = DKFont::Create(defaultFont.FilePathString());
	fontDefault->SetStyle(18);

	fontRegular = DKFont::Create(testFont.FilePathString());
	fontRegular->SetStyle(64, 0, 0, DKPoint(72,72));
	fontOutline = DKFont::Create(testFont.FilePathString());
	fontOutline->SetStyle(64, 0, 2, DKPoint(72,72), outlineBitmap);
}

void TestFrame1::OnUnload(void)
{
	fontRegular = NULL;
	fontOutline = NULL;
	fontDefault = NULL;
	texture1 = NULL;
	texture2 = NULL;

	DKApplication::Instance()->Terminate(0);
}

void TestFrame1::OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent)
{
	this->timeDelta = timeDelta;
	this->timeCurrent = timeCurrent;
	this->timeTotal += timeDelta;
	//this->SetContentScale(this->ContentResolution());
	SetRedraw();
}

void TestFrame1::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(0,0,0));
	renderer.RenderTexturedRect(DKRect(0,0,1,1), DKMatrix3::identity, DKRect(0,0,1,1), DKMatrix3::identity, texture1, NULL, DKColor(1,1,1));

	DKString inputText = strTextInput + strTextInputCandidate;

	// 입력한 글자 찍기
	DKPoint outlineSize = DKPoint(fontOutline->LineWidth(inputText), fontOutline->LineHeight());
	DKPoint outlineBegin = mouseDownRight;
	DKPoint outlineEnd = outlineBegin + PixelToLocal(DKPoint(outlineSize.x, 0));
	renderer.RenderText(outlineBegin, outlineEnd, inputText, fontOutline, DKColor(1,1,1,1));
	renderer.RenderSolidLine(outlineBegin, outlineEnd, DKColor(1,0,1,1));

//	DKRect outlineBounds = fontOutline->Bounds(inputText);
//	DKRect rectOutline = PixelToLocal(outlineBounds);
//	rectOutline.origin += mouseDownLeft;
//	renderer.RenderSolidRect(rectOutline, DKMatrix3::identity, DKColor(1,0,1,1));
//	renderer.RenderText(rectOutline, DKMatrix3::identity, inputText, fontOutline, DKColor(1,1,1,1));

	DKPoint regularSize = DKPoint(fontRegular->LineWidth(inputText), fontRegular->LineHeight());
	DKPoint regularBegin = mouseDownLeft;
	DKPoint regularEnd = regularBegin + PixelToLocal(DKPoint(regularSize.x, 0));
	renderer.RenderText(regularBegin, regularEnd, inputText, fontRegular, DKColor(1,0,0,1));
	renderer.RenderSolidLine(regularBegin, regularEnd, DKColor(0,0,1,1));

	//DKRect rectOutline = DKRect(mouseDownRight, PixelToLocal(DKSize(fontOutline->LineWidth(inputText), fontOutline->LineHeight())));
	//DKRect rectRegular = DKRect(mouseDownLeft, PixelToLocal(DKSize(fontRegular->LineWidth(inputText), fontRegular->LineHeight())));
	//renderer.RenderText(rectOutline, DKMatrix3::identity, inputText, fontOutline, DKColor(1,1,1,1));
	//renderer.RenderText(rectRegular, DKMatrix3::identity, inputText, fontRegular, DKColor(1,0,0,1));

	// 기타 글자 찍기
	DKString str1 = DKString::Format("Delta: %.6f (%.2f fps), Current:%ls",
		timeDelta, 1 / timeDelta,
		(const wchar_t*)timeCurrent.Format(DKDateTime::DateFormatWithWeekday, DKDateTime::TimeFormat24HourWithMicrosecondTimezone));
	DKString str2 = DKString::Format("ISO 8601 TZ:%ls", (const wchar_t*)timeCurrent.FormatISO8601(false));
	DKString str3 = DKString::Format("ISO 8601 UTC:%ls", (const wchar_t*)timeCurrent.FormatISO8601(true));

	DKString str4 = DKString::Format("Mouse: (%.3f, %.3f) %ls", mousePos.x, mousePos.y, IsMouseHover(0) ? L"Hover" : L"Leave");
	DKString str5 = DKString::Format("Baseline:(%.3f x %.3f%s) Outline:%.1f (%s)",
		regularSize.x, regularSize.y,
		(regularSize == outlineSize) ? "" : "- ERROR?",
		fontOutline->Outline(), this->outlineBitmap ? "Bitmap" : "Vector");

	float lineHeight = fontDefault->LineHeight();
	float str1Length = fontDefault->LineWidth(str1);
	float str2Length = fontDefault->LineWidth(str2);
	float str3Length = fontDefault->LineWidth(str3);
	float str4Length = fontDefault->LineWidth(str4);
	float str5Length = fontDefault->LineWidth(str5);
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 0)), PixelToLocal(DKPoint(str1Length, lineHeight * 0)), str1, fontDefault, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 1)), PixelToLocal(DKPoint(str2Length, lineHeight * 1)), str2, fontDefault, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 2)), PixelToLocal(DKPoint(str3Length, lineHeight * 2)), str3, fontDefault, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 3)), PixelToLocal(DKPoint(str4Length, lineHeight * 3)), str4, fontDefault, DKColor(1,1,1,1));
	renderer.RenderText(PixelToLocal(DKPoint(0, lineHeight * 4)), PixelToLocal(DKPoint(str5Length, lineHeight * 4)), str5, fontDefault, DKColor(1,1,1,1));
}

void TestFrame1::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)	// 마우스 버튼이 눌려짐
{
	CaptureKeyboard(0);
	Screen()->Window()->SetTextInputEnabled(0, true);

	if (buttonId == 0)
	{
		Screen()->Window()->ShowMouse(0, false);
		Screen()->Window()->HoldMouse(0, true);

		mouseDownLeft = pos;
	}
	else if (buttonId == 1)
	{
		mouseDownRight = pos;
	}
	else if (buttonId == 2)
	{
		Screen()->Window()->SetMousePosition(0, pos);
	}
	DKLog("%s(%d, %.2f, %.2f)\n", DKLIB_FUNCTION_NAME, buttonId, pos.x, pos.y);
}

void TestFrame1::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)		// 마우스 버튼이 올라감
{
	if (buttonId == 0)
	{
		Screen()->Window()->ShowMouse(0, true);
		Screen()->Window()->HoldMouse(0, false);
	}
	DKLog("%s(%d, %.2f, %.2f)\n", DKLIB_FUNCTION_NAME, buttonId, pos.x, pos.y);
}

void TestFrame1::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)					// 마우스가 이동할때 호출됨.
{
	mousePos = pos;

	//DKLog("%s pos:(%.1f, %.1f) delta:(%.2f, %.2f)\n", DKLIB_FUNCTION_NAME, pos.x, pos.y, delta.x, delta.y);
}

void TestFrame1::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)		// 마우스 휠이 움직임
{
	//float fLen = m_camera.Distance();
	//fLen -= (float)nDelta * 10;
	//m_camera.MovePosition(fLen);
	
	DKLog("%s pos(%.1f, %.1f) delta(%.1f, %.1f)\n", DKLIB_FUNCTION_NAME, pos.x, pos.y, delta.x, delta.y);

	if (Screen()->Window()->KeyState(0, DKVK_LEFT_CONTROL))
	{
		outlineBitmap = !outlineBitmap;
		fontOutline->SetStyle(fontOutline->PointSize(), fontOutline->Embolden(), fontOutline->Outline(), fontOutline->Resolution(), outlineBitmap);
		DKLog("Force-bitmap: %d\n", outlineBitmap);
	}
	else
	{
		const float minOutline = 0.0;
		const float maxOutline = 10.0;
		if (delta.y > 0)
		{
			float ps = fontOutline->Outline();
			ps = Clamp<float>(ps + 0.1, minOutline, maxOutline);
			fontOutline->SetStyle(fontOutline->PointSize(), fontOutline->Embolden(), ps, fontOutline->Resolution(), outlineBitmap);
			DKLog("font outline = %f \n", ps);
		}
		else if (delta.y < 0)
		{
			float ps = fontOutline->Outline();
			ps = Clamp<float>(ps - 0.1, minOutline, maxOutline);
			fontOutline->SetStyle(fontOutline->PointSize(), fontOutline->Embolden(), ps, fontOutline->Resolution(), outlineBitmap);
			DKLog("font outline = %f \n", ps);
		}
	}
}

void TestFrame1::OnMouseHover(int deviceId)												// 마우스가 프레임 영역에 들어옴
{
	DKLog("%s()\n", DKLIB_FUNCTION_NAME);
}

void TestFrame1::OnMouseLeave(int deviceId)												// 마우스가 프레임 영역에서 벗어남
{
	DKLog("%s()\n", DKLIB_FUNCTION_NAME);
}

void TestFrame1::OnKeyDown(int deviceId, DKVirtualKey key)					// 키가 눌려짐
{

	DKLog("%s(%x) = '%ls'\n", DKLIB_FUNCTION_NAME, key, (const wchar_t*)DKWindow::GetVKName((DKVirtualKey)key));
}

void TestFrame1::OnKeyUp(int deviceId, DKVirtualKey key)					// 키가 올라감
{
	DKLog("%s(%x) = '%ls'\n", DKLIB_FUNCTION_NAME, key, (const wchar_t*)DKWindow::GetVKName((DKVirtualKey)key));
}

void TestFrame1::OnTextInput(int deviceId, const DKString& str)			// 글자가 입력됨.
{
//	DKLog("%s('%ls')\n", DKLIB_FUNCTION_NAME, (const wchar_t*)str);
	if (str[0] == L'\n' || str[0] == L'\r')
		strTextInput = L"";
	else
		strTextInput += str;
}

void TestFrame1::OnTextInputCandidate(int deviceId, const DKString& str)	// 글자 입력중
{
//	DKLog("%s('%ls')\n", DKLIB_FUNCTION_NAME, (const wchar_t*)str);

	strTextInputCandidate = str;
}
