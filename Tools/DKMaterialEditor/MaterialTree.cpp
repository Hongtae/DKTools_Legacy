#include "StdAfx.h"
#include <wx/artprov.h>
#include "MaterialTree.h"
#include "MaterialNode.h"

void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
DKMaterial* GetMaterial(void);
void SetMaterialModified(void);
void OpenEditor(DKMaterial::ShaderSource* src);
bool CloseEditor(DKMaterial::ShaderSource* src);
void UpdateEditor(DKMaterial::ShaderSource* src);
void OpenProperties(MaterialNodeItemData* item);

size_t FindIndex(const DKMaterial* mat, const DKMaterial::RenderingProperty* prop)
{
	for (size_t index = 0; index < mat->renderingProperties.Count(); index++)
	{
		if ( prop == &mat->renderingProperties.Value(index))
			return index;
	}
	wxASSERT_MSG(0, "no entry?");
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
// 아이템 정렬을 위해 트리 상속
class MaterialNodeTreeCtrl : public wxTreeCtrl
{
	DECLARE_DYNAMIC_CLASS(MaterialNodeTreeCtrl)
public:
	MaterialNodeTreeCtrl(void) {}
	MaterialNodeTreeCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS, const wxValidator& validator = wxDefaultValidator, const wxString& name = "customTreeCtrl")
		: wxTreeCtrl(parent, id, pos, size, style, validator, name) {}
protected:
	int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2)
	{
		// ShadingProperty 는 이름순
		// RenderingProperty 는 위치순 (배열 인덱스)
		MaterialNodeItemData* data1 = reinterpret_cast<MaterialNodeItemData*>(GetItemData(item1));
		MaterialNodeItemData* data2 = reinterpret_cast<MaterialNodeItemData*>(GetItemData(item2));

		wxASSERT(data1);
		wxASSERT(data2);

		if (data1->type == data2->type)
		{
			switch (data1->type)
			{
			case MaterialNodeItemData::TypeProgram:
				return reinterpret_cast<MaterialNodeProgramData*>(data1)->data > reinterpret_cast<MaterialNodeProgramData*>(data2)->data;
				break;
			}
			return wxString(data1->GetName()).Cmp(data2->GetName());
		}
		return data1->type > data2->type;
	}
};


wxIMPLEMENT_DYNAMIC_CLASS(MaterialNodeTreeCtrl, wxTreeCtrl);

////////////////////////////////////////////////////////////////////////////////
wxBEGIN_EVENT_TABLE(MaterialTree, wxWindow)
	// 윈도 메시지
	EVT_CONTEXT_MENU(								MaterialTree::OnContextMenu)
	EVT_SIZE(										MaterialTree::OnSize)
	EVT_CLOSE(										MaterialTree::OnClose)
	// 메뉴
	EVT_MENU(UICommandAddProgram,					MaterialTree::OnAddProgram)
	EVT_MENU(UICommandAddSampler,					MaterialTree::OnAddSampler)
	EVT_MENU(UICommandAddUniform,					MaterialTree::OnAddUniform)
	EVT_MENU(UICommandAddStream,					MaterialTree::OnAddStream)
	EVT_MENU(UICommandAddVertexShader,				MaterialTree::OnAddVertexShader)
	EVT_MENU(UICommandAddGeometryShader,			MaterialTree::OnAddGeometryShader)
	EVT_MENU(UICommandAddFragmentShader,			MaterialTree::OnAddFragmentShader)

	EVT_MENU(UICommandRemove,						MaterialTree::OnRemove)
	EVT_MENU(UICommandRename,						MaterialTree::OnRename)
	EVT_MENU(UICommandMoveUp,						MaterialTree::OnMoveUp)
	EVT_MENU(UICommandMoveDown,						MaterialTree::OnMoveDown)

	EVT_MENU(UICommandEdit,							MaterialTree::OnEdit)
	EVT_MENU(UICommandBuild,						MaterialTree::OnBuild)
	EVT_MENU(UICommandCompile,						MaterialTree::OnCompile)

	EVT_MENU(UICommandTest,							MaterialTree::OnTest)
	// 트리 콘트롤
	EVT_TREE_ITEM_MENU(UIControlNodeTree,			MaterialTree::OnItemMenu)
	EVT_TREE_BEGIN_LABEL_EDIT(UIControlNodeTree,	MaterialTree::OnBeginLabelEdit)
	EVT_TREE_END_LABEL_EDIT(UIControlNodeTree,		MaterialTree::OnEndLabelEdit)
	EVT_TREE_ITEM_ACTIVATED(UIControlNodeTree,		MaterialTree::OnItemActivated)
	EVT_TREE_SEL_CHANGED(UIControlNodeTree,			MaterialTree::OnItemSelected)
