#include "stdafx.h"
#include <math.h>
#include "TestFrame1.h"

extern DKString g_resDir;

struct SimpleButton : public DKFrame
{
	bool pressed;
	SimpleButton(void) : pressed(false) {}

	void OnRender(DKRenderer& renderer) const
	{
		renderer.Clear(DKColor(1,1,1));
	}
	void OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
	{
		if (deviceId == 0)
		{
			this->CaptureMouse(deviceId);
			this->pressed = true;
		}
	}
	void OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
	{
		if (deviceId == 0)
		{
			this->ReleaseMouse(deviceId);
			this->pressed = false;
		}
	}
};


TestFrame1::TestFrame1(void)
{
}

TestFrame1::~TestFrame1(void)
{
}

void TestFrame1::OnLoaded(void)
{
	DKResourcePool	pool;
	pool.AddSearchPath(g_resDir);
	pool.AddSearchPath(g_resDir + "/Fonts");
	pool.AddSearchPath(g_resDir + "/dao_bazzi");
	pool.AddSearchPath(DKApplication::Instance()->EnvironmentPath(DKApplication::SystemPathAppResource));


	// 폰트 로딩
	fontRegular = DKFont::Create(pool.LoadResourceData(L"NanumGothic.ttf"));
	fontRegular->SetStyle(18);

	// 카메라 설정
	DKPoint pt = LocalToPixel(DKPoint(1,1));
	float fAspectRatio = pt.x / pt.y;

	cameraTarget = DKVector3(0, 0, 0);
	DKVector3 camPosition = DKVector3(0, cameraTarget.y + 20, -80);
	camera.SetView(camPosition, cameraTarget - camPosition, DKVector3(0,1,0));
	camera.SetPerspective(DKL_DEGREE_TO_RADIAN(85), fAspectRatio, 1, 1000);

	scene = DKOBJECT_NEW DKDynamicsScene();
	scene->lights.Add( DKLight(DKVector3(-1,-1,1), DKColor(1,1,1)) );
	scene->ambientColor = DKColor(0.25,0.25,0.25,1.0);
	scene->drawMode = DKScene::DrawCollisionShapes | DKScene::DrawConstraints | DKScene::DrawConstraintLimits | DKScene::DrawCollisionAABB;

	// 바닥 설정
	if (1)
	{
		DKObject<DKCollisionShape> plane = DKOBJECT_NEW DKStaticPlaneShape(DKVector3(0,1,0), 0);
		DKObject<DKModel> planeModel = DKOBJECT_NEW DKRigidBody(plane, 0.0);
		planeModel->SetName(L"Ground");
		scene->AddObject(planeModel);
	}
	if (1)
	{
		float mass = 10.0;
		DKObject<DKCollisionShape> sphere = DKOBJECT_NEW DKSphereShape(3);
		DKObject<DKCollisionShape> cylin = DKOBJECT_NEW DKCylinderShape(3,3,3);
		DKObject<DKCollisionShape> cone = DKOBJECT_NEW DKConeShape(3, 4);
		DKObject<DKCollisionShape> box = DKOBJECT_NEW DKBoxShape(4,4,4);
		DKObject<DKCollisionShape> door = DKOBJECT_NEW DKBoxShape(6, 0.2, 7);

		DKObject<DKCompoundShape> comp = DKOBJECT_NEW DKCompoundShape();
		comp->AddShape(box, DKNSTransform(DKQuaternion(DKVector3(0,0,1), DKL_PI / 9), DKVector3(10,1,1)));

		DKObject<DKRigidBody> model1 = DKOBJECT_NEW DKRigidBody(box, mass);
		model1->SetName(L"Model1");
		model1->SetLocalTransform(DKVector3(0,0,0));

		DKObject<DKRigidBody> model2 = DKOBJECT_NEW DKRigidBody(cone, mass);
		model2->SetName(L"Model2");
		model2->SetLocalTransform(DKVector3(0, 8, 0));

		DKObject<DKRigidBody> model3 = DKOBJECT_NEW DKRigidBody(door, mass);
		model3->SetName(L"Model3");
		model3->SetLocalTransform(DKVector3(12, 0, 0));

		DKObject<DKFixedConstraint> fixed = DKOBJECT_NEW DKFixedConstraint(model1, model2, DKVector3(0,4,0), DKVector3(0,-4,0));
		DKObject<DKHingeConstraint> hinge = DKOBJECT_NEW DKHingeConstraint(model1, model3, DKVector3(6,0,0), DKVector3(-6,0,0));
		DKObject<DKPoint2PointConstraint> p2p = DKOBJECT_NEW DKPoint2PointConstraint(model1, model3, DKVector3(6,0,0), DKVector3(-6,0,0));

		fixed->SetName(L"fixed");
		fixed->SetBreakingImpulseThreshold(1800);
		fixed->disableCollisionsBetweenLinkedBodies = false;
		hinge->SetName(L"hinge");
		hinge->SetBreakingImpulseThreshold(1000);
		hinge->disableCollisionsBetweenLinkedBodies = false;
		p2p->SetName(L"p2p");
		p2p->SetBreakingImpulseThreshold(1500);
		p2p->disableCollisionsBetweenLinkedBodies = false;

		DKObject<DKModel> model = DKObject<DKModel>::New();
		model->SetWorldTransform(DKVector3(0, 50, 0));
		model->AddChild(model1);
		model->AddChild(model2);
		model->AddChild(model3);
		model->AddChild(fixed);
		model->AddChild(hinge);
		model->AddChild(p2p);
		scene->AddObject(model);
	}
	if (1)
	{
		float mass = 50;
		float radius1 = 8;
		float radius2 = 6;
		float circumference1 = radius1 * 2 * DKL_PI;		// 둘레1
		float circumference2 = radius2 * 2 * DKL_PI;
		float gearRatio = circumference2 / circumference1;

		DKObject<DKCollisionShape> axis = DKOBJECT_NEW DKCylinderShape(1, 1, 1.2, DKCollisionShape::UpAxis::Forward);
		DKObject<DKCollisionShape> c1 = DKOBJECT_NEW DKCylinderShape(radius1, radius1, 1, DKCollisionShape::UpAxis::Forward);
		DKObject<DKCollisionShape> c2 = DKOBJECT_NEW DKCylinderShape(radius2, radius2, 1, DKCollisionShape::UpAxis::Forward);

		DKObject<DKCompoundShape> gear1 = DKOBJECT_NEW DKCompoundShape();
		DKObject<DKCompoundShape> gear2 = DKOBJECT_NEW DKCompoundShape();
		gear1->AddShape(axis, DKNSTransform::identity);
		gear1->AddShape(c1, DKNSTransform::identity);

		gear2->AddShape(axis, DKNSTransform::identity);
		gear2->AddShape(c2, DKNSTransform::identity);

		DKObject<DKRigidBody> gearBody1 = DKOBJECT_NEW DKRigidBody(gear1, mass);
		DKObject<DKRigidBody> gearBody2 = DKOBJECT_NEW DKRigidBody(gear2, mass);
		gearBody1->SetName(L"Gear1");
		gearBody2->SetName(L"Gear2");

		DKObject<DKCollisionShape> box = DKOBJECT_NEW DKBoxShape(12, 2, 6);
		DKObject<DKRigidBody> gearBox = DKOBJECT_NEW DKRigidBody(box, 100);
		gearBox->SetName(L"Gear Box");

		DKVector3 offset1 = DKVector3(8.2, 10, 0);
		DKVector3 offset2 = DKVector3(-6.2, 10, 0);
		DKObject<DKHingeConstraint> hinge1 = DKOBJECT_NEW DKHingeConstraint(gearBox, gearBody1, offset1, DKNSTransform::identity);
		DKObject<DKHingeConstraint> hinge2 = DKOBJECT_NEW DKHingeConstraint(gearBox, gearBody2, offset2, DKNSTransform::identity);
		hinge1->disableCollisionsBetweenLinkedBodies = true;
		hinge2->disableCollisionsBetweenLinkedBodies = true;

		DKObject<DKGearConstraint> gearCon = DKOBJECT_NEW DKGearConstraint(gearBody1, gearBody2, DKVector3(0, 0, 1), DKVector3(0, 0, 1), gearRatio);
		gearCon->disableCollisionsBetweenLinkedBodies = true;

		gearBox->AddChild(gearBody1);
		gearBox->AddChild(gearBody2);
		gearBox->SetLocalTransform(DKVector3(0, 20, -20));
		gearBody1->SetLocalTransform(offset1);
		gearBody2->SetLocalTransform(offset2);

		DKObject<DKModel> model = DKObject<DKModel>::New();
		model->AddChild(gearBox);
		model->AddChild(hinge1);
		model->AddChild(hinge2);
		model->AddChild(gearCon);

		scene->AddObject(model);
	}
	if (1)
	{
		DKObject<DKModel> dao_bazzi = pool.LoadResource(L"dao_bazzi.DKMODEL").SafeCast<DKModel>();
		scene->AddObject(dao_bazzi);

		DKObject<DKAnimation> anim = pool.LoadResource(L"dao_bazzi.DKANIMATION").SafeCast<DKAnimation>();
		DKObject<DKAnimationController> animCon = anim->CreateLoopController();
		dao_bazzi->SetAnimation(animCon);
		animCon->Play();
	}
	pickedObject = NULL;
	pickConstraints = NULL;

	this->displayButton = DKOBJECT_NEW SimpleButton();
	this->displayButton->SetTransform(DKAffineTransform2(DKLinearTransform2().Scale(0.1,0.1)).Translate(0.9, 0.0).Matrix3());
	this->AddSubframe(this->displayButton);
}

