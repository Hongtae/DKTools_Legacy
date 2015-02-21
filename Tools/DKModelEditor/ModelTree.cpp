#include "StdAfx.h"
#include <wx/artprov.h>
#include "ModelTree.h"
#include "ModelNode.h"


void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
DKModel* GetModel(void);
void SetModelModified(void);
DKObject<DKMaterial> OpenMaterial(void);
DKObject<DKTexture> OpenTexture(void);
void LockRenderer(void);
void UnlockRenderer(void);
void SetActiveItem(ModelNodeItemData* item);

////////////////////////////////////////////////////////////////////////////////
// 아이템 정렬을 위해 트리 상속
class ModelNodeTreeCtrl : public wxTreeCtrl
{
	wxDECLARE_DYNAMIC_CLASS(ModelNodeTreeCtrl);
public:
	ModelNodeTreeCtrl(void) {}
	ModelNodeTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS, const wxValidator& validator = wxDefaultValidator, const wxString& name = "customTreeCtrl")
		: wxTreeCtrl(parent, id, pos, size, style, validator, name) {}
protected:
	int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
	{
		ModelNodeItemData* data1 = reinterpret_cast<ModelNodeItemData*>(GetItemData(item1));
		ModelNodeItemData* data2 = reinterpret_cast<ModelNodeItemData*>(GetItemData(item2));

		if (data1->type == data2->type)
		{
			DKString name1 = data1->GetName();
			DKString name2 = data2->GetName();
			return name1.CompareNoCase(name2);
		}
		return (int)data1->type - (int)data2->type;
	}
};


wxIMPLEMENT_DYNAMIC_CLASS(ModelNodeTreeCtrl, wxTreeCtrl);

////////////////////////////////////////////////////////////////////////////////
wxBEGIN_EVENT_TABLE(ModelTree, wxWindow)
	// 윈도 메시지
	EVT_CONTEXT_MENU(								ModelTree::OnContextMenu)
	EVT_SIZE(										ModelTree::OnSize)
	EVT_CLOSE(										ModelTree::OnClose)
	// 메뉴
	EVT_MENU(UICommandRename,						ModelTree::OnRename)
	EVT_MENU(UICommandCreateCollision,				ModelTree::OnCreateCollision)
	EVT_MENU(UICommandCreateShapeHull,				ModelTree::OnCreateShapeHull)
	EVT_MENU(UICommandSetMaterial,					ModelTree::OnSetMaterial)
	EVT_MENU(UICommandSetDiffuseMap,				ModelTree::OnSetDiffuseMap)
	EVT_MENU(UICommandUpdateBoundingInfo,			ModelTree::OnUpdateBoundingInfo)

	EVT_MENU(UICommandTest,							ModelTree::OnTest)

	// 트리 콘트롤
	EVT_TREE_ITEM_MENU(UIControlNodeTree,			ModelTree::OnItemMenu)
	EVT_TREE_BEGIN_LABEL_EDIT(UIControlNodeTree,	ModelTree::OnBeginLabelEdit)
	EVT_TREE_END_LABEL_EDIT(UIControlNodeTree,		ModelTree::OnEndLabelEdit)
	EVT_TREE_ITEM_ACTIVATED(UIControlNodeTree,		ModelTree::OnItemActivated)
	EVT_TREE_SEL_CHANGED(UIControlNodeTree,			ModelTree::OnItemSelected)
wxEND_EVENT_TABLE();

ModelTree::ModelTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
{
	wxImageList* imglist = new wxImageList(16, 16, true, ModelNodeItemData::TypeMax);
	imglist->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16,16)));			// TypeModel
	imglist->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16,16)));			// TypeRigidBody
	imglist->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16,16)));			// TypeSoftBody
	imglist->Add(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_OTHER, wxSize(16,16)));	// TypeMesh
	imglist->Add(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_OTHER, wxSize(16,16)));	// TypeShape
	imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));		// TypeConstraint
	imglist->Add(wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(16,16)));			// TypeMax (error)
	wxASSERT(imglist->GetImageCount() >= ModelNodeItemData::TypeMax);

	nodeTree = new ModelNodeTreeCtrl(this, UIControlNodeTree, GetClientAreaOrigin(), GetClientSize(),
		wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxTR_EDIT_LABELS );

	nodeTree->AssignImageList(imglist);
	nodeTree->Show(true);
}

ModelTree::~ModelTree(void)
{
}

void ModelTree::OnSize(wxSizeEvent& e)
{
	nodeTree->SetSize(GetClientRect());
}

void ModelTree::OnClose(wxCloseEvent& e)
{
	nodeTree->DeleteAllItems();
}

void ModelTree::OnContextMenu(wxContextMenuEvent& e)
{
	wxPoint pt = e.GetPosition();

	DKLog("OnContextMenu at screen coords (%i, %i)\n", pt.x, pt.y);
}

