#include "StdAfx.h"
#include <math.h>
#include "RenderFrame.h"
#include "Polygonizer.h"

#define VOLUME_COMPACT_DELAY		5.0

#define DEFAULT_CAMERA_FOV		(2.0 * atan(35.0/85.0))

#define VOXEL_MESH_WIDTH	16
#define VOXEL_MESH_HEIGHT	16
#define VOXEL_MESH_DEPTH	16

template <typename T>
inline T VoxelSizeDiv(T s, T unit)
{
	return ((s / unit) + (s % unit ? 1 : 0 ));
}

template <typename T>
inline T LocationToIndex(T x, T y, T z, T w, T h, T d)
{
	return x + (w * y) + (w * h * z);
}

template <typename T>
inline void IndexToLocation(T idx, T w, T h, T d, T *x, T *y, T *z)
{
	T xy = w * h;
	T xy2 = idx % xy;
	*z = idx / xy;
	*y = xy2 / w;
	*x = xy2 % w;
}

RenderFrame::RenderFrame(void)
	: cameraMove(false)
	, sliderX(-1), sliderY(-1), sliderZ(-1)
	, volumeSizeX(0), volumeSizeY(0), volumeSizeZ(0)
	, voxelColor(1,1,1)
	, bgColor(0,0,0)
	, terminate(false)
	, volumeUpdated(false)
{
}

RenderFrame::~RenderFrame(void)
{
}

void RenderFrame::OnLoaded(void)
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	DKApplication* app = DKApplication::Instance();
	DKResourcePool pool;
	pool.AddResourceData("default_font.ttf", app->LoadStaticResource("default_font.ttf"));
	pool.AddResourceData("voxel.DKMATERIAL", app->LoadStaticResource("voxel.DKMATERIAL"));
	pool.AddSearchPath(app->EnvironmentPath(DKApplication::SystemPathAppResource));

	defaultFont = DKFont::Create(pool.LoadResourceData("default_font.ttf"));
	defaultFont->SetStyle(14);

	DKSize contentSize = ContentResolution();
	float aspectRatio = contentSize.width / contentSize.height;
	camera.SetView(DKVector3(0, 0, 50), DKVector3(0,0,-1), DKVector3(0,1,0));
	camera.SetPerspective(DEFAULT_CAMERA_FOV, aspectRatio, 1, 5000);

	if (editorMaterial == NULL)
	{
		// 기본 (에디터용) 머티리얼
		const DKMaterial::ShaderSource defaultVertexShader = {L"defaultVertexShader",
			"#version 120\n"
			"uniform mat4 modelViewProjectionMatrix;"
			"attribute vec3 position;"
			"void main(void) {"
			"	gl_Position = modelViewProjectionMatrix * vec4(position,1);"
			"}", DKShader::TypeVertexShader, 0};
		const DKMaterial::ShaderSource defaultFragmentShader = {L"defaultFragmentShader",
			"#version 120\n"
			"uniform vec4 color;"
			"void main(void) {"
			"	gl_FragColor = color;"
			"}", DKShader::TypeFragmentShader, 0};

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
			editorMaterial = material;
	}
	modelScene = DKObject<DKScene>::New();
	modelScene->drawMode = DKScene::DrawMeshes;
	modelScene->ambientColor = DKColor(0.5, 0.5, 0.5);
	modelScene->lights.Add( DKLight(DKVector3(-1,-1,-1), DKColor(1,1,1)) );

	editorScene = DKObject<DKScene>::New();
	//editorScene->drawMode = DKScene::DrawCollisionShapes | DKScene::DrawConstraints | DKScene::DrawConstraintLimits | DKScene::DrawCollisionAABB | DKScene::DrawMeshes;
	editorScene->drawMode = DKScene::DrawMeshes;
	editorScene->ambientColor = DKColor(0.5, 0.5, 0.5);
	editorScene->lights.Add( DKLight(DKVector3(-1,-1,-1), DKColor(1,1,1)) );

	sliderBodyX = DKOBJECT_NEW DKRigidBody("SliderX");
	sliderBodyY = DKOBJECT_NEW DKRigidBody("SliderY");
	sliderBodyZ = DKOBJECT_NEW DKRigidBody("SliderZ");

	const float alpha = 0.25;
	sliderPlaneX = CreateXZPlaneMesh(editorMaterial);
	sliderPlaneY = CreateXZPlaneMesh(editorMaterial);
	sliderPlaneZ = CreateXZPlaneMesh(editorMaterial);

	sliderPlaneX->SetLocalTransform(DKQuaternion(DKVector3(0, 0, 1), DKL_MATH_PI_2));
	sliderPlaneY->SetLocalTransform(DKNSTransform::identity);
	sliderPlaneZ->SetLocalTransform(DKQuaternion(DKVector3(1, 0, 0), -DKL_MATH_PI_2));

	sliderPlaneX->SetMaterialProperty("color", DKMaterial::PropertyArray(DKColor(1,0,0,alpha).val, 4));
	sliderPlaneY->SetMaterialProperty("color", DKMaterial::PropertyArray(DKColor(0,1,0,alpha).val, 4));
	sliderPlaneZ->SetMaterialProperty("color", DKMaterial::PropertyArray(DKColor(0,0,1,alpha).val, 4));

	sliderBodyX->AddChild(sliderPlaneX);
	sliderBodyY->AddChild(sliderPlaneY);
	sliderBodyZ->AddChild(sliderPlaneZ);

	volumeBounds = CreateWireframeBoxModel(editorMaterial);
	volumeBounds->SetName("VolumeBounds");
	editorScene->AddObject(volumeBounds);

	voxelStorage = DKObject<DKVoxel32SparseVolume>::New();
	voxelMaterial = pool.LoadResource("voxel.DKMATERIAL").SafeCast<DKMaterial>();

	this->SetVolumeSize(10, 10, 10);
	this->SetSliderPos(0,0,0);

	DKObject<DKOperation> threadOp = (DKOperation*) DKFunction([this]()
	{
		cond.Lock();
		while (!this->terminate)
		{
			if (this->volumeUpdated)
			{
				DKCriticalSection<DKSpinLock> guard(this->lock);
				if (this->voxelStorage)
				{
					DKLog("VolumeStorage compacting...\n");
					this->voxelStorage->Compact();
					DKLog("VolumeStorage compacted.\n");
				}
				this->volumeUpdated = false;
			}
			cond.Wait();
		}
		cond.Unlock();
	})->Invocation();
	this->thread = DKThread::Create(threadOp);
}