void TestFrame1::OnUnload(void)
{
	this->fontRegular = NULL;
	this->displayButton = NULL;
	this->scene = NULL;

	DKApplication::Instance()->Terminate(0);
}

void TestFrame1::OnContentResized(void)
{
	DKSize size = ContentResolution();
	float fAspectRatio = size.width / size.height;
	camera.SetPerspective(DKL_DEGREE_TO_RADIAN(85), fAspectRatio, 1, 1000);
}

void TestFrame1::RemovePickingConstraint(void)
{
	if (pickedObject && pickConstraints)
	{
		pickedObject->KeepActivating(false);

		pickConstraints->RootObject()->RemoveFromScene();
		pickedObject = NULL;
		pickConstraints = NULL;
	}
}

void TestFrame1::OnMouseDown(int deviceId, int buttonId, const DKPoint& pos)
{
	int id = Max(deviceId, buttonId);

	//if (mousePositions.Find(id) != NULL)
	//	DKLog("[%s] ERROR: deviceId: %d ALREADY EXIST!\n", DKLIB_FUNCTION_NAME, id);

	CaptureMouse(deviceId);	
	mousePositions.Value(id) = pos;

	if (id == 0)
	{
		Screen()->Window()->ShowMouse(deviceId, false);
		Screen()->Window()->HoldMouse(deviceId, true);
	}
	else if (id == 1)
	{
		RemovePickingConstraint();

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
		DKCollisionObject* obj = this->scene->RayTestClosest(rayBeginOnWorld, rayEndOnWorld, &hitPoint);
		if (obj)
		{
			if (obj->IsStatic())
			{
				DKLog("rayTest: static object! (%ls)\n", (const wchar_t*)obj->Name());
			}
			else if (obj->IsKinematic())
			{
				DKLog("rayTest: kinematic object! (%ls)\n", (const wchar_t*)obj->Name());
			}
			else
			{
				DKLog("rayTest: dynamic object! (%ls) \n", (const wchar_t*)obj->Name());
				if (obj->objectType == DKCollisionObject::RigidBody)
				{
					DKRigidBody* rigidBody = static_cast<DKRigidBody*>(obj);
					DKNSTransform tr = DKNSTransform().Identity();
					tr.position = hitPoint * DKNSTransform(obj->WorldTransform()).Inverse();
 
					DKObject<DKGeneric6DofConstraint> g6dof = DKOBJECT_NEW DKGeneric6DofConstraint(rigidBody, tr);
					g6dof->SetLimit(DKConstraint::ParamAxis::LinearX, 0, 0);
					g6dof->SetLimit(DKConstraint::ParamAxis::LinearY, 0, 0);
					g6dof->SetLimit(DKConstraint::ParamAxis::LinearZ, 0, 0);
					g6dof->SetLimit(DKConstraint::ParamAxis::AngularX, 0, 0);
					g6dof->SetLimit(DKConstraint::ParamAxis::AngularY, 0, 0);
					g6dof->SetLimit(DKConstraint::ParamAxis::AngularZ, 0, 0);

					g6dof->SetParam(DKConstraint::ParamType::STOP_CFM, DKConstraint::ParamAxis::LinearX, 0.8);
					g6dof->SetParam(DKConstraint::ParamType::STOP_CFM, DKConstraint::ParamAxis::LinearY, 0.8);
					g6dof->SetParam(DKConstraint::ParamType::STOP_CFM, DKConstraint::ParamAxis::LinearZ, 0.8);
					g6dof->SetParam(DKConstraint::ParamType::STOP_CFM, DKConstraint::ParamAxis::AngularX, 0.8);
					g6dof->SetParam(DKConstraint::ParamType::STOP_CFM, DKConstraint::ParamAxis::AngularY, 0.8);
					g6dof->SetParam(DKConstraint::ParamType::STOP_CFM, DKConstraint::ParamAxis::AngularZ, 0.8);

					g6dof->SetParam(DKConstraint::ParamType::STOP_ERP, DKConstraint::ParamAxis::LinearX, 0.1);
					g6dof->SetParam(DKConstraint::ParamType::STOP_ERP, DKConstraint::ParamAxis::LinearY, 0.1);
					g6dof->SetParam(DKConstraint::ParamType::STOP_ERP, DKConstraint::ParamAxis::LinearZ, 0.1);
					g6dof->SetParam(DKConstraint::ParamType::STOP_ERP, DKConstraint::ParamAxis::AngularX, 0.1);
					g6dof->SetParam(DKConstraint::ParamType::STOP_ERP, DKConstraint::ParamAxis::AngularY, 0.1);
					g6dof->SetParam(DKConstraint::ParamType::STOP_ERP, DKConstraint::ParamAxis::AngularZ, 0.1);

					g6dof->SetFrameA(hitPoint);
					g6dof->SetName(L"g6dof-picker");

					this->pickedObject = obj;
					this->pickConstraints = g6dof;
					this->pickedObject->KeepActivating(true);
					this->pickDistance = (hitPoint - camPos).Length();

					this->scene->AddObject(g6dof);
				}
				else
				{
					DKLog("SceneObject(%ls) has no collision object!\n", (const wchar_t*)obj->Name());
				}
			}
		}
	}

	//if (deviceId == 0)
	//{
	//	DKVector3 cameraDir = camera.GetDirection();
	//	light.SetDirection(cameraDir);
	//}

	//DKLog("[%s] mouse down: %d pos:(%.3f, %.3f)\n", DKLIB_FUNCTION_NAME, id, pos.x, pos.y);
}

