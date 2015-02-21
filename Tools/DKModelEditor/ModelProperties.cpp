#include "StdAfx.h"
#include "ModelProperties.h"
#include "StreamDataModel.h"
#include "SamplerDataModel.h"
#include "MaterialDataModel.h"

void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
DKModel* GetModel(void);
void SetModelModified(void);
DKObject<DKMaterial> OpenMaterial(void);
void LockRenderer(void);
void UnlockRenderer(void);
void UpdateTreeItem(wxTreeItemId item);

enum class ModelTypes
{
	Container = 0,
	RigidBody,
	SoftBody,
	Static,
};

wxBEGIN_EVENT_TABLE(ModelProperties, wxWindow)
	// 윈도 메시지
	EVT_CONTEXT_MENU(							ModelProperties::OnContextMenu)
	EVT_SIZE(									ModelProperties::OnSize)
	EVT_CLOSE(									ModelProperties::OnClose)
	// 메뉴
	EVT_MENU(UICommandTest,						ModelProperties::OnTest)
	// 프로퍼티그리드 이벤트
	EVT_PG_CHANGING(UIControlPropertyGrid,		ModelProperties::OnPropertyChanging)
	EVT_PG_CHANGED(UIControlPropertyGrid,		ModelProperties::OnPropertyChanged)
	EVT_PG_LABEL_EDIT_BEGIN(UIControlPropertyGrid,	ModelProperties::OnPropertyLabelEditBegin)
	EVT_PG_LABEL_EDIT_ENDING(UIControlPropertyGrid,	ModelProperties::OnPropertyLabelEditEnding)

	EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY,	ModelProperties::OnDataViewItemStartEditing)
	EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY,	ModelProperties::OnDataViewItemEditingDone)
	EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY,	ModelProperties::OnDataViewItemValueChanged)
wxEND_EVENT_TABLE();

