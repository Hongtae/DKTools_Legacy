#include "StdAfx.h"
#include "TestFrame1.h"
#include <math.h>

extern DKString g_resDir;

TestFrame1::TestFrame1(void)
	: gyroCameraState(false)
	, manualMoveState(false)	
	, skeletalViewState(false)
	, playAnimationState(false)
	, animationToggleState(false)
	, animationReverseState(false)
	, nextCharacterIndex(0)
{
}

TestFrame1::~TestFrame1(void)
{
}

void TestFrame1::OnLoaded(void)
{
	DKTimer timer;
	timer.Reset();

	resourcePool.AddSearchPath(g_resDir);
	resourcePool.AddSearchPath(g_resDir + "/Fonts");
	resourcePool.AddSearchPath(g_resDir + "/DarkIllusion");
	resourcePool.AddSearchPath(g_resDir + "/katana");

	// 카메라 설정
	DKPoint pt = LocalToPixel(DKPoint(1,1));
	float fAspectRatio = pt.x / pt.y;

	cameraTarget = DKVector3(0,100,0);
	DKVector3 camPosition = DKVector3(0, cameraTarget.y, 120);
	camera.SetView(camPosition, cameraTarget - camPosition, DKVector3(0,1,0));
	camera.SetPerspective(DKL_DEGREE_TO_RADIAN(90), fAspectRatio, 25, 1000);

	// 폰트 로딩
	const DKString fontFile = L"NanumGothic.ttf";
	fontRegular = DKFont::Create(resourcePool.LoadResourceData(fontFile));
	fontRegular->SetStyle(24);
	fontOutline = DKFont::Create(resourcePool.LoadResourceData(fontFile));
	fontOutline->SetStyle(24, 0, 2);
	fontRegularUI = DKFont::Create(resourcePool.LoadResourceData(fontFile));
	fontRegularUI->SetStyle(18);
	fontOutlineUI = DKFont::Create(resourcePool.LoadResourceData(fontFile));
	fontOutlineUI->SetStyle(18, 0, 2);

	this->katana = resourcePool.LoadResource(L"katana.DKMODEL").SafeCast<DKModel>();

	mainScene = DKOBJECT_NEW DKScene();

	// 조명 설정
	mainScene->lights.Add( DKLight(DKVector3(1,1,1), DKColor(1,1,1)) );
	mainScene->ambientColor = DKColor(0.25,0.25,0.25,1.0);

	this->animations.Insert(L"walk", resourcePool.LoadResource(L"walk.DKANIMATION").SafeCast<DKAnimation>());
	this->animations.Insert(L"attack", resourcePool.LoadResource(L"attack.DKANIMATION").SafeCast<DKAnimation>());

	this->animationController = this->animations.Value(L"walk")->CreateLoopController();
	this->animationController->Play();
	this->animationController->SetSpeed(1.0);

	// 쉐이더 미리 읽어놓음.
	resourcePool.LoadResource(L"skin.DKMATERIAL");
	resourcePool.LoadResource(L"static.DKMATERIAL");

	// 자이로 센서
	gyroScope = GyroScope::SharedInstance();
		
	// 바닥 만들기
	if (true)
	{
		DKObject<DKMaterial> planeMaterial = resourcePool.LoadResource(L"plane.DKMATERIAL").SafeCast<DKMaterial>();

		DKObject<DKTexture2D> tex = resourcePool.LoadResource(L"koo.jpg").SafeCast<DKTexture2D>();
		
		struct PlaneVertexData
		{
			DKVector3 pos;
			DKVector2 tex;
		};		
		PlaneVertexData vert[4] = {
			{DKVector3(-1, 0,-1), DKVector2(0,0)},
			{DKVector3(-1, 0, 1), DKVector2(0,1)},
			{DKVector3( 1, 0,-1), DKVector2(1,0)},
			{DKVector3( 1, 0, 1), DKVector2(1,1)}
		};
		DKVertexBuffer::Decl decls[2] = {
			{DKVertexStream::StreamPosition, L"", DKVertexStream::TypeFloat3, false, 0},
			{DKVertexStream::StreamTexCoord, L"", DKVertexStream::TypeFloat2, false, MEMBER_OFFSET(PlaneVertexData, tex)}
		};
		DKObject<DKVertexBuffer> vb = DKVertexBuffer::Create(decls, 2, vert, sizeof(PlaneVertexData), 4, DKVertexBuffer::MemoryLocationStatic, DKVertexBuffer::BufferUsageDraw);

		DKObject<DKStaticMesh> groundMesh = DKObject<DKStaticMesh>::New();
		groundMesh->SetDrawFace(DKMesh::DrawFaceBoth);
		groundMesh->SetDefaultPrimitiveType(DKPrimitive::TypeTriangleStrip);
		groundMesh->AddVertexBuffer(vb);
		groundMesh->SetSampler(L"diffuseMap", DKMaterial::TextureArray(tex.SafeCast<DKTexture>(), 1), NULL);
		float c[4] = {1,1,1,1};
		groundMesh->SetMaterialProperty(L"color", DKMaterial::PropertyArray(c, 4));
		groundMesh->SetMaterial(planeMaterial);
		const float tileSize = 250;
		groundMesh->SetScale(tileSize);
		groundMesh->SetName("groundMesh");

		DKObject<DKCollisionShape> plane = DKOBJECT_NEW DKStaticPlaneShape(DKVector3(0,1,0), 0);
		DKObject<DKRigidBody> groundShape = DKOBJECT_NEW DKRigidBody(plane, 0);
		groundShape->SetName(L"groundShape");
		groundShape->AddChild(groundMesh);
		this->ground = groundShape;
		mainScene->AddObject(this->ground);
	}

	playAnimation = DKObject<FrameButton>::New();
	playAnimation->SetText(L"Ani on/off");
	playAnimation->SetTextFont(fontRegularUI);
//	playAnimation->SetOutlineFont(fontOutlineUI);
	playAnimation->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	animationToggle = DKObject<FrameButton>::New();
	animationToggle->SetText(L"Ani toggle");
	animationToggle->SetTextFont(fontRegularUI);
//	animationToggle->SetOutlineFont(fontOutlineUI);
	animationToggle->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	animationReverse = DKObject<FrameButton>::New();
	animationReverse->SetText(L"Ani rev");
	animationReverse->SetTextFont(fontRegularUI);
//	animationReverse->SetOutlineFont(fontOutlineUI);
	animationReverse->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	skeletalView = DKObject<FrameButton>::New();
	skeletalView->SetText(L"Skeletal on/off");
	skeletalView->SetTextFont(fontRegularUI);
//	skeletalView->SetOutlineFont(fontOutlineUI);
	skeletalView->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	prevCharacter = DKObject<FrameButton>::New();
	prevCharacter->SetText(L"Prev Char");
	prevCharacter->SetTextFont(fontRegularUI);
//	prevCharacter->SetOutlineFont(fontOutlineUI);
	prevCharacter->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	nextCharacter = DKObject<FrameButton>::New();
	nextCharacter->SetText(L"Next Char");
	nextCharacter->SetTextFont(fontRegularUI);
//	nextCharacter->SetOutlineFont(fontOutlineUI);
	nextCharacter->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);
	
	gyroCamera = DKObject<FrameButton>::New();
	gyroCamera->SetText(L"gyro on/off");
	gyroCamera->SetTextFont(fontRegularUI);
