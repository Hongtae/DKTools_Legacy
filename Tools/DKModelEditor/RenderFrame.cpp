#include "StdAfx.h"
#include <math.h>
#include "RenderFrame.h"


// 기본 머티리얼
DKMaterial::ShaderSource defaultVertexShader = {L"defaultVertexShader",
	"#version 120\n"
	"uniform mat4 modelViewProjectionMatrix;"
	"attribute vec3 position;"
	"void main(void) {"
	"	gl_Position = modelViewProjectionMatrix * vec4(position,1);"
	"}", DKShader::TypeVertexShader, 0};
DKMaterial::ShaderSource defaultFragmentShader = {L"defaultFragmentShader",
	"#version 120\n"
	"uniform vec4 color;"
	"void main(void) {"
	"	gl_FragColor = color;"
	"}", DKShader::TypeFragmentShader, 0};


RenderFrame::RenderFrame(void)
	: mouseLButtonDown(false)
	, mouseRButtonDown(false)
	, mouseWButtonDown(false)
	, cameraNear(1.0f)
	, cameraFar(60000.0f)
{
}

RenderFrame::~RenderFrame(void)
{
}

void RenderFrame::OnLoaded(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	defaultFont = DKFont::Create(DKApplication::Instance()->LoadStaticResource("default_font.ttf"));
	defaultFont->SetStyle(14);

	DKSize contentSize = ContentResolution();
	float aspectRatio = contentSize.width / contentSize.height;
	camera.SetView(DKVector3(0, 0, 50), DKVector3(0,0,-1), DKVector3(0,1,0));
	camera.SetPerspective(DKL_DEGREE_TO_RADIAN(80), aspectRatio, this->cameraNear, this->cameraFar);

	// 라인과 구에 쓰일 머티리얼 생성
	DKObject<DKMaterial>	material = DKObject<DKMaterial>::New();
	// in vec2 defaultPosition
	DKMaterial::StreamProperty position = {DKVertexStream::StreamPosition, DKVertexStream::TypeFloat2, 1};
	// uniform color
	DKMaterial::ShadingProperty color = {DKShaderConstant::UniformUserDefine, DKShaderConstant::TypeFloat4, DKMaterial::PropertyArray(1.0f, 4)};
	// uniform modelViewProjectionMatrix
	DKMaterial::ShadingProperty modelViewProjectionMatrix = {DKShaderConstant::UniformModelViewProjectionMatrix, DKShaderConstant::TypeFloat4x4, DKMaterial::PropertyArray()};

	DKMaterial::RenderingProperty	rp = 
	{
		L"defaultRender", DKMaterial::RenderingProperty::DepthFuncLessEqual, true,
		DKBlendState::defaultAlpha, DKArray<DKMaterial::ShaderSource>(), NULL
	};
	rp.shaders.Add(defaultVertexShader);
	rp.shaders.Add(defaultFragmentShader);

	material->renderingProperties.Add(rp);

	material->streamProperties.Insert(L"position", position);
	material->shadingProperties.Insert(L"color", color);
	material->shadingProperties.Insert(L"modelViewProjectionMatrix", modelViewProjectionMatrix);

	DKMaterial::BuildLog log;
	if (!material->Build(&log))
	{
		DKLog("Building default material error: %ls\n", (const wchar_t*)log.errorLog);
		DKLog("While trying to build shader: %ls, in program: %ls.\n", (const wchar_t*)log.failedShader, (const wchar_t*)log.failedProgram);
	}
	else
		defaultMaterial = material;

	modelScene = DKObject<DKScene>::New();
	modelScene->drawMode = DKScene::DrawCollisionShapes | DKScene::DrawConstraints | DKScene::DrawConstraintLimits | DKScene::DrawCollisionAABB | DKScene::DrawMeshes;

	modelScene->ambientColor = DKColor(0.5, 0.5, 0.5);
	modelScene->lights.Add( DKLight(DKVector3(1,1,1), DKColor(1,1,1)) );

	DKObject<DKStaticMesh> gridModel = CreateGridLinesModel(10, this->defaultMaterial);
	gridModel->SetScale(100);
	modelScene->AddObject(gridModel);
}

void RenderFrame::OnUnload(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	defaultFont = NULL;
	defaultMaterial = NULL;

	targetObject = NULL;

	modelScene = NULL;
}

void RenderFrame::OnContentResized(void)
{ 
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	DKSize contentSize = ContentResolution();
	float aspectRatio = contentSize.width / contentSize.height;
	camera.SetPerspective(DKL_DEGREE_TO_RADIAN(80), aspectRatio, this->cameraNear, this->cameraFar);
}

void RenderFrame::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	if (buttonId == 0)
		mouseLButtonDown = true;
	else if (buttonId == 1)
		mouseRButtonDown = true;
	else if (buttonId == 2)
		mouseWButtonDown = true;
}

void RenderFrame::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	if (buttonId == 0)
		mouseLButtonDown = false;
	else if (buttonId == 1)
		mouseRButtonDown = false;
	else if (buttonId == 2)
		mouseWButtonDown = false;

}