ModelProperties::ModelProperties(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
	, modelNode(NULL)
{
	auto initInfoPage = [this](InfoPage& page) -> bool
	{
		long notebookStyle = wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER;
		page.notebook = new wxAuiNotebook(this, wxID_ANY, GetClientAreaOrigin(), GetClientSize(), notebookStyle);

		long propertyGridStyle = wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE;
		page.infoView = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, propertyGridStyle);
		page.infoView->SetAutoLayout(true);

		page.rotationX = new wxFloatProperty("Pitch");
		page.rotationY = new wxFloatProperty("Yaw");
		page.rotationZ = new wxFloatProperty("Roll");
		page.positionX = new wxFloatProperty("PositionX");
		page.positionY = new wxFloatProperty("PositionY");
		page.positionZ = new wxFloatProperty("PositionZ");

		page.infoView->Bind(wxEVT_PG_CHANGING, &ModelProperties::OnPropertyChanging, this);
		page.infoView->Bind(wxEVT_PG_CHANGED, &ModelProperties::OnPropertyChanged, this);
		return true;
	};
	auto initModelPage = [&](ModelInfo& page) -> bool
	{
		if (initInfoPage(page))
		{
			static const wchar_t* typeLabels[] =
			{
				L"Container",
				L"RigidBody",
				L"SoftBody",
				L"Static",
				NULL
			};
			const long typeValues[] =
			{
				(long)ModelTypes::Container,
				(long)ModelTypes::RigidBody,
				(long)ModelTypes::SoftBody,
				(long)ModelTypes::Static
			};

			page.infoView->Append(new wxPropertyCategory("General"));
			page.type = new wxEnumProperty("Type", wxPG_LABEL, typeLabels, typeValues, typeValues[0]);
			page.infoView->Append(page.type);

			page.name = new wxStringProperty("Name", wxPG_LABEL, "");

			page.infoView->Append(page.name);

			page.infoView->Append(new wxPropertyCategory("Transform"));
			page.infoView->Append(page.rotationX);
			page.infoView->Append(page.rotationY);
			page.infoView->Append(page.rotationZ);
			page.infoView->Append(page.positionX);
			page.infoView->Append(page.positionY);
			page.infoView->Append(page.positionZ);

			page.notebook->AddPage(page.infoView, "Model");
			return true;
		}
		return false;
	};
	// modelInfo 초기화
	if (initModelPage(modelInfo))
	{
	}
	// rigidBodyInfo 초기화
	if (initModelPage(rigidBodyInfo))
	{
		rigidBodyInfo.collisionView = new wxWindow(this, wxID_ANY); 
		rigidBodyInfo.notebook->AddPage(rigidBodyInfo.collisionView, "Rigid Body"); 
	}
	// softBodyInfo 초기화
	if (initModelPage(softBodyInfo))
	{
		softBodyInfo.collisionView = new wxWindow(this, wxID_ANY); 
		softBodyInfo.notebook->AddPage(softBodyInfo.collisionView, "Soft Body"); 
	}
	// meshInfo 초기화
	if (initModelPage(meshInfo))
	{
		meshInfo.hidden = new wxBoolProperty("Hidden", wxPG_LABEL, true);
		meshInfo.primitive = new wxStringProperty("Primitive", wxPG_LABEL, "Triangles");
		meshInfo.cullMode = new wxStringProperty("Cull", wxPG_LABEL, "CW"); 
		meshInfo.material = new wxStringProperty("Material", wxPG_LABEL, "test");

		long propertyGridStyle = wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE;
		meshInfo.meshView = new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, propertyGridStyle);
		meshInfo.meshView->SetAutoLayout(true);
		meshInfo.meshView->Append(meshInfo.hidden);
		meshInfo.meshView->Append(meshInfo.primitive);
		meshInfo.meshView->Append(meshInfo.cullMode);
		meshInfo.meshView->Append(meshInfo.material);

		meshInfo.streamView = new wxDataViewCtrl(this, wxID_ANY);
		if (true)
		{
			wxObjectDataPtr<StreamDataModel> dataModel(new StreamDataModel);
			meshInfo.streamView->AssociateModel(dataModel.get());
			for (size_t i = 0; i < dataModel->dataViewColumns.Count(); ++i)
				meshInfo.streamView->AppendColumn(dataModel->dataViewColumns.Value(i));
		}

		meshInfo.samplerView = new wxDataViewCtrl(this, wxID_ANY);
		if (true)
		{
			wxObjectDataPtr<SamplerDataModel> dataModel(new SamplerDataModel);
			meshInfo.samplerView->AssociateModel(dataModel.get());
			for (size_t i = 0; i < dataModel->dataViewColumns.Count(); ++i)
				meshInfo.samplerView->AppendColumn(dataModel->dataViewColumns.Value(i));
		}

		meshInfo.materialView = new wxDataViewCtrl(this, wxID_ANY);
		if (true)
		{
			wxObjectDataPtr<MaterialDataModel> dataModel(new MaterialDataModel);
			meshInfo.materialView->AssociateModel(dataModel.get());
			for (size_t i = 0; i < dataModel->dataViewColumns.Count(); ++i)
				meshInfo.materialView->AppendColumn(dataModel->dataViewColumns.Value(i));
		}

		meshInfo.notebook->AddPage(meshInfo.meshView, "Mesh");
		meshInfo.notebook->AddPage(meshInfo.streamView, "Stream");
		meshInfo.notebook->AddPage(meshInfo.samplerView, "Sampler");
		meshInfo.notebook->AddPage(meshInfo.materialView, "Material");
	}

	// shapeInfo 초기화
	if (initInfoPage(shapeInfo))
	{
		static const wchar_t* typeLabels[] =
		{
			L"box",
			L"capsule",
			L"cylinder",
			L"cone",
			L"sphere",
			L"multi sphere",
			L"convex hull",
			L"static plane",
			L"static triangles",
			NULL
		};
		static const long typeValues[] =
		{
			(long)DKCollisionShape::ShapeType::Box,
			(long)DKCollisionShape::ShapeType::Capsule,
			(long)DKCollisionShape::ShapeType::Cylinder,
			(long)DKCollisionShape::ShapeType::Cone,
			(long)DKCollisionShape::ShapeType::Sphere,
			(long)DKCollisionShape::ShapeType::MultiSphere,
			(long)DKCollisionShape::ShapeType::ConvexHull,
			(long)DKCollisionShape::ShapeType::StaticPlane,
			(long)DKCollisionShape::ShapeType::StaticTriangleMesh
		};
		wxASSERT( (sizeof(typeLabels) / sizeof(typeLabels[0])) == (sizeof(typeValues) / sizeof(typeValues[0]))+1 );

		shapeInfo.type = new wxEnumProperty("Type", wxPG_LABEL, typeLabels, typeValues, typeValues[0]);
		shapeInfo.infoView->Append(shapeInfo.type);

		shapeInfo.infoView->Append(new wxPropertyCategory("Transform"));
		shapeInfo.infoView->Append(shapeInfo.rotationX);
		shapeInfo.infoView->Append(shapeInfo.rotationY);
		shapeInfo.infoView->Append(shapeInfo.rotationZ);
		shapeInfo.infoView->Append(shapeInfo.positionX);
		shapeInfo.infoView->Append(shapeInfo.positionY);
		shapeInfo.infoView->Append(shapeInfo.positionZ);

		shapeInfo.notebook->AddPage(shapeInfo.infoView, "Shape");
	}

	// constraintInfo 초기화
	if (initInfoPage(constraintInfo))
	{
		static const wchar_t* typeLabels[] =
		{
			L"fixed",
			L"p2p",
			L"hinge",
			L"cone twist",
			L"g6dof",
			L"g6dof spring",
			L"slider",
			NULL
		};
		static const long typeValues[] =
		{
			(long)DKConstraint::LinkType::Fixed,
			(long)DKConstraint::LinkType::Point2Point,
			(long)DKConstraint::LinkType::Hinge,
			(long)DKConstraint::LinkType::ConeTwist,
			(long)DKConstraint::LinkType::Generic6Dof,
			(long)DKConstraint::LinkType::Generic6DofSpring,
			(long)DKConstraint::LinkType::Slider,
		};
		wxASSERT( (sizeof(typeLabels) / sizeof(typeLabels[0])) == (sizeof(typeValues) / sizeof(typeValues[0]))+1 );

		constraintInfo.type = new wxEnumProperty("Type", wxPG_LABEL, typeLabels, typeValues, typeValues[0]);
		constraintInfo.infoView->Append(constraintInfo.type);

		constraintInfo.infoView->Append(new wxPropertyCategory("Transform"));
		constraintInfo.infoView->Append(constraintInfo.rotationX);
		constraintInfo.infoView->Append(constraintInfo.rotationY);
		constraintInfo.infoView->Append(constraintInfo.rotationZ);
		constraintInfo.infoView->Append(constraintInfo.positionX);
		constraintInfo.infoView->Append(constraintInfo.positionY);
		constraintInfo.infoView->Append(constraintInfo.positionZ);

		constraintInfo.notebook->AddPage(constraintInfo.infoView, "Constraint");
	}

	modelInfo.notebook->Show(false);
	rigidBodyInfo.notebook->Show(false);
	softBodyInfo.notebook->Show(false);
	meshInfo.notebook->Show(false);
	shapeInfo.notebook->Show(false);
	constraintInfo.notebook->Show(false);
}