//	gyroCamera->SetOutlineFont(fontOutlineUI);
	gyroCamera->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	manualMove = DKObject<FrameButton>::New();
	manualMove->SetText(L"move?");
	manualMove->SetTextFont(fontRegularUI);
//	manualMove->SetOutlineFont(fontOutlineUI);
	manualMove->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);

	debugTest = DKObject<FrameButton>::New();
	debugTest->SetText(L"debug");
	debugTest->SetTextFont(fontRegularUI);
//	debugTest->SetOutlineFont(fontOutlineUI);
	debugTest->SetCallback(DKFunction(this, &TestFrame1::OnButtonEvent), DKRunLoop::CurrentRunLoop(), this);	

	AddSubframe(gyroCamera);
	AddSubframe(manualMove);
	AddSubframe(skeletalView);
	AddSubframe(prevCharacter);
	AddSubframe(nextCharacter);
	AddSubframe(playAnimation);
	AddSubframe(animationToggle);
	AddSubframe(animationReverse);
	AddSubframe(debugTest);

	charFiles.Add(L"kon.DKMODEL");
	charFiles.Add(L"dih.DKMODEL");
	charFiles.Add(L"dil.DKMODEL");

	nextCharacterIndex = 0;
	//DKRunLoop::CurrentRunLoop()->PostOperation(DKFunction(this, &TestFrame1::OnButtonEvent)->Invocation(FrameButton::ButtonEventActivated, gyroCamera));
	DKRunLoop::CurrentRunLoop()->PostOperation(DKFunction(this, &TestFrame1::OnButtonEvent)->Invocation(FrameButton::ButtonEventActivated, playAnimation));
	DKRunLoop::CurrentRunLoop()->PostOperation(DKFunction(this, &TestFrame1::OnButtonEvent)->Invocation(FrameButton::ButtonEventActivated, nextCharacter));
	
	loadingTime = timer.Elapsed();
}