void RenderFrame::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
//	DKLog("%s(%.1f, %.1f)\n", DKLIB_FUNCTION_NAME, pos.x, pos.y);

	if (mouseLButtonDown)
	{
		DKVector3 dir = camera.ViewDirection();
		DKVector3 up = DKVector3(0,1,0);
		DKVector3 left = DKVector3::Cross(dir, up);

		DKQuaternion qY(up, -delta.x);
		DKQuaternion qX(left, delta.y);
		DKQuaternion rot = qX * qY;

		DKVector3 pos = camera.ViewPosition() - cameraTarget;
		pos.Rotate(rot);
		pos += cameraTarget;
		camera.SetView(pos, cameraTarget - pos, up);
	}

	mousePos = pos;
}	

void RenderFrame::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	DKLog("%s (%f,%f)\n", DKLIB_FUNCTION_NAME, delta.x, delta.y);

	DKVector3 cameraPos = camera.ViewPosition();
	DKVector3 dir = (cameraTarget - cameraPos).Normalize();

	cameraPos += dir * delta.y * this->ContentResolution().height * 10;

	if ((cameraPos - cameraTarget).Length() >= 1.0)
	{
		camera.SetView(cameraPos, cameraTarget - cameraPos, DKVector3(0,1,0));
	}
}	

void RenderFrame::OnMouseHover(int deviceId)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}

void RenderFrame::OnMouseLeave(int deviceId)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}											

void RenderFrame::OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent)
{ 
	this->timeElapsed = timeDelta;
	this->timeCurrent = timeCurrent;

	Lock();

	modelScene->Update(timeDelta, tick);

	Unlock();

	SetRedraw();
}

void RenderFrame::OnRender(DKRenderer& renderer) const
{
	Lock();

	renderer.Clear(DKColor(1, 0, 1));

	renderer.RenderScene(modelScene, camera, 0);

	Unlock();
}