wxEND_EVENT_TABLE();

MaterialTree::MaterialTree(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
{
	wxImageList* imglist = new wxImageList(16, 16, true, 2);
	imglist->Add(wxArtProvider::GetBitmap(wxART_FOLDER, wxART_OTHER, wxSize(16,16)));				// TypeMaterial
	imglist->Add(wxArtProvider::GetBitmap(wxART_NORMAL_FILE, wxART_OTHER, wxSize(16,16)));			// TypeShader
	imglist->Add(wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, wxSize(16,16)));					// TypeSampler
	imglist->Add(wxArtProvider::GetBitmap(wxART_HELP_BOOK, wxART_OTHER, wxSize(16,16)));			// TypeUniform
	imglist->Add(wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_OTHER, wxSize(16,16)));				// TypeStream
	imglist->Add(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE , wxART_OTHER, wxSize(16,16)));		// TypeProgram
	imglist->Add(wxArtProvider::GetBitmap(wxART_ERROR, wxART_OTHER, wxSize(16,16)));				// TypeMax (error)
	wxASSERT(imglist->GetImageCount() >= MaterialNodeItemData::TypeMax);

	nodeTree = new MaterialNodeTreeCtrl(this, UIControlNodeTree, GetClientAreaOrigin(), GetClientSize(),
		wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_SINGLE | wxTR_EDIT_LABELS);
	nodeTree->AssignImageList(imglist);
	nodeTree->Show(true);
}

MaterialTree::~MaterialTree(void)
{
}

void MaterialTree::OnContextMenu(wxContextMenuEvent& e)
{
	wxPoint pt = e.GetPosition();

	DKLog("OnContextMenu at screen coords (%i, %i)\n", pt.x, pt.y);
}

void MaterialTree::OnItemMenu(wxTreeEvent& e)
{
	wxTreeItemId item = e.GetItem();
	MaterialNodeItemData *data = item.IsOk() ? (MaterialNodeItemData*)nodeTree->GetItemData(item) : NULL;

	wxPoint clientpt = e.GetPoint();
	wxPoint screenpt = nodeTree->ClientToScreen(clientpt);

	DKLog("OnItemMenu (id=%x) at screen coords (%i, %i)\n", item.m_pItem, screenpt.x, screenpt.y);

	nodeTree->SelectItem(item);

	ShowMenu(item, clientpt);
	e.Skip();
}

void MaterialTree::OnBeginLabelEdit(wxTreeEvent& e)
{
	wxTreeItemId item = e.GetItem();
	MaterialNodeItemData *data = item.IsOk() ? (MaterialNodeItemData*)nodeTree->GetItemData(item) : NULL;
	if (data)
		nodeTree->SetItemText(item, (const wchar_t*)data->GetName());
	else
		e.Veto();
}

void MaterialTree::OnEndLabelEdit(wxTreeEvent& e)
{
	wxTreeItemId item = e.GetItem();
	wxString name = e.GetLabel();

	MaterialNodeItemData *data = item.IsOk() ? (MaterialNodeItemData*)nodeTree->GetItemData(item) : NULL;
	wxASSERT(data);

	if (e.IsEditCancelled() == false && !name.IsNull() && !name.IsEmpty() && name != data->GetName())
	{
		if (		// 유니폼, 스트림, 샘플러는 이름이 유니크 해야한다.
			(data->type == MaterialNodeItemData::TypeUniform && GetMaterial()->shadingProperties.Find((const wchar_t*)name)) ||
			(data->type == MaterialNodeItemData::TypeStream && GetMaterial()->streamProperties.Find((const wchar_t*)name)) ||
			(data->type == MaterialNodeItemData::TypeSampler && GetMaterial()->samplerProperties.Find((const wchar_t*)name)))
		{
			wxMessageDialog dlg(this, "already exist!", "name conflict", wxOK | wxICON_ERROR );
			dlg.ShowModal();
			e.Veto();
			return;
		}
		else if (!data->SetName(name))
		{
			wxMessageDialog dlg(this, "Unable to set object name.", "Set Name Error", wxOK | wxICON_ERROR );
			dlg.ShowModal();
			e.Veto();
			return;
		}

		// 쉐이더소스의 이름이 바뀐거면 에디터도 갱신한다. (에디터가 열려있다면 갱신된다)
		if (data->type == MaterialNodeItemData::TypeShader)
		{
			UpdateEditor(reinterpret_cast<MaterialNodeShaderData*>(data)->data);
		}

		UpdateTreeItem(item, false);
		nodeTree->SortChildren(nodeTree->GetRootItem());

		if (nodeTree->GetSelection() == item)
			OpenProperties(data);

		SetMaterialModified();
	}
	else
	{
		UpdateTreeItem(item, false);
	}
	e.Veto();
}