void TestFrame1::OnUnload(void)
{
	animationController = NULL;
	animations.Clear();
	
	fontRegular = NULL;
	fontOutline = NULL;
	fontRegularUI = NULL;
	fontOutlineUI = NULL;

	gyroCamera = NULL;
	manualMove = NULL;
	skeletalView = NULL;
	playAnimation = NULL;
	animationToggle = NULL;
	animationReverse = NULL;
	prevCharacter = NULL;
	nextCharacter = NULL;
	debugTest = NULL;

	character = NULL;
	katana = NULL;
	ground = NULL;
	mainScene = NULL;

	gyroScope = NULL;
	
	resourcePool.RemoveAllResources();
	resourcePool.RemoveAllResourceData();
	
	DKApplication::Instance()->Terminate(0);
}

void TestFrame1::OnContentResized(void)
{
	DKSize size = ContentResolution();
	float fAspectRatio = size.width / size.height;
	camera.SetPerspective(DKL_DEGREE_TO_RADIAN(90), fAspectRatio, 25, 1000);

	DKSize contentSize = LocalToPixel(DKSize(1,1));
	int buttonMargin = 5;
	int buttonWidth = contentSize.width / 3 - (buttonMargin * 4);
	int buttonHeight = contentSize.height / 15;
	contentSize.height -= 40;		// top margin

	DKSize buttonSize = DKSize(buttonWidth, buttonHeight);
	DKVector2 buttonScale = PixelToLocal(buttonSize).Vector();

	DKPoint buttonPos = DKPoint(buttonMargin, contentSize.height - (buttonHeight + buttonMargin));
	playAnimation->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos.x += buttonWidth + buttonMargin;
	animationToggle->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos.x += buttonWidth + buttonMargin;
	animationReverse->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos = DKPoint(buttonMargin, contentSize.height - (buttonHeight + buttonMargin)*2);
	skeletalView->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos.x += buttonWidth + buttonMargin;
	prevCharacter->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos.x += buttonWidth + buttonMargin;
	nextCharacter->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());
	
	buttonPos = DKPoint(buttonMargin, contentSize.height - (buttonHeight + buttonMargin)*3);
	gyroCamera->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos.x += buttonWidth + buttonMargin;
	manualMove->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());

	buttonPos.x += buttonWidth + buttonMargin;
	debugTest->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(buttonScale), PixelToLocal(buttonPos).Vector()).Matrix3());
}

void TestFrame1::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	int id = Max(deviceId, buttonId);

	//if (mousePositions.Find(id) != NULL)
	//	DKLog("[%s] ERROR: deviceId: %d ALREADY EXIST!\n", DKLIB_FUNCTION_NAME, id);
	
	mousePositions.Value(id) = pos;
	
	Screen()->Window()->ShowMouse(deviceId, false);
	Screen()->Window()->HoldMouse(deviceId, true);
	CaptureMouse(deviceId);	
	
	if (id == 1)
	{
		const DKSize& frameSize = this->ContentScale();

		// 스크린 한가운데가 0,0 이므로 변환함. (-1.0 ~ 1.0)
		float pX = (pos.x / frameSize.width) * 2.0 - 1.0;
		float pY = (pos.y / frameSize.height) * 2.0 - 1.0;

		DKVector3 camPos = this->camera.ViewPosition();

		const DKMatrix4& viewProj = this->camera.ViewProjectionMatrix();
		const DKMatrix4 viewProjInv = DKMatrix4(viewProj).Inverse();

		DKVector3 rayBeginOnScreen = DKVector3(pX, pY, -1.0);
		DKVector3 rayEndOnScreen = DKVector3(pX, pY, 1.0);

		DKVector3 rayBeginOnWorld = rayBeginOnScreen * viewProjInv;
		DKVector3 rayEndOnWorld = rayEndOnScreen * viewProjInv;

		DKVector3 hitPoint(0,0,0);
		DKModel* obj = this->mainScene->RayTestClosest(rayBeginOnWorld, rayEndOnWorld, &hitPoint);
		if (obj)
		{
			DKLog("rayTest: %ls\n", (const wchar_t*)obj->Name());
		}
	}
	//if (deviceId == 0)
	//{
	//	DKVector3 cameraDir = camera.GetDirection();
	//	light.SetDirection(cameraDir);
	//}
	
	DKLog("[%s] mouse down: %d pos:(%.3f, %.3f)\n", DKLIB_FUNCTION_NAME, id, pos.x, pos.y);
}