void RenderFrame::OnUnload(void)
{
	cond.Lock();
	terminate = true;
	cond.Broadcast();
	cond.Unlock();
	thread->WaitTerminate();

	modelScene = NULL;
	editorScene = NULL;

	defaultFont = NULL;
	editorMaterial = NULL;
	voxelMaterial = NULL;

	sliderBodyX = NULL;
	sliderBodyY = NULL;
	sliderBodyZ = NULL;
	sliderPlaneX = NULL;
	sliderPlaneY = NULL;
	sliderPlaneZ = NULL;
	volumeBounds = NULL;
	
	voxelStorage = NULL;
	voxelModels.Clear();
}

void RenderFrame::OnContentResized(void)
{ 
	DKLog("%s\n", DKLIB_FUNCTION_NAME);

	DKSize contentSize = ContentResolution();
	float aspectRatio = contentSize.width / contentSize.height;
	camera.SetPerspective(DEFAULT_CAMERA_FOV, aspectRatio, 1, 5000);
}

void RenderFrame::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	if (buttonId == 0)
		cameraMove = true;
	else if (buttonId == 1)
	{
		const DKSize& frameSize = this->ContentScale();

		// 스크린 한가운데가 0,0 이므로 변환함. (-1.0 ~ 1.0)
		float pX = (pos.x / frameSize.width) * 2.0 - 1.0;
		float pY = (pos.y / frameSize.height) * 2.0 - 1.0;

		DKVector3 camPos = this->camera.ViewPosition();

		const DKMatrix4& viewProj = this->camera.ViewMatrix() * this->camera.ProjectionMatrix();
		const DKMatrix4 viewProjInv = DKMatrix4(viewProj).Inverse();

		DKVector3 rayBeginOnScreen = DKVector3(pX, pY, -1.0);
		DKVector3 rayEndOnScreen = DKVector3(pX, pY, 1.0);

		DKVector3 rayBeginOnWorld = rayBeginOnScreen * viewProjInv;
		DKVector3 rayEndOnWorld = rayEndOnScreen * viewProjInv;

		DKVector3 hitPoint(0,0,0);
		DKCollisionObject* obj = this->editorScene->RayTestClosest(rayBeginOnWorld, rayEndOnWorld, &hitPoint);
		if (obj)
		{
			bool lc = Screen()->Window()->KeyState(0, DKVK_LEFT_CONTROL);
			bool rc = Screen()->Window()->KeyState(0, DKVK_RIGHT_CONTROL);
			bool ctrl = lc | rc;

			this->SetVolumeData(floor(hitPoint.x), floor(hitPoint.y), floor(hitPoint.z), ctrl);
		}
		//else { DKLog("rayTest failed\n"); }
	}
}