void MaterialTree::OnSize(wxSizeEvent& e)
{
//	e.Skip();
	nodeTree->SetSize(GetClientRect());
	nodeTree->Show(true);
}

void MaterialTree::OnClose(wxCloseEvent& e)
{
//	e.Skip();
}

void MaterialTree::ShowMenu(wxTreeItemId item, const wxPoint& pt)
{
	// 머티리얼 자체에는 유니폼,프로그램,텍스쳐,쉐이더,스트림을 가질 수 있다.
	// 프로그램은 쉐이더만 가질 수 있다.
	// 쉐이더는 버텍스쉐이더, 프래그먼트쉐이더, 지오메트리 쉐이더를 가질 수 있다.
	// 프로퍼티는 형식과 값을 가질수 있다.
	// 텍스쳐는 파일과 속성을 가질 수 있다.

	if ( item.IsOk() )
	{
		MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
		wxASSERT(data);

		bool copy = false;
		bool cut = false;
		bool paste = false;
		bool remove = true;

		bool rename = true;
		bool properties = false;

		bool addProgram = false;
		bool addUniform = false;
		bool addStream = false;
		bool addSampler = false;
		bool addShader = false;

		bool edit = false;
		bool build = false;
		bool compile = false;

		wxMenu menu;

		switch (data->type)
		{
		case MaterialNodeItemData::TypeMaterial:
			addProgram = true;
			addUniform = true;
			addStream = true;
			addSampler = true;
			addShader = true;
			build = true;
			remove = false;
			break;
		case MaterialNodeItemData::TypeProgram:
			addShader = true;
			build = true;
			break;
		case MaterialNodeItemData::TypeShader:
			compile = true;
			edit = true;
			break;
		case MaterialNodeItemData::TypeSampler:
			break;
		case MaterialNodeItemData::TypeUniform:
			break;
		case MaterialNodeItemData::TypeStream:
			break;
		default:
			break;
		}

		if (menu.GetMenuItemCount() > 1)
			menu.AppendSeparator();
		if (copy)
			menu.Append(UICommandTest, "Copy");
		if (cut)
			menu.Append(UICommandTest, "Cut");
		if (paste)
			menu.Append(UICommandTest, "Paste");

		if (menu.GetMenuItemCount() > 1 && (addProgram || addSampler || addUniform || addStream || addShader))
			menu.AppendSeparator();

		if (addProgram)
			menu.Append(UICommandAddProgram, "Add &Program");
		if (addSampler)
			menu.Append(UICommandAddSampler, "Add &Sampler");
		if (addUniform)
			menu.Append(UICommandAddUniform, "Add &Uniform");
		if (addStream)
			menu.Append(UICommandAddStream, "Add &Stream");
		if (addShader)
		{
			wxMenu*	shader = new wxMenu;
			shader->Append(UICommandAddVertexShader, "Vertex Shader");
			shader->Append(UICommandAddFragmentShader, "Fragment Shader");
			shader->Append(UICommandAddGeometryShader, "Geometry Shader");
			menu.AppendSubMenu(shader, "Add Shader");
		}
		if (menu.GetMenuItemCount() > 1 && (build || compile || edit))
		{
			menu.AppendSeparator();
		}
		if (edit)
			menu.Append(UICommandEdit, "Edit");
		if (build)
			menu.Append(UICommandBuild, "Build");
		if (compile)
			menu.Append(UICommandCompile, "Compile");

		if (menu.GetMenuItemCount() > 1 && (rename || properties))
			menu.AppendSeparator();

		if (rename)
			menu.Append(UICommandRename, "Rename");
		if (remove)
			menu.Append(UICommandRemove, "Remove");
		if (properties)
			menu.Append(UICommandTest, "Properties");
		
		PopupMenu(&menu, pt);
	}
	else
	{
	}
}