void TestFrame1::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	int id = Max(deviceId, buttonId);
	
	//if (mousePositions.Find(id) == NULL)
	//	DKLog("[%s] ERROR: deviceId: %d NOT EXIST!\n", DKLIB_FUNCTION_NAME, id);
	
	mousePositions.Remove(id);

	Screen()->Window()->ShowMouse(deviceId, true);
	Screen()->Window()->HoldMouse(deviceId, false);
	ReleaseMouse(deviceId);

	DKLog("[%s] mouse up: %d pos:(%.3f, %.3f)\n", DKLIB_FUNCTION_NAME, id, pos.x, pos.y);
}

void TestFrame1::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	if (deviceId == 0 && mousePositions.Find(deviceId))
	{
		if (gyroCameraState == false)
		{
		//	float x = delta.x * 0.01;		// up 축 회전
		//	float y = delta.y * 0.01;		// left 축 회전
			float x = delta.x * 10;
			float y = delta.y * 10;
			
			DKVector3 dir = camera.ViewDirection();
			DKVector3 up = camera.ViewUp();
			DKVector3 left = DKVector3::Cross(dir, up);
			
			DKQuaternion qX(up, -x);
			DKQuaternion qY(left, y);
			DKQuaternion rot = qX * qY;
			
			if (manualMoveState)		// 카메라 이동
			{
				DKVector3 pos = camera.ViewPosition();
				DKVector3 vec = pos - cameraTarget;				
				pos = vec.Rotate(rot) + cameraTarget;
				up = up.Rotate(rot);

				camera.SetView(pos, cameraTarget - pos, up);
			}
			else								// 조명 이동
			{
				DKLight& light = mainScene->lights.Value(0);
				light.SetDirection(light.Direction() * rot.Inverse()); // 방향은 위치와 반대로 바껴야 하기 때문에 역행렬
			}
		}
	}
}

void TestFrame1::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	//DKLog("%s (%f,%f)\n", DKLIB_FUNCTION_NAME, delta.x, delta.y);

	//DKVector3 pos = m_scene.defaultCamera.Position();
	//pos += m_scene.defaultCamera.Direction() * delta.y;
	//m_scene.defaultCamera.SetPosition(pos);
}

void TestFrame1::OnMouseHover(int deviceId)
{
}

void TestFrame1::OnMouseLeave(int deviceId)
{
}

void TestFrame1::OnUpdate(double timeDelta, DKTimeTick tick, const DKDateTime& timeCurrent)
{
	this->timeDelta = timeDelta;
	this->timeCurrent = timeCurrent;
	this->updateTick = tick;
	
	if (gyroCameraState)
	{
		DKMatrix3 systemRotation = gyroScope->Orientation();

		systemRotation.Transpose();
		// 원점을 중심으로 회전하므로 원점 기준으로 변경함 거리 만 적용
		float cameraDistance = (cameraTarget - camera.ViewPosition()).Length();
		DKVector3 pos = DKVector3(0, 0, cameraDistance) * systemRotation;
		pos += cameraTarget;
		DKVector3 up = DKVector3(0,1,0) * systemRotation;

		camera.SetView(pos, cameraTarget - pos, up);

		mainScene->lights.Value(0).SetDirection(camera.ViewDirection());
	}

	mainScene->Update(timeDelta, tick);

	if (skeletalViewState)
	{
		mainScene->drawMode = DKScene::DrawCollisionShapes | DKScene::DrawSkeletalLines;
	}
	else
	{
		mainScene->drawMode = DKScene::DrawMeshes;
	}

	SetRedraw();
}