void RenderFrame::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	if (buttonId == 0)
		cameraMove = false;
}

void RenderFrame::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
//	DKLog("%s(%.1f, %.1f)\n", DKLIB_FUNCTION_NAME, pos.x, pos.y);

	if (cameraMove)
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
	//DKLog("%s (%f,%f)\n", DKLIB_FUNCTION_NAME, delta.x, delta.y);

	DKVector3 cameraPos = camera.ViewPosition();
	DKVector3 dir = (cameraTarget - cameraPos).Normalize();

	cameraPos += dir * delta.y * this->ContentResolution().height * 1;

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

	DKCriticalSection<DKSpinLock> guard(lock);

	modelScene->Update(timeDelta, tick);
	editorScene->Update(timeDelta, tick);

	SetRedraw();
}

void RenderFrame::OnRender(DKRenderer& renderer) const
{
	DKCriticalSection<DKSpinLock> guard(lock);

	renderer.Clear(bgColor);

	renderer.RenderScene(modelScene, camera, 0);
	renderer.RenderScene(editorScene, camera, 0);
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

void RenderFrame::ResetVoxelData(void)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	this->voxelStorage->ResetContents();
	for (VoxelModel& vm : voxelModels)
		vm.dirty = true;
	Screen()->PostOperation(DKFunction(this, &RenderFrame::ReshapeVolume)->Invocation());
}

void RenderFrame::SetVolumeSize(int x, int y, int z)
{
	DKCriticalSection<DKSpinLock> guard(lock);

	DKASSERT_DEBUG(x > 0);
	DKASSERT_DEBUG(y > 0);
	DKASSERT_DEBUG(z > 0);

	float thick = 0.1;

	float fx = float(x) * 0.5;
	float fy = float(y) * 0.5;
	float fz = float(z) * 0.5;

	if (volumeSizeY != y || volumeSizeZ != z)
	{
		DKObject<DKBoxShape> shape = DKOBJECT_NEW DKBoxShape(thick, fy, fz);
		DKObject<DKCompoundShape> compound = DKObject<DKCompoundShape>::New();
		compound->AddShape(shape, DKVector3(0,fy,fz));
		sliderBodyX->SetCollisionShape(compound);
		sliderPlaneX->SetScale(DKVector3(y, 1, z));
	}
	if (volumeSizeX != x || volumeSizeZ != z)
	{
		DKObject<DKBoxShape> shape = DKOBJECT_NEW DKBoxShape(fx, thick, fz);
		DKObject<DKCompoundShape> compound = DKObject<DKCompoundShape>::New();
		compound->AddShape(shape, DKVector3(fx,0,fz));
		sliderBodyY->SetCollisionShape(compound);
		sliderPlaneY->SetScale(DKVector3(x, 1, z));
	}
	if (volumeSizeX != x || volumeSizeY != y)
	{
		DKObject<DKBoxShape> shape = DKOBJECT_NEW DKBoxShape(fx, fy, thick);
		DKObject<DKCompoundShape> compound = DKObject<DKCompoundShape>::New();
		compound->AddShape(shape, DKVector3(fx,fy,0));
		sliderBodyZ->SetCollisionShape(compound);
		sliderPlaneZ->SetScale(DKVector3(x, 1, y));
	}

	volumeSizeX = x;
	volumeSizeY = y;
	volumeSizeZ = z;

	voxelStorage->SetDimensions(x, y, z);

	auto createVoxelMesh = [this]()->DKObject<DKStaticMesh>
	{
		DKVertexBuffer::Decl decls[3] = {
			{DKVertexStream::StreamPosition, "position", DKVertexStream::TypeFloat3, false, 0},
#if VOXEL_16BIT_NORMAL
			{DKVertexStream::StreamNormal, "normal", DKVertexStream::TypeShort4, true, MEMBER_OFFSET(VolumeVertex, normal)},
#else
			{DKVertexStream::StreamNormal, "normal", DKVertexStream::TypeFloat3, false, MEMBER_OFFSET(VolumeVertex, normal)},
#endif
			{DKVertexStream::StreamColor, "color", DKVertexStream::TypeUByte4, true, MEMBER_OFFSET(VolumeVertex, color)},
		};
		DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(decls, 3, 0, sizeof(VolumeVertex), 0, DKVertexBuffer::MemoryLocationDynamic, DKVertexBuffer::BufferUsageDraw);
		DKObject<DKStaticMesh> mesh = DKObject<DKStaticMesh>::New();
		mesh->SetDrawFace(DKMesh::DrawFaceCCW);
		mesh->SetDefaultPrimitiveType(DKPrimitive::TypeTriangles);
		mesh->AddVertexBuffer(vb);
		mesh->SetMaterial(voxelMaterial);
		return mesh;
	};
	int w = VoxelSizeDiv<int>(volumeSizeX, VOXEL_MESH_WIDTH);
	int h = VoxelSizeDiv<int>(volumeSizeY, VOXEL_MESH_HEIGHT);
	int d = VoxelSizeDiv<int>(volumeSizeZ, VOXEL_MESH_DEPTH);

	voxelModels.Resize( w * h * d);
	for (VoxelModel& vm : voxelModels)
	{
		if (vm.surface == NULL)
		{
			vm.surface = createVoxelMesh();
			vm.surface->SetName("Surface");
			vm.surface->SetHidden(false);
			modelScene->AddObject(vm.surface);
		}
		vm.dirty = true;
	}
	DKLog("Volume Resized: %d x %d x %d (%d units)\n", volumeSizeX, volumeSizeY, volumeSizeZ, (int)voxelModels.Count());

	DKVector3 scale(x,y,z);
	this->volumeBounds->SetScale(scale);
	this->cameraTarget = scale * 0.5;
	DKVector3 pos = camera.ViewPosition();
	camera.SetView(pos, cameraTarget - pos, DKVector3(0,1,0));

	Screen()->PostOperation(DKFunction(this, &RenderFrame::ReshapeVolume)->Invocation());
}