void ModelTree::OnItemMenu(wxTreeEvent& e)
{
	wxTreeItemId item = e.GetItem();

	wxPoint clientpt = e.GetPoint();
	wxPoint screenpt = nodeTree->ClientToScreen(clientpt);

	DKLog("OnItemMenu (id=%x) at screen coords (%i, %i)\n", item.m_pItem, screenpt.x, screenpt.y);

	nodeTree->SelectItem(item);

	ShowMenu(item, clientpt);
	e.Skip();

}

void ModelTree::OnBeginLabelEdit(wxTreeEvent& e)
{
	wxTreeItemId item = e.GetItem();
	ModelNodeItemData *data = item.IsOk() ? (ModelNodeItemData*)nodeTree->GetItemData(item) : NULL;

	wxASSERT( data );

	if (data->IsNameEditable())
		nodeTree->SetItemText(item, data->GetName());
	else
		e.Veto();
}

void ModelTree::OnEndLabelEdit(wxTreeEvent& e)
{
	wxTreeItemId item = e.GetItem();
	wxString name = e.GetLabel();

	ModelNodeItemData* data = item.IsOk() ? (ModelNodeItemData*)nodeTree->GetItemData(item) : NULL;
	wxASSERT(data);

	if (e.IsEditCancelled() == false && name != (const wchar_t*)data->GetName())
	{
		data->SetName((const wchar_t*)name);

		UpdateTreeItem(item, false);
		nodeTree->SortChildren(nodeTree->GetRootItem());

		if (nodeTree->GetSelection() == item)
			SetActiveItem(data);

		SetModelModified();
	}
	else
	{
		UpdateTreeItem(item, false);
	}
	e.Veto();
}

void ModelTree::ShowMenu(wxTreeItemId item, const wxPoint& pt)
{
	if ( item.IsOk() )
	{
		ModelNodeItemData *data = (ModelNodeItemData*)nodeTree->GetItemData(item);
		wxASSERT(data);

		bool copy = false;
		bool cut = false;
		bool paste = false;

		bool rename = true;
		bool properties = false;

		wxMenu menu;

		menu.Append(UICommandTest, "Create Child Model");

		wxMenu* importData = new wxMenu;
		importData->Append(UICommandTest, "Mesh");
		importData->Append(UICommandTest, "Child Model");
		menu.AppendSubMenu(importData, "Import");

		wxMenu* exportData = new wxMenu;
		exportData->Append(UICommandTest, "Mesh");
		exportData->Append(UICommandTest, "Model");
		exportData->Append(UICommandTest, "Collision Shape");
		menu.AppendSubMenu(exportData, "Export");

		menu.AppendSeparator();

		wxMenu* tangentSpace = new wxMenu;
		tangentSpace->Append(UICommandTest, "Tangent with Handedness");
		tangentSpace->Append(UICommandTest, "Tangent with Binormal");
		menu.AppendSubMenu(tangentSpace, "Update Tangent Space");

		menu.Append(UICommandTest, "Update Face Normals");
		menu.Append(UICommandTest, "Update Collision Shape");
		menu.Append(UICommandUpdateBoundingInfo, "Update Bounding Info");

		menu.AppendSeparator();

		menu.Append(UICommandCreateCollision, "Create Convex HULL");
		menu.Append(UICommandCreateShapeHull, "Create Shape Hull");
		menu.Append(UICommandSetMaterial, "Set Material");

		if (data->type == ModelNodeItemData::TypeMesh)
		{
			menu.Append(UICommandSetDiffuseMap, "Set Bitmap as DiffuseMap");
		}

		menu.AppendSeparator();

		menu.Append(UICommandTest, "Copy");
		menu.Append(UICommandTest, "Cut");
		menu.Append(UICommandTest, "Paste");

		menu.AppendSeparator();
		menu.Append(UICommandRename, "Rename");

		menu.AppendSeparator();
		menu.Append(UICommandTest, "Delete");

		PopupMenu(&menu, pt);
	}
}

void ModelTree::RebuildTree(void)
{
	SetActiveItem(NULL);

	DKModel* m = GetModel();
	wxASSERT(m);
	nodeTree->DeleteAllItems();

	wxTreeItemId root = RebuildTree(m, 0);
	UpdateTreeItem(root, true);
	nodeTree->SortChildren(root);
	nodeTree->Expand(root);
}