void TestFrame1::OnRender(DKRenderer& renderer) const
{
	DKRenderState& renderState = DKOpenGLContext::RenderState();
	renderState.LineWidth(2.0);

	renderer.Clear(DKColor(0,0,1));
	renderer.RenderScene(mainScene, this->camera, 0);

	static DKArray<DKString> drawText;
	drawText.Clear();
	drawText.Add(DKString::Format("Elapsed: %.6f (%.2f fps), Tick:%u", timeDelta, 1 / timeDelta, updateTick));
	drawText.Add(DKString::Format("Loading: %f", this->loadingTime));
	drawText.Add(L"");
	
	DKArray<const DKMap<int, DKPoint>::Pair*> mice;
	mice.Reserve(mousePositions.Count());
	mousePositions.EnumerateForward([&mice](const DKMap<int, DKPoint>::Pair& pair)
									{
										mice.Add(&pair);
									});

	for (int i = 0; i < mice.Count(); i++)
	{
		DKPoint pt = Screen()->Window()->MousePosition(mice.Value(i)->key);
		drawText.Add(DKString::Format("Mouse[%d] (%.3f, %.3f) %s",
									  (int)mice.Value(i)->key, pt.x, pt.y,
									  Screen()->Window()->IsMouseHeld(mice.Value(i)->key) ? "(Hold)":"(Release)"));
	}

	float baseline = fontRegular->Baseline();
	for (int i = 0; i < drawText.Count(); i++)
	{
		DKPoint lineBegin(0, fontRegular->LineHeight() * i + baseline);
		DKPoint lineEnd(fontRegular->LineWidth(drawText.Value(i)), lineBegin.y);
		renderer.RenderText(PixelToLocal(lineBegin), PixelToLocal(lineEnd), drawText.Value(i), fontRegular, DKColor(1,1,1,1));
	}
	
	for (int i = 0; i < mice.Count(); i++)
	{
		DKPoint pos = Screen()->Window()->MousePosition(mice.Value(i)->key);
		
		static const int size = 100;
		DKRect rc = DKRect(pos - size/2, DKSize(size,size));
		renderer.RenderSolidRect(PixelToLocal(rc), DKMatrix3::identity, DKColor(1,1,0,0.7));
	}
}

void TestFrame1::OnKeyDown(int deviceId, DKVirtualKey lKey)
{
}

void TestFrame1::OnKeyUp(int deviceId, DKVirtualKey lKey)
{
}

void TestFrame1::OnTextInput(int deviceId, const DKFoundation::DKString& str)
{
}

void TestFrame1::OnTextInputCandidate(int deviceId, const DKFoundation::DKString& str)
{
}