ModelProperties::~ModelProperties(void)
{
}

void ModelProperties::OnContextMenu(wxContextMenuEvent& e)
{
	wxPoint pt = e.GetPosition();

	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
	DKLog("OnContextMenu at screen coords (%i, %i)\n", pt.x, pt.y);
}

void ModelProperties::OnSize(wxSizeEvent& e)
{
	wxRect rc = GetClientRect();
	modelInfo.notebook->SetSize(rc); 
	rigidBodyInfo.notebook->SetSize(rc);
	softBodyInfo.notebook->SetSize(rc);
	meshInfo.notebook->SetSize(rc);
	shapeInfo.notebook->SetSize(rc);
	constraintInfo.notebook->SetSize(rc);
}

void ModelProperties::OnClose(wxCloseEvent& e)
{
	static_cast<StreamDataModel*>(meshInfo.streamView->GetModel())->RemoveAll();
	static_cast<SamplerDataModel*>(meshInfo.samplerView->GetModel())->RemoveAll();
	static_cast<MaterialDataModel*>(meshInfo.materialView->GetModel())->RemoveAll();	
}

void ModelProperties::OnTest(wxCommandEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
}

void ModelProperties::SetActiveNode(ModelNodeItemData* item)
{
	modelInfo.notebook->Show(false);
	rigidBodyInfo.notebook->Show(false);
	softBodyInfo.notebook->Show(false);
	meshInfo.notebook->Show(false);
	shapeInfo.notebook->Show(false);
	constraintInfo.notebook->Show(false);

	static_cast<StreamDataModel*>(meshInfo.streamView->GetModel())->RemoveAll();
	static_cast<SamplerDataModel*>(meshInfo.samplerView->GetModel())->RemoveAll();
	static_cast<MaterialDataModel*>(meshInfo.materialView->GetModel())->RemoveAll();
	
	if (item)
	{
		modelNode = item;

		InfoPage* currentPage = NULL;

		switch (item->type)
		{
		case ModelNodeItemData::TypeModel:
			{
				ModelNodeModelData* modelNode = static_cast<ModelNodeModelData*>(item);

				wxASSERT(modelNode->model != NULL);
				modelInfo.name->SetValueFromString(item->GetName());

				currentPage = &modelInfo;
			}
			break;
		case ModelNodeItemData::TypeRigidBody:
			{
				ModelNodeModelData* modelNode = static_cast<ModelNodeModelData*>(item);

				wxASSERT(modelNode->model != NULL);
				rigidBodyInfo.name->SetValueFromString(item->GetName());

				currentPage = &rigidBodyInfo;
			}
			break;
		case ModelNodeItemData::TypeSoftBody:
			{
				ModelNodeModelData* modelNode = static_cast<ModelNodeModelData*>(item);

				wxASSERT(modelNode->model != NULL);
				softBodyInfo.name->SetValueFromString(item->GetName());

				currentPage = &softBodyInfo;
			}
			break;
		case ModelNodeItemData::TypeMesh:
			{
				ModelNodeMeshData* meshNode = static_cast<ModelNodeMeshData*>(item);

				wxASSERT(meshNode->mesh != NULL);

				meshInfo.name->SetValueFromString(item->GetName());
				meshInfo.hidden->SetValueFromInt(meshNode->mesh->IsHidden());
				DKMaterial* material = meshNode->mesh->Material();
				if (material)
					meshInfo.material->SetValueFromString((const wchar_t*)material->Name());
				else
					meshInfo.material->SetValueFromString("");

				// 샘플러 추출
				SamplerDataModel* samplerModel = (SamplerDataModel*)meshInfo.samplerView->GetModel();
				meshNode->mesh->SamplerMap().EnumerateForward([samplerModel](const DKMesh::TextureSamplerMap::Pair& p)
				{
					for (size_t i = 0; i < p.value.textures.Count(); ++i)
					{
						SamplerDataModel::Sampler sampler = {p.key, i, p.value.textures.Value(i)};
						samplerModel->AddSampler(sampler);
					}
				});

				currentPage = &meshInfo;
			}
			break;
		case ModelNodeItemData::TypeShape:
			{
				ModelNodeShapeData* shapeNode = static_cast<ModelNodeShapeData*>(item);

				shapeInfo.type->SetValueFromInt((long)shapeNode->shape->type, wxPG_FULL_VALUE);

				currentPage = &shapeInfo;
			}
			break;
		case ModelNodeItemData::TypeConstraint:
			{
				ModelNodeConstraintData* consNode = static_cast<ModelNodeConstraintData*>(item);

				constraintInfo.type->SetValueFromInt((long)consNode->constraint->type, wxPG_FULL_VALUE);

				currentPage = &constraintInfo;
			}
			break;
		default:
			PrintLog(wxString::Format("[%s] Unknown Type:0x%x.\n", DKLIB_FUNCTION_NAME, item->type), wxColour(255,0,0));
			modelNode = NULL;
			break;
		}

		if (currentPage)
		{
			DKNSTransform trans = item->Transform();

			float pitch = trans.orientation.Pitch();
			float yaw = trans.orientation.Yaw();
			float roll = trans.orientation.Roll();	

			currentPage->rotationX->SetValueFromString((const wchar_t*)DKString::Format("%.1f", (double)DKL_RADIAN_TO_DEGREE(pitch)));
			currentPage->rotationY->SetValueFromString((const wchar_t*)DKString::Format("%.1f", (double)DKL_RADIAN_TO_DEGREE(yaw)));
			currentPage->rotationZ->SetValueFromString((const wchar_t*)DKString::Format("%.1f", (double)DKL_RADIAN_TO_DEGREE(roll)));																							   
			currentPage->positionX->SetValueFromString((const wchar_t*)DKString::Format("%.4f", trans.position.x));
			currentPage->positionY->SetValueFromString((const wchar_t*)DKString::Format("%.4f", trans.position.y));
			currentPage->positionZ->SetValueFromString((const wchar_t*)DKString::Format("%.4f", trans.position.z));

			currentPage->notebook->Show(true);
		}
	}
	else
	{
		PrintLog(wxString::Format("[%s] cleared.\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
		modelNode = NULL;
	}
}

void ModelProperties::OnPropertyChanging(wxPropertyGridEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
	// 이름 변경 확인
	wxPGProperty* prop = e.GetProperty();
	wxString label = prop->GetLabel();

	if (label.CmpNoCase("Name") == 0)
	{
		wxString oldName = prop->GetValue();
		wxString newName = e.GetValue();

		if (newName == modelNode->GetName())
		{
			e.Veto();
			return;
		}
	}
}

void ModelProperties::OnPropertyChanged(wxPropertyGridEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));

	// 이름 변경 확인
	wxPGProperty* prop = e.GetProperty();
	wxString label = prop->GetLabel();

	if (label.CmpNoCase("Name") == 0)
	{
		wxString value = prop->GetValue();
		modelNode->SetName(value);

		UpdateTreeItem(modelNode->GetId());
		SetModelModified();
	}
	else
	{
		DKLog("Property(%ls) value changed but not implemented yet.\n", (const wchar_t*)label);
	}
}

void ModelProperties::OnPropertyLabelEditBegin(wxPropertyGridEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void ModelProperties::OnPropertyLabelEditEnding(wxPropertyGridEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void ModelProperties::OnDataViewItemStartEditing(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void ModelProperties::OnDataViewItemEditingDone(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void ModelProperties::OnDataViewItemValueChanged(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}