void RenderFrame::SetSliderPos(int x, int y, int z)
{
	DKCriticalSection<DKSpinLock> guard(lock);

	DKASSERT_DEBUG(x >= 0 && x < volumeSizeX);
	DKASSERT_DEBUG(y >= 0 && y < volumeSizeY);
	DKASSERT_DEBUG(z >= 0 && z < volumeSizeZ);

	if (sliderX != x)
	{
		sliderBodyX->SetLocalTransform(DKVector3(float(x)+0.5,0,0));
		sliderX = x;
	}
	if (sliderY != y)
	{
		sliderBodyY->SetLocalTransform(DKVector3(0,float(y)+0.5,0));
		sliderY = y;
	}
	if (sliderZ != z)
	{
		sliderBodyZ->SetLocalTransform(DKVector3(0,0,float(z)+0.5));
		sliderZ = z;
	}
	DKLog("slider pos: %d, %d, %d\n", x, y, z);
}

void RenderFrame::SetSliderPlaneVisible(bool x, bool y, bool z)
{
	DKCriticalSection<DKSpinLock> guard(lock);

	auto updateSlider = [](DKScene* scene, DKRigidBody* s, bool visible)
	{
		if (visible)
		{
			if (s->Scene() == NULL)
				scene->AddObject(s);
		}
		else
		{
			if (s->Scene())
				s->RemoveFromScene();
		}
	};

	updateSlider(editorScene, sliderBodyX, x);
	updateSlider(editorScene, sliderBodyY, y);
	updateSlider(editorScene, sliderBodyZ, z);
}

void RenderFrame::SetVoxelColor(const DKColor& c)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	this->voxelColor = c;
}

void RenderFrame::SetWireframeColor(const DKColor& c)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	volumeBounds->SetMaterialProperty("color", DKMaterial::PropertyArray(c.val, 4));
}

void RenderFrame::SetBGColor(const DKColor& c)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	this->bgColor = c;
}