void TestFrame1::OnMouseUp(int deviceId, int buttonId, const DKPoint& pos)
{
	int id = Max(deviceId, buttonId);

	//if (mousePositions.Find(id) == NULL)
	//	DKLog("[%s] ERROR: deviceId: %d NOT EXIST!\n", DKLIB_FUNCTION_NAME, id);

	ReleaseMouse(deviceId);
	mousePositions.Remove(id);

	if (id == 0)
	{
		Screen()->Window()->ShowMouse(deviceId, true);
		Screen()->Window()->HoldMouse(deviceId, false);
	}
	else if (id == 1)
	{
		RemovePickingConstraint();
	}

	//DKLog("[%s] mouse up: %d pos:(%.3f, %.3f)\n", DKLIB_FUNCTION_NAME, id, pos.x, pos.y);
}

void TestFrame1::OnMouseMove(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	if (deviceId == 0 && mousePositions.Find(0))
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

		DKVector3 pos = camera.ViewPosition();
		DKVector3 vec = pos - cameraTarget;				
		pos = vec.Rotate(rot) + cameraTarget;
		up = up.Rotate(rot);

		camera.SetView(pos, cameraTarget - pos, up);
	}
	else if (pickedObject && pickConstraints)
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

		DKVector3 rayDir = (rayEndOnWorld - rayBeginOnWorld).Normalize();

		DKNSTransform pivot = pickConstraints->FrameA();
		pivot.position = rayBeginOnWorld + (rayDir * pickDistance);
		pickConstraints->SetFrameA(pivot);
	}
}