void MaterialTree::OnAddProgram(wxCommandEvent& e)
{
	wxASSERT(nodeTree->GetRootItem() == nodeTree->GetSelection());

	wxTreeItemId root = nodeTree->GetRootItem();

	DKMaterial* mat = GetMaterial();
	wxASSERT(mat);

	DKArray<DKMaterial::RenderingProperty>::Index id = mat->renderingProperties.Add(DKMaterial::RenderingProperty());
	DKMaterial::RenderingProperty& prop = mat->renderingProperties.Value(id);
	prop.depthFunc = DKMaterial::RenderingProperty::DepthFuncAlways;
	prop.depthWrite = false;
	prop.name = "Untitled Program";
	wxTreeItemId item = nodeTree->AppendItem(root, "", MaterialNodeItemData::TypeProgram, -1, new MaterialNodeProgramData(&prop));
	nodeTree->SortChildren(root);
	nodeTree->SelectItem(item);
	nodeTree->EditLabel(item);
}

void MaterialTree::OnAddSampler(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::OnAddUniform(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	// 유니폼 속성 다이얼로그를 띄워서 속성 가져온 후 추가해야 한다.

	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		DKMaterial* mat = reinterpret_cast<DKMaterial*>(data->GetData());
		DKString uniformName = "";
		for (int i = 0; true ; i++)
		{
			uniformName = DKString::Format("Variable_%d", i);
			if (mat->shadingProperties.Find(uniformName) == NULL)
				break;
		}
		// 테스트 (나중에 타입 다이얼로그 띄워야함)
		DKMaterial::ShadingProperty& prop = mat->shadingProperties.Value(uniformName);
		prop.id = DKShaderConstant::UniformUnknown;
		prop.type = DKShaderConstant::TypeUnknown;

		wxTreeItemId itemid = nodeTree->AppendItem(item, "", MaterialNodeItemData::TypeUniform, -1, new MaterialNodeUniformData(uniformName, &prop));
		nodeTree->SortChildren(item);
		nodeTree->SelectItem(itemid);
		nodeTree->EditLabel(itemid);
	}

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::OnAddStream(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	// TODO: 스트림 속성 다이얼로그를 띄워서 속성 가져온 후 추가할것
	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		DKMaterial* mat = reinterpret_cast<DKMaterial*>(data->GetData());
		DKString streamName = "";
		for (int i = 0; true ; i++)
		{
			streamName = DKString::Format("Stream_%d", i);
			if (mat->streamProperties.Find(streamName) == NULL)
				break;
		}
		// 테스트로 그냥 붙여봄
		DKMaterial::StreamProperty& prop = mat->streamProperties.Value(streamName);
		prop.id = DKVertexStream::StreamUnknown;
		prop.type = DKVertexStream::TypeUnknown;
		prop.components = 0;

		wxTreeItemId itemid = nodeTree->AppendItem(item, "", MaterialNodeItemData::TypeStream, -1, new MaterialNodeStreamData(streamName, &prop));
		nodeTree->SortChildren(item);
		nodeTree->SelectItem(itemid);
		nodeTree->EditLabel(itemid);
	}

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::OnAddVertexShader(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	DKMaterial::ShaderSource shader = {"untitled shader", "", DKShader::TypeVertexShader, NULL};
	DKMaterial::ShaderSource* src = NULL;
	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		DKMaterial* mat = reinterpret_cast<DKMaterial*>(data->GetData());
		DKArray<DKMaterial::ShaderSource>::Index index = mat->shaderProperties.Add(shader);
		src = &mat->shaderProperties.Value(index);
	}
	else if (data->type == MaterialNodeItemData::TypeProgram)
	{
		DKMaterial::RenderingProperty* prog = reinterpret_cast<DKMaterial::RenderingProperty*>(data->GetData());
		DKArray<DKMaterial::ShaderSource>::Index index = prog->shaders.Add(shader);
		src = &prog->shaders.Value(index);
	}
	else
	{
		wxASSERT_MSG(0, "Wrong point!");
	}

	wxTreeItemId itemid = nodeTree->AppendItem(item, "", MaterialNodeItemData::TypeShader, -1, new MaterialNodeShaderData(src));
	nodeTree->SortChildren(item);
	nodeTree->SelectItem(itemid);
	nodeTree->EditLabel(itemid);

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::OnAddGeometryShader(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	DKMaterial::ShaderSource shader = {"untitled shader", "", DKShader::TypeGeometryShader, NULL};
	DKMaterial::ShaderSource* src = NULL;
	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		DKMaterial* mat = reinterpret_cast<DKMaterial*>(data->GetData());
		DKArray<DKMaterial::ShaderSource>::Index index = mat->shaderProperties.Add(shader);
		src = &mat->shaderProperties.Value(index);
	}
	else if (data->type == MaterialNodeItemData::TypeProgram)
	{
		DKMaterial::RenderingProperty* prog = reinterpret_cast<DKMaterial::RenderingProperty*>(data->GetData());
		DKArray<DKMaterial::ShaderSource>::Index index = prog->shaders.Add(shader);
		src = &prog->shaders.Value(index);
	}
	else
	{
		wxASSERT_MSG(0, "Wrong point!");
	}

	wxTreeItemId itemid = nodeTree->AppendItem(item, "", MaterialNodeItemData::TypeShader, -1, new MaterialNodeShaderData(src));
	nodeTree->SortChildren(item);
	nodeTree->SelectItem(itemid);
	nodeTree->EditLabel(itemid);

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::OnAddFragmentShader(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	DKMaterial::ShaderSource shader = {"untitled shader", "", DKShader::TypeFragmentShader, NULL};
	DKMaterial::ShaderSource* src = NULL;
	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		DKMaterial* mat = reinterpret_cast<DKMaterial*>(data->GetData());
		DKArray<DKMaterial::ShaderSource>::Index index = mat->shaderProperties.Add(shader);
		src = &mat->shaderProperties.Value(index);
	}
	else if (data->type == MaterialNodeItemData::TypeProgram)
	{
		DKMaterial::RenderingProperty* prog = reinterpret_cast<DKMaterial::RenderingProperty*>(data->GetData());
		DKArray<DKMaterial::ShaderSource>::Index index = prog->shaders.Add(shader);
		src = &prog->shaders.Value(index);
	}
	else
	{
		wxASSERT_MSG(0, "Wrong point!");
	}

	wxTreeItemId itemid = nodeTree->AppendItem(item, "", MaterialNodeItemData::TypeShader, -1, new MaterialNodeShaderData(src));
	nodeTree->SortChildren(item);
	nodeTree->SelectItem(itemid);
	nodeTree->EditLabel(itemid);

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::RebuildTree(void)
{
	DKMaterial* mat = GetMaterial();
	wxASSERT(mat);
	nodeTree->DeleteAllItems();

	wxTreeItemId root = nodeTree->AddRoot("", MaterialNodeItemData::TypeMaterial, -1, new MaterialNodeMaterialData(mat));
	RebuildTree(nodeTree->GetItemData(root));

	UpdateTreeItem(root, true);
	nodeTree->SortChildren(root);
	nodeTree->Expand(root);
}

void MaterialTree::RebuildTree(wxTreeItemData* p)
{
	// 타입별로 자식 노드들 추가후 재귀호출 (자기 자신은 이미 추가된 후임)
	switch (reinterpret_cast<MaterialNodeItemData*>(p)->type)
	{
	case MaterialNodeItemData::TypeMaterial:
		{
			wxASSERT(GetMaterial() == reinterpret_cast<MaterialNodeItemData*>(p)->GetData());
			MaterialNodeMaterialData* node = reinterpret_cast<MaterialNodeMaterialData*>(p);
			DKMaterial* material = reinterpret_cast<DKMaterial*>(node->GetData());

			// ShadingProperty 추가
			material->shadingProperties.EnumerateForward([=](DKMaterial::ShadingPropertyMap::Pair& pair)
														{
															wxTreeItemId item = nodeTree->AppendItem(p->GetId(), "", MaterialNodeItemData::TypeUniform, -1,
																									 new MaterialNodeUniformData(pair.key, &pair.value));
															RebuildTree(nodeTree->GetItemData(item));
														});

			// StreamProperty 추가
			material->streamProperties.EnumerateForward([=](DKMaterial::StreamPropertyMap::Pair& pair)
														{
															wxTreeItemId item = nodeTree->AppendItem(p->GetId(), "", MaterialNodeItemData::TypeStream, -1,
																									 new MaterialNodeStreamData(pair.key, &pair.value));
															RebuildTree(nodeTree->GetItemData(item));
														});

			// RenderingProperty 추가
			for (int i = 0; i < material->renderingProperties.Count(); i++)
			{
				wxTreeItemId item = nodeTree->AppendItem(p->GetId(), "", MaterialNodeItemData::TypeProgram, -1,
					new MaterialNodeProgramData(&material->renderingProperties.Value(i)));
				RebuildTree(nodeTree->GetItemData(item));
			}
			// ShaderSource 추가
			for (int i = 0; i < material->shaderProperties.Count(); i++)
			{
				wxTreeItemId item = nodeTree->AppendItem(p->GetId(), "", MaterialNodeItemData::TypeShader, -1,
					new MaterialNodeShaderData(&material->shaderProperties.Value(i)));
				RebuildTree(nodeTree->GetItemData(item));
			}
		}
		break;
	case MaterialNodeItemData::TypeProgram:
		{
			MaterialNodeProgramData* data = reinterpret_cast<MaterialNodeProgramData*>(p);
			// ShaderSource 추가
			for (int i = 0; i < reinterpret_cast<DKMaterial::RenderingProperty*>(data->GetData())->shaders.Count(); i++)
			{
				wxTreeItemId item = nodeTree->AppendItem(p->GetId(), "", MaterialNodeItemData::TypeShader, -1,
					new MaterialNodeShaderData(&reinterpret_cast<DKMaterial::RenderingProperty*>(data->GetData())->shaders.Value(i)));
				RebuildTree(nodeTree->GetItemData(item));
			}
		}
		break;
	case MaterialNodeItemData::TypeShader:
		break;
	case MaterialNodeItemData::TypeSampler:
		break;
	case MaterialNodeItemData::TypeUniform:
		{
			MaterialNodeUniformData* data = reinterpret_cast<MaterialNodeUniformData*>(p);
		}
		break;
	case MaterialNodeItemData::TypeStream:
		{
			MaterialNodeStreamData* data = reinterpret_cast<MaterialNodeStreamData*>(p);
		}
		break;
	default:
		wxASSERT_MSG(0, "must not enter here!");
		break;
	}
	nodeTree->SortChildren(p->GetId());
}

void MaterialTree::UpdateTreeItem(wxTreeItemId item, bool recursive)
{
	wxASSERT(item.IsOk());
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	wxString itemText = "";
	DKString resName = data->GetName();

	switch (data->type)
	{
	case MaterialNodeItemData::TypeProgram:
		itemText = (const wchar_t*)DKString::Format("%d:%ls",
			FindIndex(GetMaterial(), reinterpret_cast<MaterialNodeProgramData*>(data)->data),
			resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Program" );
		break;
	case MaterialNodeItemData::TypeShader:
		itemText = (const wchar_t*)DKString::Format("%ls:%ls",
			reinterpret_cast<MaterialNodeShaderData*>(data)->data->type == DKShader::TypeVertexShader ? L"VS" :
			(reinterpret_cast<MaterialNodeShaderData*>(data)->data->type == DKShader::TypeFragmentShader ? L"FS" :
			(reinterpret_cast<MaterialNodeShaderData*>(data)->data->type == DKShader::TypeGeometryShader ? L"GS" : L"??")),
			resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Shader" );
		break;
	case MaterialNodeItemData::TypeUniform:
		itemText = (const wchar_t*)DKString::Format("%ls:%ls",
			(const wchar_t*)DKShaderConstant::UniformToString(reinterpret_cast<MaterialNodeUniformData*>(data)->data->id),
			resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Uniform" );
		break;
	case MaterialNodeItemData::TypeStream:
		itemText = (const wchar_t*)DKString::Format("%ls:%ls",
			(const wchar_t*)DKVertexStream::StreamToString(reinterpret_cast<MaterialNodeStreamData*>(data)->data->id),
			resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Stream" );
		break;
	case MaterialNodeItemData::TypeMaterial:
		itemText = resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Material";
		break;
	case MaterialNodeItemData::TypeSampler:
		itemText = resName.Length() > 0 ? (const wchar_t*)resName : L"Untitled Sampler";
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

void MaterialTree::OnRename(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	if (item.IsOk())
	{
		nodeTree->EditLabel(item);
	}
}

void MaterialTree::OnRemove(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);
	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		PrintLog("Material cannot be removed.\n");
	}
	else
	{
		if (data->type == MaterialNodeItemData::TypeStream)
		{
			DKString streamName = data->GetName();
			GetMaterial()->streamProperties.Remove(streamName);

		}
		else if (data->type == MaterialNodeItemData::TypeUniform)
		{
			DKString uniformName = data->GetName();
			GetMaterial()->shadingProperties.Remove(uniformName);
		}
		nodeTree->Delete(item);
		e.Skip();
	}
}

void MaterialTree::OnMoveUp(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data && data->type == MaterialNodeItemData::TypeProgram);

	// 인덱스 찾아서 바로 전꺼와 위치 변경함
}

void MaterialTree::OnMoveDown(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data && data->type == MaterialNodeItemData::TypeProgram);

	// 인덱스 찾아서 바로 다음꺼와 위치 변경
}

void MaterialTree::OnEdit(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data && data->type == MaterialNodeItemData::TypeShader);

	OpenEditor(reinterpret_cast<DKMaterial::ShaderSource*>(data->GetData()));
}

void MaterialTree::OnBuild(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	DKMaterial* mat = GetMaterial();
	wxASSERT(mat);

	if (data->type == MaterialNodeItemData::TypeMaterial)
	{
		DKMaterial::BuildLog log;
		if (mat->Build(&log))
		{
			if (log.errorLog.Length() > 0)
			{
				PrintLog(wxString::Format("Build Message: %ls\n", (const wchar_t*)log.errorLog));
			}
			PrintLog("Build succeed.\n");
		}
		else
		{
			PrintLog(wxString::Format("%ls\nBuild failed.\n", (const wchar_t*)log.errorLog));
			PrintLog(wxString::Format("While trying to compile shader named %ls\n", (const wchar_t*)log.failedShader));
		}
	}
	else if (data->type == MaterialNodeItemData::TypeProgram)
	{
		long index = FindIndex(mat, reinterpret_cast<MaterialNodeProgramData*>(data)->data);
		DKMaterial::BuildLog log;
		if (mat->BuildProgram(index, &log))
		{
			if (log.errorLog.Length() > 0)
				PrintLog(wxString::Format("Build Message: %ls\n", (const wchar_t*)log.errorLog));
			PrintLog("Build succeed.\n");
		}
		else
		{
			PrintLog(wxString::Format("%ls\nBuild failed.\n", (const wchar_t*)log.errorLog));
			PrintLog(wxString::Format("While trying to compile shader named %ls\n", (const wchar_t*)log.failedShader));
		}
	}

	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
}

void MaterialTree::OnCompile(wxCommandEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data && data->type == MaterialNodeItemData::TypeShader);
	DKMaterial::ShaderSource* src = reinterpret_cast<MaterialNodeShaderData*>(data)->data;

	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));

	PrintLog(wxString::Format("Compiling shader %ls\n", (const wchar_t*)src->name), wxColour(0,0,255));
	DKString result = "";
	if (DKMaterial::CompileShaderSource(src, result))
		PrintLog("Compilation succeed.\n", wxColour(0,0,255));
	else
		PrintLog("Compilation failed.\n", wxColour(0,0,255));

	if (result.Length())
		PrintLog(wxString::Format("Compiler message below:\n%ls\n", (const wchar_t*)result), wxColour(255,0,0));
}

void MaterialTree::OnTest(wxCommandEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
}

void MaterialTree::OnItemActivated(wxTreeEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	if (data->type == MaterialNodeItemData::TypeShader)
	{
		OpenEditor(reinterpret_cast<DKMaterial::ShaderSource*>(data->GetData()));
	}
	PrintLog(wxString::Format("[%s] target: %ls\n", DKLIB_FUNCTION_NAME, data->GetName()), wxColour(255,0,0));
}

void MaterialTree::OnItemSelected(wxTreeEvent& e)
{
	wxTreeItemId item = nodeTree->GetSelection();
	MaterialNodeItemData *data = (MaterialNodeItemData*)nodeTree->GetItemData(item);
	wxASSERT(data);

	OpenProperties(data);
}
