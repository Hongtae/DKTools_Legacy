#include "StdAfx.h" 
#include <math.h>
#include "SplineTest.h"

extern DKObject<DKResourcePool>	resourcePool;

static inline bool Vector3Compare(const DKVector3& lhs, const DKVector3& rhs)
{
	if (lhs.x == rhs.x)
	{
		if (lhs.y == rhs.y)
			return lhs.z < rhs.z;
		return lhs.y < rhs.y;
	}
	return lhs.x < rhs.x;
}

SplineTest::SplineTest(void)
	: splinePoints(Vector3Compare)
	, splineType(DKSpline::CatmullRom)
{
}

SplineTest::~SplineTest(void)
{
}

void SplineTest::OnRender(DKRenderer& renderer) const
{
	renderer.Clear(DKColor(0,0,1));

	if (splinePoints.Count() > 1)
	{
		DKArray<DKPoint> lines;
		float pixel = 1.0 / ContentResolution().width;

		int first = 0;
		int count = splinePoints.Count();
		int last = count - 1;

		if (splineType == DKSpline::CatmullRom)
		{
			for (int i = 0; i + 1 < count; i++)
			{
				DKVector3 p0 = i > 0 ? splinePoints.Value(i-1) : splinePoints.Value(0);
				DKVector3 p1 = splinePoints.Value(i);
				DKVector3 p2 = splinePoints.Value(i+1);
				DKVector3 p3 = i + 2 < count ? splinePoints.Value(i+2) : splinePoints.Value(last);

				DKSpline3 spline(p0, p1, p2, p3);

				float length = p2.x - p1.x;
				float step = pixel / length;

				for (float x = 0; x < length;)
				{
					DKVector3 p1 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p1.x, p1.y));
					x = Min<float>(x + step, length);
					DKVector3 p2 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p2.x, p2.y));			
				}
			}	
		}
		else if (splineType == DKSpline::UniformCubic)
		{
			for (int i = 0; i + 1 < count; i++)
			{
				DKVector3 p0 = i > 0 ? splinePoints.Value(i-1) : splinePoints.Value(0);
				DKVector3 p1 = splinePoints.Value(i);
				DKVector3 p2 = splinePoints.Value(i+1);
				DKVector3 p3 = i + 2 < count ? splinePoints.Value(i+2) : splinePoints.Value(last);

				DKSpline3 spline(p0, p1, p2, p3);

				float length = p2.x - p1.x;
				float step = pixel / length;

				for (float x = 0; x < length;)
				{
					DKVector3 p1 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p1.x, p1.y));
					x = Min<float>(x + step, length);
					DKVector3 p2 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p2.x, p2.y));			
				}
			}
		}
		else if (splineType == DKSpline::Hermite)
		{
			for (int i = 0; i + 1 < count; i ++)
			{
				DKVector3 p0 = i > 0 ? splinePoints.Value(i-1) : splinePoints.Value(0);
				DKVector3 p1 = splinePoints.Value(i);
				DKVector3 p2 = splinePoints.Value(i+1);
				DKVector3 p3 = i + 2 < count ? splinePoints.Value(i+2) : splinePoints.Value(last);

				DKVector3 t0 = DKVector3(p1 - p0);
				DKVector3 t1 = DKVector3(p2 - p1);
				DKVector3 t2 = DKVector3(p3 - p2);

				DKSpline3 spline(p1, p2, t0, t2);

				float length = p2.x - p1.x;
				float step = pixel / length;

				for (float x = 0; x < length;)
				{
					DKVector3 p1 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p1.x, p1.y));
					x = Min<float>(x + step, length);
					DKVector3 p2 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p2.x, p2.y));			
				}
			}			
		}
		else if (splineType == DKSpline::Bezier)
		{
			for (int i = 0; i + 3 < count; i+=4)
			{
				DKVector3 p0 = splinePoints.Value(i);
				DKVector3 p1 = splinePoints.Value(i+1);
				DKVector3 p2 = splinePoints.Value(i+2);
				DKVector3 p3 = splinePoints.Value(i+3);

				DKSpline3 spline(p0, p1, p2, p3);

				float length = p3.x - p0.x;
				float step = pixel / length;

				for (float x = 0; x < length;)
				{
					DKVector3 p1 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p1.x, p1.y));
					x = Min<float>(x + step, length);
					DKVector3 p2 = spline.Interpolate(x / length, splineType);
					lines.Add(DKPoint(p2.x, p2.y));			
				}
			}	
		}

		renderer.RenderSolidLines(lines, lines.Count(), DKColor(1,1,1,1));
	}
	DKSize pointSize = DKSize(1.0 / ContentResolution().width, 1.0 / ContentResolution().height);
	pointSize *= 20;

	for (int i = 0; i < splinePoints.Count(); i++)
	{
		const DKVector3& v = splinePoints.Value(i);
		DKRect rect = DKRect( v.x - pointSize.width * 0.5, v.y - pointSize.height * 0.5, pointSize.width, pointSize.height);
		renderer.RenderSolidRect(rect, DKMatrix3::identity, DKColor(0,0,0,1));
		renderer.RenderText(rect, DKMatrix3::identity, DKString::Format("%d", i), font, DKColor(1,1,1,1));
	}

	DKString splineText = L"ERROR";
	switch (splineType)
	{
	case DKSpline::CatmullRom:
		splineText = DKString::Format("CatmullRom %d points", splinePoints.Count());
		break;
	case DKSpline::UniformCubic:
		splineText = DKString::Format("UniformCubic %d points", splinePoints.Count());
		break;
	case DKSpline::Hermite:
		splineText = DKString::Format("Hermite %d points", splinePoints.Count());
		break;
	case DKSpline::Bezier:
		splineText = DKString::Format("Bezier %d points", splinePoints.Count());
		break;
	}


	DKString text = DKString::Format("Content: %dx%d (Spline %ls)", (int)ContentResolution().width, (int)ContentResolution().height, (const wchar_t*)splineText);
	DKRect rect = font->Bounds(text);
	rect.origin += DKPoint(2, font->Baseline() + 1);
	rect = PixelToLocal(rect);
	renderer.RenderSolidRect(rect, DKMatrix3::identity, DKColor(0,0,0,0.5));
	renderer.RenderText(rect, DKMatrix3::identity, text, font, DKColor(1,1,1,1));
}