void TestFrame1::OnButtonEvent(FrameButton::ButtonEvent e, DKObject<FrameButton> btn)
{
	if (e != FrameButton::ButtonEventActivated)
		return;

	if (btn == gyroCamera)
	{
		gyroCameraState = !gyroCameraState;
		if (gyroCameraState)
		{
			gyroCamera->SetText(L"gyro on");
			manualMove->RemoveFromSuperframe();
		}
		else
		{
			gyroCamera->SetText(L"gyro off");
			AddSubframe(manualMove);
		}
	}
	else if (btn == manualMove)
	{
		manualMoveState = !manualMoveState;
		if (manualMoveState)
			manualMove->SetText(L"camera move");
		else
			manualMove->SetText(L"light move");
	}
	else if (btn == skeletalView)
	{
		skeletalViewState = !skeletalViewState;
		if (skeletalViewState)
			skeletalView->SetText(L"mesh");
		else
			skeletalView->SetText(L"skeletal");
	}
	else if (btn == playAnimation)
	{
		playAnimationState = !playAnimationState;
		if (playAnimationState)
		{
			playAnimation->SetText(L"stop anim");
			animationController->Play();
		}
		else
		{
			playAnimation->SetText(L"play anim");
			animationController->Stop();
		}
	}
	else if (btn == animationToggle)
	{
		animationToggleState = !animationToggleState;
		if (animationToggleState)
		{
			animationToggle->SetText(L"ani2");
			animationController = animations.Value(L"attack")->CreateLoopController();
			animationController->Play();
			if (this->character)
				this->character->SetAnimation(this->animationController);
		}
		else
		{
			animationToggle->SetText(L"ani1");
			animationController = animations.Value(L"walk")->CreateLoopController();
			animationController->Play();
			if (this->character)
				this->character->SetAnimation(this->animationController);
		}
	}
	else if (btn == animationReverse)
	{
		animationReverseState = !animationReverseState;
		if (animationReverseState)
		{
			animationReverse->SetText(L"reverse");
			animationController->SetSpeed(-1.0);
		}
		else
		{
			animationReverse->SetText(L"normal");
			animationController->SetSpeed(1.0);
		}
	}
	else if (btn == prevCharacter)
	{
		if (charFiles.Count() > 0)
		{
			DKTimer timer;
			timer.Reset();

			if (this->character)
				this->character->RemoveFromScene();
			this->character = NULL;
			this->katana->RemoveFromParent();

			DKObject<DKResourcePool> pool = resourcePool.Clone();
			DKObject<DKModel> characterModel = pool->LoadResource(this->charFiles.Value(this->nextCharacterIndex)).SafeCast<DKModel>();
			this->loadingTime = timer.Elapsed();
			DKLog("Character Loading: %f\n", this->loadingTime);

			if (this->nextCharacterIndex > 0)
				this->nextCharacterIndex--;
			else
				this->nextCharacterIndex = this->charFiles.Count() - 1;

			this->character = characterModel;
			DKModel* weapon = this->character->FindDescendant(L"Weapon");
			if (weapon)
				weapon->AddChild(this->katana);
			this->character->SetAnimation(this->animationController);
			mainScene->AddObject(this->character);
		}
	}
	else if (btn == nextCharacter)
	{
		if (charFiles.Count() > 0)
		{
			DKTimer timer;
			timer.Reset();

			if (this->character)
				this->character->RemoveFromScene();
			this->character = NULL;
			this->katana->RemoveFromParent();

			DKObject<DKResourcePool> pool = resourcePool.Clone();
			DKObject<DKModel> characterModel = pool->LoadResource(this->charFiles.Value(this->nextCharacterIndex)).SafeCast<DKModel>();
			this->loadingTime = timer.Elapsed();
			DKLog("Character Loading: %f\n", this->loadingTime);

			this->nextCharacterIndex = (this->nextCharacterIndex + 1) % charFiles.Count();

			this->character = characterModel;
			DKModel* weapon = this->character->FindDescendant(L"Weapon");
			if (weapon)
				weapon->AddChild(this->katana);
			this->character->SetAnimation(this->animationController);
			mainScene->AddObject(this->character);
		}
	}
	else if (btn == debugTest)
	{
		// 파일 매핑 테스트.. (누를때마다 오프셋 바꾸면서 테스트함)
		DKObject<DKFile> tmpFile = DKFile::CreateTemporary();
		size_t maxSize = 1234;
		for (int i = 0; i < maxSize; i++)
		{
			unsigned char c = (i % 0xff);
			tmpFile->Write(&c, 1);
		}
		
		static size_t offset = 0;
		size_t len = maxSize - offset;
		DKObject<DKData> map1 = tmpFile->MapContentRange(offset, len);
		size_t errCount = 0;
		tmpFile = NULL;
		if (map1)
		{
			DKLog("MpaContentRange(%lu, %lu) created.\n", offset, len);
			unsigned char* p = (unsigned char*)map1->LockExclusive();
			for (int i = 0; i < len; i++)
			{
				unsigned char c = (offset + i) % 0xff;
				if (p[i] != c)
					errCount++;
			}
			map1->UnlockExclusive();
			map1 = NULL;
		}
		else
		{
			DKLog("MapContentRange(%lu, %lu) failed.\n", offset, len);
		}
		DKLog("Error: %lu\n", errCount);
		offset += 7;


		static bool debugCallStackByException = false;		
		DKLog("\n\n ** Debug Call Stack ** \n\n");
		if (debugCallStackByException)
		{
			try
			{
				DKERROR_THROW("*** Debug CallStack Generate by DKError::RaiseException ***");
			}
			catch (DKError& e)
			{
				if (!DKFoundation::IsDebuggerPresent())
					e.PrintDescriptionWithStackFrames();
			}
		}
		else
		{
			DKError err("*** Debug CallStack Generate by DKError::RetraceStackFrames ***");
			err.RetraceStackFrames();
			err.PrintDescriptionWithStackFrames();
		}
		debugCallStackByException = !debugCallStackByException;
	}
}