void TestFrame1::OnMouseWheel(int deviceId, const DKPoint& pos, const DKVector2& delta)
{
	const float minDistance = 10;
	const float maxDistance = 200;

	DKVector3 cameraPosition = camera.ViewPosition();
	float cameraDistance = (cameraPosition - cameraTarget).Length();

	float d = delta.y > 0 ? -5 : +5;

	float distance = Clamp<float>(cameraDistance + d, minDistance, maxDistance);

	if (distance != cameraDistance)
	{
		DKVector3 up = camera.ViewUp();
		DKVector3 p = cameraPosition - cameraTarget;
		p.Normalize();
		p *= distance;
		p += cameraTarget;

		camera.SetView(p, cameraTarget - p, up);
	}
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

	this->scene->Update(timeDelta, tick);

	bool drawMeshes = true;

	if (this->displayButton.StaticCast<SimpleButton>()->pressed)
		drawMeshes = false;
	else if (Screen()->Window()->KeyState(0, DKVK_LEFT_SHIFT))
		drawMeshes = false;

	if (drawMeshes)
	{
		scene->drawMode = DKScene::DrawMeshes;
	}
	else
	{
		scene->drawMode = DKScene::DrawCollisionShapes | DKScene::DrawConstraints | DKScene::DrawConstraintLimits | DKScene::DrawCollisionAABB | DKScene::DrawSkeletalLines;
	}

	SetRedraw();
}