void RenderFrame::SetVolumeData(int x, int y, int z, bool a)
{
	float offset = 0.2;
	
	//DKLog("SetVolumeData:%d, %d, %d (%d)\n", x,y,z,a);
	DKVoxel32 voxel;
	if (voxelStorage->GetVoxelAtLocation(x, y, z, voxel))
	{
		float val = static_cast<float>(voxel.level) / 255.0;
		if (a)
			val -= offset;
		else
			val += offset;

		val = Clamp(val, 0.0f, 1.0f);

		DKVoxel32 voxel2;
		voxel2.level = static_cast<unsigned char>(val * 255.0);
		DKColor::RGBA32 rgb = this->voxelColor.RGBA32Value();
		voxel2.data[0] = rgb.r;
		voxel2.data[1] = rgb.g;
		voxel2.data[2] = rgb.b;

		if (voxel.uintValue != voxel2.uintValue)
		{
			DKLog("SetVoxel(%d,%d,%d) (%d -> %d)\n", x, y, z, (int)voxel.level, (int)voxel2.level);
			bool b = voxelStorage->SetVoxelAtLocation(x, y, z, voxel2);
			DKASSERT_DEBUG(b);

			int w = VoxelSizeDiv<int>(volumeSizeX, VOXEL_MESH_WIDTH);
			int h = VoxelSizeDiv<int>(volumeSizeY, VOXEL_MESH_HEIGHT);
			int d = VoxelSizeDiv<int>(volumeSizeZ, VOXEL_MESH_DEPTH);

			// 현재 위치에서 뒤로 두칸, 앞으로 한칸 영역을 갱신한다.
			int x0 = Max<int>(x - 2, 0);
			int y0 = Max<int>(y - 2, 0);
			int z0 = Max<int>(z - 2, 0);
			int x1 = Min<int>(x + 1, volumeSizeX - 1);
			int y1 = Min<int>(y + 1, volumeSizeY - 1);
			int z1 = Min<int>(z + 1, volumeSizeZ - 1);

			for (int zz = z0; zz <= z1; ++zz)
			{
				for (int yy = y0; yy <= y1; ++yy)
				{
					for (int xx = x0; xx <= x1; ++xx)
					{
						int x2 = xx / VOXEL_MESH_WIDTH;
						int y2 = yy / VOXEL_MESH_HEIGHT;
						int z2 = zz / VOXEL_MESH_DEPTH;

						int index = LocationToIndex(x2,y2,z2, w,h,d);
						voxelModels.Value(index).dirty = true;
					}
				}
			}
			Screen()->PostOperation(DKFunction(this, &RenderFrame::ReshapeVolume)->Invocation());

			cond.Lock();
			this->volumeUpdated = true;
			this->volumeUpdatedTimer.Reset();
			cond.Unlock();

			// delay 후에 다른 쓰레드에서 volume 이 compact 되게끔 한다.
			auto fn = [=]()
			{
				cond.Lock();
				if (this->volumeUpdated)
				{
					double e = volumeUpdatedTimer.Elapsed();
					// delay 가 아직 안지났으면 그냥 무시함.
					if (e > VOLUME_COMPACT_DELAY || fabs(e - VOLUME_COMPACT_DELAY) < 0.001)
						cond.Signal();
				}
				cond.Unlock();
			};
			Screen()->PostOperation(DKFunction(fn)->Invocation(), VOLUME_COMPACT_DELAY);
		}
	}
	else
	{
		DKASSERT_DEBUG(0);
	}
}