wxTreeItemId ModelTree::RebuildTree(DKModel* model, wxTreeItemId parent)
{
	ModelNodeModelData* parentItemData = NULL;
	if (parent.IsOk())
		parentItemData = (ModelNodeModelData*)nodeTree->GetItemData(parent);

	DKCollisionObject* col = NULL;

	wxTreeItemId tid = 0;

	ModelNodeModelData* itemData = NULL;

	switch (model->type)
	{
	case DKModel::TypeMesh:
		itemData = new ModelNodeMeshData(static_cast<DKMesh*>(model), parentItemData);
		break;
	case DKModel::TypeCollision:
		col = static_cast<DKCollisionObject*>(model);
		switch (col->objectType)
		{
		case DKCollisionObject::RigidBody:
			itemData = new ModelNodeRigidBodyData((DKRigidBody*)col, parentItemData);
			break;
		case DKCollisionObject::SoftBody:
			itemData = new ModelNodeSoftBodyData((DKSoftBody*)col, parentItemData);
			break;
		default:
			wxFAIL_MSG("NOT IMPLEMENTED");
			break;
		}
		break;
	default:		// unknown
		itemData = new ModelNodeModelData(model, parentItemData);
	}

	if (parent.IsOk())
		tid = nodeTree->AppendItem(parent, itemData->GetName(), itemData->type, -1, itemData);
	else
		tid = nodeTree->AddRoot(itemData->GetName(), itemData->type, -1, itemData);

	if (col && col->CollisionShape())
	{
		// 콜리젼 쉐이프를 모두 배열 형태로 바꾼다. (compound 하나에 개별 쉐이프들)
		struct ShapeItem
		{
			DKCollisionShape* shape;
			DKNSTransform trans;
		};
		DKArray<ShapeItem> shapeArray;
		
		DKCollisionShape* shape = col->CollisionShape();
		if (shape->IsCompund())
		{
			struct DecomposeShape
			{
				DKArray<ShapeItem>& shapeArray;
				void operator () (DKCompoundShape* compund, const DKNSTransform& trans) const
				{
					for (size_t i = 0; i < compund->NumberOfShapes(); ++i)
					{
						DKCollisionShape* s = compund->ShapeAtIndex(i);
						DKNSTransform t = compund->TransformAtIndex(i) * trans;

						if (s->IsCompund())
						{
							this->operator()((DKCompoundShape*)s, t);
						}
						else
						{
							ShapeItem item = {s, t};
							shapeArray.Add(item);
						}
					}
				}
			};
			DecomposeShape decomp = {shapeArray};
			decomp((DKCompoundShape*)shape, DKNSTransform::identity);
		}
		else
		{
			ShapeItem item = {shape, col->LocalTransform()};
			shapeArray.Add(item);
		}
		for (ShapeItem& si : shapeArray)
		{
			ModelNodeItemData* shapeData = new ModelNodeShapeData(si.shape, si.trans, itemData);
			nodeTree->AppendItem(tid, shapeData->GetName(), shapeData->type, -1, shapeData);
		}
	}

	for (size_t i = 0; i < model->NumberOfChildren(); ++i)
	{
		DKModel* sub = model->ChildAtIndex(i);
		RebuildTree(sub, tid);
	}
	return tid;
}

void ModelTree::UpdateTreeItem(wxTreeItemId item, bool recursive)
{
	wxASSERT(item.IsOk());
	ModelNodeItemData *data = (ModelNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	wxString itemText = "";

	DKString resName = data->GetName();

	switch (data->type)
	{
	case ModelNodeItemData::TypeModel:
		itemText = resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Model";
		break;
	case ModelNodeItemData::TypeRigidBody:
		itemText = resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled RigidBody";
		break;
	case ModelNodeItemData::TypeSoftBody:
		itemText = resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled SoftBody";
		break;
	case ModelNodeItemData::TypeMesh:
		itemText = resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Mesh";
		break;
	case ModelNodeItemData::TypeShape:
		itemText = wxString::Format("Shape (%ls)", (const wchar_t*)resName);
		break;
	case ModelNodeItemData::TypeConstraint:
		itemText = wxString::Format("Constraint (%ls)", (const wchar_t*)resName);
		break;
	default:
		wxFAIL_MSG("Invalid Item Type");
		break;
	}

	nodeTree->SetItemText(item, itemText);
	nodeTree->SetItemTextColour(item, data->GetItemColor());

	if (recursive)
	{
		wxTreeItemIdValue cookie;
		for (wxTreeItemId child = nodeTree->GetFirstChild(item, cookie); child.IsOk(); child = nodeTree->GetNextChild(item, cookie))
		{
			UpdateTreeItem(child, true);
		}
	}
}

void ModelTree::SortEnclosingItem(wxTreeItemId item)
{
	wxASSERT(item.IsOk());
	wxTreeItemId parentItem = nodeTree->GetItemParent(item);
	if (parentItem.IsOk())
	{
		nodeTree->SortChildren(parentItem);
	}
}

void ModelTree::OnRename(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
		nodeTree->EditLabel(item);
	}
}