void TestFrame1::OnRender(DKRenderer& renderer) const
{
	DKRenderState&	renderState = DKOpenGLContext::RenderState();

	renderer.Clear(DKColor(0.75,0.75,0.75));
	renderer.SetPolygonOffset(1, 1);

//	renderer.RenderSolidSphere(DKVector3(0,0,0), 20, 8, 8, camera.ViewProjectionMatrix(), DKColor(1,1,1,1));
//	renderer.RenderWireframeSphere(DKVector3(0,0,0), 20, 8, 8, camera.ViewProjectionMatrix(), DKColor(1,0,0,1));

//	renderer.RenderSolidAABB(DKVector3(0,0,0), DKVector3(20,20,20), camera.ViewProjectionMatrix(), DKColor(1,0,0,1));
//	renderer.RenderWireframeAABB(DKVector3(0,0,0), DKVector3(20,20,20), camera.ViewProjectionMatrix(), DKColor(1,1,1,1));

	bool useRenderer = false;

	if (useRenderer)
	{
		renderer.RenderScene(scene, camera, 0);
	}
	else
	{
		//int drawModes = DKScene::DrawCollisionShapes | DKScene::DrawConstraints | DKScene::DrawConstraintLimits | DKScene::DrawAABB;
		//dynScene->Render(renderer, camera, 0, drawModes);
		renderer.RenderScene(scene, camera, 0);
	}

	DKApplication* app = DKApplication::Instance();

	static DKArray<DKString> drawText;
	drawText.Clear();

	DKVector3 camPos = camera.ViewPosition();
	float cameraDistance = (camPos - cameraTarget).Length();
	// 글자 찍기	
	drawText.Add(DKString::Format("Elapsed: %.6f (%.2f fps), Tick:%u", timeDelta, 1 / timeDelta, updateTick));
	drawText.Add(DKString::Format("CamDistance: %.3f", cameraDistance));
	drawText.Add(L"");

	float baseline = fontRegular->Baseline();
	for (int i = 0; i < drawText.Count(); i++)
	{
		DKPoint lineBegin(0, fontRegular->LineHeight() * i + baseline);
		DKPoint lineEnd(fontRegular->LineWidth(drawText.Value(i)), lineBegin.y);
		renderer.RenderText(PixelToLocal(lineBegin), PixelToLocal(lineEnd), drawText.Value(i), fontRegular, DKColor(1,1,1,1));
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
	DKLog("%s text=\"%ls\"\n", DKLIB_FUNCTION_NAME, (const wchar_t*)str);
}

void TestFrame1::OnTextInputCandidate(int deviceId, const DKFoundation::DKString& str)
{
	DKLog("%s text=\"%ls\"\n", DKLIB_FUNCTION_NAME, (const wchar_t*)str);
}