void RenderFrame::ReshapeVolume(void)
{
	DKCriticalSection<DKSpinLock> guard(lock);

	Polygonizer polygonizer;

	auto polygonizerMesh = [&polygonizer](DKStaticMesh* mesh)
	{
		if (polygonizer.triangleIndices.Count() > 0)
		{
			const VolumeVertex* verts = polygonizer.triangleVertices;
			size_t numVerts = polygonizer.triangleVertices.Count();
			const unsigned int* indices = polygonizer.triangleIndices;
			size_t numIndices = polygonizer.triangleIndices.Count();

	//		DKLog("%llu vertices, %llu triangles generated.\n",
	//			(unsigned long long)numVerts,
	//			(unsigned long long)(numIndices / 3));

			DKObject<DKIndexBuffer> ib = DKIndexBuffer::Create(indices, numIndices, DKPrimitive::TypeTriangles, DKIndexBuffer::MemoryLocationStatic, DKIndexBuffer::BufferUsageDraw);
			mesh->SetIndexBuffer(ib);
			mesh->VertexBufferAtIndex(0)->UpdateContent(verts, numVerts, DKVertexBuffer::MemoryLocationDynamic, DKVertexBuffer::BufferUsageDraw);
		}
		else
		{
			mesh->SetIndexBuffer(NULL);
			mesh->VertexBufferAtIndex(0)->UpdateContent(0, 0, DKVertexBuffer::MemoryLocationDynamic, DKVertexBuffer::BufferUsageDraw);
		}
	};

	int w = VoxelSizeDiv<int>(volumeSizeX, VOXEL_MESH_WIDTH);
	int h = VoxelSizeDiv<int>(volumeSizeY, VOXEL_MESH_HEIGHT);
	int d = VoxelSizeDiv<int>(volumeSizeZ, VOXEL_MESH_DEPTH);

	DKObject<Polygonizer::Picker> voxelPicker = DKFunction([this](int x, int y, int z)->DKVoxel32
	{
		DKVoxel32 result;
		result.uintValue = 0;
		if (x >= 0 && y >= 0 && z >= 0)
			this->voxelStorage->GetVoxelAtLocation(x, y, z, result);
		return result;
	});

	Polygonizer::Range range;
	size_t index = 0;
	for (int z = 0; z < d; ++z)
	{
		range.beginZ = z * VOXEL_MESH_DEPTH;
		range.endZ = range.beginZ + VOXEL_MESH_DEPTH;
		if (range.endZ >= volumeSizeZ)	range.endZ = volumeSizeZ -1;

		for (int y = 0; y < h; ++y)
		{
			range.beginY = y * VOXEL_MESH_HEIGHT;
			range.endY = range.beginY + VOXEL_MESH_HEIGHT;
			if (range.endY >= volumeSizeY)	range.endY = volumeSizeY -1;
			
			for (int x = 0; x < w; ++x)
			{
				range.beginX = x * VOXEL_MESH_WIDTH;
				range.endX = range.beginX + VOXEL_MESH_WIDTH;
				if (range.endX >= volumeSizeX)	range.endX = volumeSizeX -1;

				VoxelModel& vm = voxelModels.Value(index);
				if (vm.dirty)		// 표면 복셀 갱신
				{
					polygonizer.Reset();
					polygonizer.Polygonize(range, voxelPicker);
					polygonizer.Finalize();
					polygonizerMesh(vm.surface);
					vm.dirty = false;

				//	DKLog("model index:%d, (%d-%d),(%d-%d),(%d-%d) reshaped.\n",
				//		index, range.beginX, range.endX, range.beginY, range.endY, range.beginZ, range.endZ);
				}
				index++;
			}
		}
	}
}

DKObject<DKStaticMesh> RenderFrame::CreateXZPlaneMesh(DKMaterial* material)
{
	DKVector3 pos[4] = {
		DKVector3(0,0,0),
		DKVector3(0,0,1),
		DKVector3(1,0,0),
		DKVector3(1,0,1)
	};
	DKVertexBuffer::Decl decl = {DKVertexStream::StreamPosition, "", DKVertexStream::TypeFloat3, false, 0};
	DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(&decl, 1, pos, sizeof(DKVector3), 4, DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);

	DKObject<DKStaticMesh> mesh = DKObject<DKStaticMesh>::New();
	mesh->SetDrawFace(DKMesh::DrawFaceBoth);
	mesh->SetDefaultPrimitiveType(DKPrimitive::TypeTriangleStrip);
	mesh->AddVertexBuffer(vb);
	mesh->SetMaterial(material);
	float c[4] = {1,1,1,0.5};
	mesh->SetMaterialProperty(L"color", DKMaterial::PropertyArray(c,4));
	mesh->SetMaterial(material);
	return mesh;
}

DKObject<DKStaticMesh> RenderFrame::CreateWireframeBoxModel(DKMaterial* material)
{
	DKArray<DKVector3> points;
	points.Add(DKVector3(0,0,1));
	points.Add(DKVector3(0,1,1));
	points.Add(DKVector3(1,1,1));
	points.Add(DKVector3(1,0,1));
	points.Add(DKVector3(0,0,0));
	points.Add(DKVector3(0,1,0));
	points.Add(DKVector3(1,1,0));
	points.Add(DKVector3(1,0,0));

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

	DKObject<DKStaticMesh> mesh = DKObject<DKStaticMesh>::New();
	mesh->SetDrawFace(DKMesh::DrawFaceBoth);
	mesh->SetDefaultPrimitiveType(DKPrimitive::TypeLines);
	mesh->AddVertexBuffer(vb);
	mesh->SetIndexBuffer(ib);
	float c[4] = {1,1,1,1};
	mesh->SetMaterialProperty(L"color", DKMaterial::PropertyArray(c,4));
	mesh->SetMaterial(material);
	return mesh;
}