void ModelTree::OnCreateCollision(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
		ModelNodeItemData* itemData = (ModelNodeItemData*)(nodeTree->GetItemData(item));
		if (itemData->type == ModelNodeItemData::TypeModel)
		{
/*
			DKModel* target = static_cast<ModelNodeModelData*>(itemData)->model;
			wxASSERT(target != NULL);

			size_t numMeshes = target->NumberOfMeshes();

			wxASSERT(numMeshes > 0);

			DKArray<DKTriangle> triangles;
			// 모든 매시에서 triangle 을 추출함.
			for (size_t i = 0; i < numMeshes; ++i)
			{
				DKMesh* mesh = target->MeshAtIndex(i);
				DKMatrix4 tm = target->MeshTransformAtIndex(i);


			}
*/
		}
		else
		{
			wxFAIL_MSG("Invalid Object");
		}
	}
}

void ModelTree::OnCreateShapeHull(wxCommandEvent& e)
{
	// 정점을 추출하여 btShapeHull 을 이용하여 convex-hull 생성함. 
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
	}
}

void ModelTree::OnSetMaterial(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
		ModelNodeItemData* itemData = (ModelNodeItemData*)(nodeTree->GetItemData(item));
		DKModel* model = itemData->GetModel();
		wxASSERT(model != NULL);

		DKObject<DKMaterial> mat = OpenMaterial();
		if (mat)
		{
			bool recursive = false;
			if (model->NumberOfChildren() > 0)
			{
				wxMessageDialog dlg(this, "Update Recursive?", "recursive update", wxYES_NO | wxCANCEL | wxYES_DEFAULT | wxICON_QUESTION);
				int ret = dlg.ShowModal();
				if (ret == wxID_YES)
					recursive = true;
				else if (ret == wxID_CANCEL)
					return;
			}

			LockRenderer();

			bool forcedApply = false;
			auto matSetter = [&mat, forcedApply](DKModel* obj)
			{
				if (obj->type == DKModel::TypeMesh)
				{
					DKMesh* mesh = static_cast<DKMesh*>(obj);
					bool valid = mesh->CanAdoptMaterial(mat);
					if (valid || forcedApply)
						mesh->SetMaterial(mat);
					if (!valid)
					{
						if (forcedApply)
							DKLog("Mesh:%ls cannot adopt material, set forced.\n", (const wchar_t*)mesh->Name());
						else
							DKLog("Mesh:%ls cannot adopt material!\n", (const wchar_t*)mesh->Name());
					}
				}
			};

			if (recursive)
				model->Enumerate(DKFunction(matSetter));
			else
			{				
				if (model->type == DKModel::TypeMesh)
					matSetter(model);
				else
					DKLog("Model:%ls is not a mesh!\n", (const wchar_t*)model->Name());
			}

			UnlockRenderer();
			SetModelModified();
			// material 이 갱신되었으므로 선택된 아이템을 다시 선택함.
			// (아이템이 선택되었을때 DKModel::Clone() 을 통해 복사하므로, 다시 복사해야함.
			SetActiveItem(itemData);
		}
	}
}

void ModelTree::OnSetDiffuseMap(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
		ModelNodeItemData* itemData = (ModelNodeItemData*)(nodeTree->GetItemData(item));
		DKModel* model = itemData->GetModel();
		wxASSERT(model != NULL);

		if (model->type == DKModel::TypeMesh)
		{
			DKMesh* mesh = dynamic_cast<DKMesh*>(model);
			wxASSERT(mesh != NULL);

			DKObject<DKTexture> tex = OpenTexture();
			if (tex)
			{
				LockRenderer();

				mesh->SetSampler(L"diffuseMap", tex, NULL);

				UnlockRenderer();
				SetModelModified();
				SetActiveItem(itemData);
			}
		}
	}
}

void ModelTree::OnUpdateBoundingInfo(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
		//DKModel* target = reinterpret_cast<ModelNodeItemData*>(nodeTree->GetItemData(item))->data;

		//DKMesh* mesh = target->Mesh();
		//if (mesh)
		//	mesh->UpdateBoundingInfo();

		//target->UpdateBoundingInfo();
		//target->RebuildBoundingInfo();
		//SetModelModified();
	}
}

void ModelTree::OnTest(wxCommandEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
}

void ModelTree::OnItemActivated(wxTreeEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	ModelNodeItemData *data = (ModelNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	if (data->type == ModelNodeItemData::TypeModel)
		DKLog("Item Activated (Type:Model, name:%ls)\n", (const wchar_t*)data->GetName());
	else if (data->type == ModelNodeItemData::TypeMesh)
		DKLog("Item Activated (Type:Mesh, name:%ls)\n", (const wchar_t*)data->GetName());
	else
		DKLog("Item Activated (Type:0x%x, name:%ls)\n", data->type, (const wchar_t*)data->GetName());
}

void ModelTree::OnItemSelected(wxTreeEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	ModelNodeItemData *data = (ModelNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	SetActiveItem(data);
}