void RenderFrame::OnKeyDown(int deviceId, DKVirtualKey key)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void RenderFrame::OnKeyUp(int deviceId, DKVirtualKey key)
{  
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void RenderFrame::OnTextInput(int deviceId, const DKString& str)
{   
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void RenderFrame::OnTextInputCandidate(int deviceId, const DKString& str)
{   
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}	

void RenderFrame::SetModel(DKModel* model)
{
	Lock();

	if (this->targetObject)
		this->targetObject->RemoveFromScene();

	this->targetObject = NULL;

	if (model)
	{
		cameraTarget = DKVector3(0,0,0);

		this->targetObject = model;
		if (!modelScene->AddObject(model))
			DKLog("ERROR: model:%ls failed to add scene!\n", (const wchar_t*)model->Name());

		/*
		const DKAABox& box = targetModel->BoundingAABox();
		if (box.IsValid())
		{
			DKVector3 center = (box.positionMax + box.positionMin) / 2;
			helperScene->AddObject(boxModel, DKAffineTransform3().Scale(box.positionMax - center).Translate(center), boxModel);
		}
		const DKSphere& sphere = targetModel->BoundingSphere();
		if (sphere.IsValid())
		{
			helperScene->AddObject(sphereModel, DKAffineTransform3().Scale(sphere.radius).Translate(sphere.center), sphereModel);
		}
		*/
	}
	else
	{
		cameraTarget = DKVector3(0,0,0);
	}
	DKVector3 pos = camera.ViewPosition();
	camera.SetView(pos, cameraTarget - pos, DKVector3(0,1,0));

	Unlock();
}

void RenderFrame::Lock(void) const
{
	lockRenderer.Lock();
}

void RenderFrame::Unlock(void) const
{
	lockRenderer.Unlock();
}

DKObject<DKStaticMesh> RenderFrame::CreateGridLinesModel(int div, DKMaterial* material)
{
	DKArray<DKLine>	lines;
	lines.Add(DKLine(DKVector3(1,0,0), DKVector3(-1,0,0)));
	lines.Add(DKLine(DKVector3(0,0,1), DKVector3(0,0,-1)));

	for (int i = 0; i < div; i++)
	{
		lines.Add(DKLine(DKVector3(float(i+1)/float(div), 0, 1), DKVector3(float(i+1)/float(div), 0, -1)));
		lines.Add(DKLine(DKVector3(-float(i+1)/float(div), 0, 1), DKVector3(-float(i+1)/float(div), 0, -1)));
		lines.Add(DKLine(DKVector3(1, 0, float(i+1)/float(div)), DKVector3(-1, 0, float(i+1)/float(div))));
		lines.Add(DKLine(DKVector3(1, 0, -float(i+1)/float(div)), DKVector3(-1, 0, -float(i+1)/float(div))));
	}

	return CreateWireframeModel(lines, material);
}

DKObject<DKStaticMesh> RenderFrame::CreateBoundSphereModel(int div, DKMaterial* material)
{
	div = Clamp<int>(div, 0, 89);		// 89 를 넘기면 인덱스가 unsigned short 범위를 벗어난다.

	DKArray<unsigned short> indices;
	DKArray<DKVector3> points;

	int vDiv = div*2+2;		// 세로로 반바퀴 (위에서 아래까지 반원)
	int hDiv = vDiv*2;		// 가로로 한바퀴 (가로로 원)

	points.Reserve(vDiv * hDiv);
	indices.Reserve(vDiv * hDiv);

	points.Add(DKVector3(0,1,0));	// 맨 위
	points.Add(DKVector3(0,-1,0));	// 맨 아래

	int offset = 2;
	for (int i = 1; i < vDiv; i++)
	{
		float y = cos(DKL_PI * (float(i)/float(vDiv)));

		DKVector3 p = DKVector3(0,1,0).RotateX(DKL_PI * (float(i)/float(vDiv)));

		// 가로로 둥근 원 추가
		for (int k = 0; k < hDiv; k++)
		{
			DKVector3 p2(p);
			p2.RotateY(DKL_PI * float(k)/float(vDiv));

			points.Add(p2);

			indices.Add(offset + k);

			if (k == hDiv - 1)
				indices.Add(offset);
			else
				indices.Add(offset + k+1);
		}
		offset += hDiv;
	}
	// 세로로 반원 인덱스만 추가
	for (int i = 0; i < hDiv; i++)
	{
		indices.Add(0);
		for (int k = 0; k < vDiv-1; k++)
		{
			int index = 2 + (k * hDiv) + i;
			indices.Add(index);
			indices.Add(index);
		}
		indices.Add(1);
	}

	return CreateWireframeModel(points, indices, material);
}

DKObject<DKStaticMesh> RenderFrame::CreateBoxModel(DKMaterial* material)
{
	DKArray<DKVector3> points;
	points.Add(DKVector3(-1,-1,1));
	points.Add(DKVector3(-1,1,1));
	points.Add(DKVector3(1,1,1));
	points.Add(DKVector3(1,-1,1));
	points.Add(DKVector3(-1,-1,-1));
	points.Add(DKVector3(-1,1,-1));
	points.Add(DKVector3(1,1,-1));
	points.Add(DKVector3(1,-1,-1));

	DKArray<unsigned short> indices;
	indices.Add(0);	indices.Add(1);
	indices.Add(1);	indices.Add(2);
	indices.Add(2); indices.Add(3);
	indices.Add(3); indices.Add(0);

	indices.Add(0); indices.Add(4);
	indices.Add(1); indices.Add(5);
	indices.Add(2); indices.Add(6);
	indices.Add(3); indices.Add(7);

	indices.Add(4);	indices.Add(5);
	indices.Add(5);	indices.Add(6);
	indices.Add(6); indices.Add(7);
	indices.Add(7); indices.Add(4);

	return CreateWireframeModel(points, indices, material);
}

DKObject<DKStaticMesh> RenderFrame::CreateWireframeModel(const DKArray<DKLine>& lines, DKMaterial* material)
{
	DKVertexBuffer::Decl d = {DKVertexStream::StreamPosition, L"position", DKVertexStream::TypeFloat3, false, 0};
	DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(&d, 1, (const DKLine*)lines, sizeof(DKVector3), lines.Count() * 2, DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);

	DKObject<DKStaticMesh> mesh = DKObject<DKStaticMesh>::New();
	mesh->SetDrawFace(DKMesh::DrawFaceBoth);
	mesh->SetDefaultPrimitiveType(DKPrimitive::TypeLines);
	mesh->AddVertexBuffer(vb);
	float c[4] = {1,1,1,1};
	mesh->SetMaterialProperty(L"color", DKMaterial::PropertyArray(c,4));
	mesh->SetMaterial(material);
	return mesh;
}

DKObject<DKStaticMesh> RenderFrame::CreateWireframeModel(const DKArray<DKVector3>& points, const DKArray<unsigned short>& indices, DKMaterial* material)
{
	DKVertexBuffer::Decl d = {DKVertexStream::StreamPosition, L"position", DKVertexStream::TypeFloat3, false, 0};
	DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(&d, 1, (const DKVector3*)points, sizeof(DKVector3), points.Count(), DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);
	DKObject<DKIndexBuffer> ib = DKIndexBuffer::Create(indices, indices.Count(), DKPrimitive::TypeLines, DKIndexBuffer::MemoryLocationStatic, DKIndexBuffer::BufferUsageDraw);

	DKObject<DKStaticMesh>   mesh = DKObject<DKStaticMesh>::New();
	mesh->SetDrawFace(DKMesh::DrawFaceBoth);
	mesh->SetDefaultPrimitiveType(DKPrimitive::TypeLines);
	mesh->AddVertexBuffer(vb);
	mesh->SetIndexBuffer(ib);
	float c[4] = {1,1,1,1};
	mesh->SetMaterialProperty(L"color", DKMaterial::PropertyArray(c,4));
	mesh->SetMaterial(material);
	return mesh;
}