void SplineTest::OnUpdate(double tickDelta, DKTimeTick tick, const DKDateTime& tickDate)
{
}

void SplineTest::OnLoaded(void)
{
	font = DKFont::Create(resourcePool->LoadResourceData(L"NanumGothic.ttf"));
	font->SetStyle(18);

	ResetPoints();
}

void SplineTest::OnUnload(void)
{
	resourcePool->RemoveAllResources();
	resourcePool->RemoveAllResourceData();
	font = NULL;
}

void SplineTest::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	int id = Max(deviceId, buttonId);

	if (id == 0)
		DKRunLoop::CurrentRunLoop()->PostOperation(DKFunction(this, &SplineTest::ResetPoints)->Invocation());
	else if (id == 1)
		DKRunLoop::CurrentRunLoop()->PostOperation(DKFunction(this, &SplineTest::NextSpline)->Invocation());		
}

void SplineTest::ResetPoints(void)
{
	splinePoints.Clear();

	int height = 10;
	int width = 9;

	for (int x = 1; x < width; x++)
	{
		int y = 1 + DKRandom() % (height-1);

		splinePoints.Insert(DKVector3( static_cast<float>(x) / static_cast<float>(width), static_cast<float>(y) / static_cast<float>(height), 0.0 ));
	}
	SetRedraw();
}

void SplineTest::NextSpline(void)
{
	switch (splineType)
	{
	case DKSpline::CatmullRom:
		splineType = DKSpline::UniformCubic;
		break;
	case DKSpline::UniformCubic:
		splineType = DKSpline::Hermite;
		break;
	case DKSpline::Hermite:
		splineType = DKSpline::Bezier;
		break;
	default:
		splineType = DKSpline::CatmullRom;
		break;
	}
	SetRedraw();
}
