#include "StdAfx.h"
#include "MaterialProperties.h"

void PrintLog(const wxString& msg, const wxColour& c = wxColour(0,0,0));
DKMaterial* GetMaterial(void);
void SetMaterialModified(void);
void UpdateTreeItem(wxTreeItemId item);
void UpdateEditor(DKMaterial::ShaderSource* shader);

wxBEGIN_EVENT_TABLE(MaterialProperties, wxWindow)
	// 윈도 메시지
	EVT_CONTEXT_MENU(								MaterialProperties::OnContextMenu)
	EVT_SIZE(										MaterialProperties::OnSize)
	EVT_CLOSE(										MaterialProperties::OnClose)
	// 메뉴
	EVT_MENU(UICommandTest,							MaterialProperties::OnTest)
	// 프로퍼티그리드 이벤트
	EVT_PG_CHANGING(UIControlPropertyGrid,			MaterialProperties::OnPropertyChanging)
	EVT_PG_CHANGED(UIControlPropertyGrid,			MaterialProperties::OnPropertyChanged)
wxEND_EVENT_TABLE();

MaterialProperties::MaterialProperties(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
	, materialNode(NULL)
{
	properties = new wxPropertyGrid(this, UIControlPropertyGrid, GetClientAreaOrigin(), GetClientSize(), wxPG_SPLITTER_AUTO_CENTER | wxPG_DEFAULT_STYLE);
}

MaterialProperties::~MaterialProperties(void)
{
}

void MaterialProperties::OnContextMenu(wxContextMenuEvent& e)
{
	wxPoint pt = e.GetPosition();

	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
	DKLog("OnContextMenu at screen coords (%i, %i)\n", pt.x, pt.y);
}

void MaterialProperties::OnSize(wxSizeEvent& e)
{
	properties->SetSize(GetClientRect());
}

void MaterialProperties::OnClose(wxCloseEvent& e)
{
}

void MaterialProperties::OnTest(wxCommandEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
}

void MaterialProperties::SetActiveNode(MaterialNodeItemData* item)
{
	properties->Clear();

	if (item)
	{
		materialNode = item;

		switch (item->type)
		{
		case MaterialNodeItemData::TypeMaterial:
			{
				//properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Material"))->SetFlag(wxPG_PROP_DISABLED);
				properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Material"));
				properties->Append(new wxStringProperty("Name", wxPG_LABEL, item->GetName()));
			}
			break;
		case MaterialNodeItemData::TypeShader:
			{
				wxString shaderType = "Unknown Shader";
				switch (reinterpret_cast<DKMaterial::ShaderSource*>(item->GetData())->type)
				{
				case DKShader::TypeVertexShader:	shaderType = "Vertex Shader";		break;
				case DKShader::TypeFragmentShader:	shaderType = "Fragment Shader";		break;
				case DKShader::TypeGeometryShader:	shaderType = "Geometry Shader";		break;
				}
				//properties->Append(new wxStringProperty("Class", wxPG_LABEL, shaderType))->SetFlag(wxPG_PROP_DISABLED);
				properties->Append(new wxStringProperty("Class", wxPG_LABEL, shaderType));
				properties->Append(new wxStringProperty("Name", wxPG_LABEL, item->GetName()));
			}
			break;
		case MaterialNodeItemData::TypeSampler:
			{
				//properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Texture"))->SetFlag(wxPG_PROP_DISABLED);
				properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Texture"));
				properties->Append(new wxStringProperty("Name", wxPG_LABEL, item->GetName()));
			}
			break;
		case MaterialNodeItemData::TypeUniform:
			{
				//properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Uniform"))->SetFlag(wxPG_PROP_DISABLED);
				properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Uniform"));
				properties->Append(new wxStringProperty("Name", wxPG_LABEL, item->GetName()));

				wxArrayString	uniforms;
				wxArrayInt		values;
				for (int i = 0; i < DKShaderConstant::UniformMax; i++)
				{
					uniforms.Add((const wchar_t*)DKShaderConstant::UniformToString(static_cast<DKShaderConstant::Uniform>(i)));
					values.Add(i);
				}
				properties->Append(new wxEnumProperty("Uniform", wxPG_LABEL, uniforms, values, (int)reinterpret_cast<DKMaterial::ShadingProperty*>(item->GetData())->id));
			}
			break;
		case MaterialNodeItemData::TypeStream:
			{
				//properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Attribute"))->SetFlag(wxPG_PROP_DISABLED);
				properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Attribute"));
				properties->Append(new wxStringProperty("Name", wxPG_LABEL, item->GetName()));

				wxArrayString	streams;
				wxArrayInt		values;
				for (int i = 0; i < DKVertexStream::StreamMax; i++)
				{
					streams.Add((const wchar_t*)DKVertexStream::StreamToString(static_cast<DKVertexStream::Stream>(i)));
					values.Add(i);
				}
				properties->Append(new wxEnumProperty("Stream", wxPG_LABEL, streams, values, (int)reinterpret_cast<DKMaterial::StreamProperty*>(item->GetData())->id));
			}
			break;
		case MaterialNodeItemData::TypeProgram:
			{
				//properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Program"))->SetFlag(wxPG_PROP_DISABLED);
				properties->Append(new wxStringProperty("Class", wxPG_LABEL, "Program"));
				properties->Append(new wxStringProperty("Name", wxPG_LABEL, item->GetName()));

				wxArrayString	depthFuncs;
				wxArrayInt		values;

				depthFuncs.Add("Never");			values.Add((int)DKMaterial::RenderingProperty::DepthFuncNever);
				depthFuncs.Add("Always");			values.Add((int)DKMaterial::RenderingProperty::DepthFuncAlways);
				depthFuncs.Add("Less");				values.Add((int)DKMaterial::RenderingProperty::DepthFuncLess);
				depthFuncs.Add("LessEqual");		values.Add((int)DKMaterial::RenderingProperty::DepthFuncLessEqual);
				depthFuncs.Add("Equal");			values.Add((int)DKMaterial::RenderingProperty::DepthFuncEqual);
				depthFuncs.Add("Greater");			values.Add((int)DKMaterial::RenderingProperty::DepthFuncGreater);
				depthFuncs.Add("GreaterEqual");		values.Add((int)DKMaterial::RenderingProperty::DepthFuncGreaterEqual);
				depthFuncs.Add("NotEqual");			values.Add((int)DKMaterial::RenderingProperty::DepthFuncNotEqual);

				properties->Append(new wxEnumProperty("DepthFunc", wxPG_LABEL, depthFuncs, values, (int)reinterpret_cast<DKMaterial::RenderingProperty*>(item->GetData())->depthFunc));
				properties->Append(new wxBoolProperty("DepthWrite", wxPG_LABEL, reinterpret_cast<DKMaterial::RenderingProperty*>(item->GetData())->depthWrite));
			}
			break;
		default:
			PrintLog(wxString::Format("[%s] Unknown Type:0x%x.\n", DKLIB_FUNCTION_NAME, item->type), wxColour(255,0,0));
			materialNode = NULL;
			break;
		}
	}
	else
	{
		PrintLog(wxString::Format("[%s] cleared.\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
		materialNode = NULL;
	}
}

void MaterialProperties::OnPropertyChanging(wxPropertyGridEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));
	// 이름 변경 확인
	wxPGProperty* prop = e.GetProperty();
	wxString label = prop->GetLabel();

	if (label.CmpNoCase("Name") == 0)
	{
		wxString oldName = prop->GetValue();
		wxString newName = e.GetValue();

		if (newName.IsNull() || newName.IsEmpty() || newName == materialNode->GetName())
		{
			e.Veto();
			return;
		}

		switch (materialNode->type)
		{
		case MaterialNodeItemData::TypeSampler:		// 텍스쳐는 이름이 유니크 해야함.
			if (GetMaterial()->samplerProperties.Find((const wchar_t*)newName))
			{
				wxMessageDialog dlg(this, "already exist!", "name conflict", wxOK | wxICON_ERROR );
				dlg.ShowModal();
				e.Veto();
				return;
			}
			break;		
		case MaterialNodeItemData::TypeUniform:		// 유니폼은 이름이 유니크 해야함.
			if (GetMaterial()->shadingProperties.Find((const wchar_t*)newName))
			{
				wxMessageDialog dlg(this, "already exist!", "name conflict", wxOK | wxICON_ERROR );
				dlg.ShowModal();
				e.Veto();
				return;
			}
			break;		
		case MaterialNodeItemData::TypeStream:		// 애트리뷰트는 이름이 유니크 해야함.
			if (GetMaterial()->streamProperties.Find((const wchar_t*)newName))
			{
				wxMessageDialog dlg(this, "already exist!", "name conflict", wxOK | wxICON_ERROR );
				dlg.ShowModal();
				e.Veto();
				return;
			}
			break;		
		}
	}
}

void MaterialProperties::OnPropertyChanged(wxPropertyGridEvent& e)
{
	PrintLog(wxString::Format("[%s]\n", DKLIB_FUNCTION_NAME), wxColour(255,0,0));

	// 이름 변경 확인
	wxPGProperty* prop = e.GetProperty();
	wxString label = prop->GetLabel();

	if (label.CmpNoCase("Name") == 0)
	{
		wxString value = prop->GetValue();
		if (!materialNode->SetName(value))
		{
			wxMessageDialog dlg(this, "Unable to set object name.", "Set Name Errorn", wxOK | wxICON_ERROR );
			dlg.ShowModal();
			e.Veto();
			return;
		}
	}
	else
	{
		switch (materialNode->type)
		{
		case MaterialNodeItemData::TypeMaterial:
			break;	
		case MaterialNodeItemData::TypeShader:
			break;		
		case MaterialNodeItemData::TypeSampler:
			break;		
		case MaterialNodeItemData::TypeUniform:
			if (label.CmpNoCase("Uniform") == 0)
			{
				long value = prop->GetValue();
				reinterpret_cast<DKMaterial::ShadingProperty*>(materialNode->GetData())->id = DKShaderConstant::Uniform(value);
			}
			break;		
		case MaterialNodeItemData::TypeStream:
			if (label.CmpNoCase("Stream") == 0)
			{
				long value = prop->GetValue();
				reinterpret_cast<DKMaterial::StreamProperty*>(materialNode->GetData())->id = DKVertexStream::Stream(value);
			}
			break;		
		case MaterialNodeItemData::TypeProgram:
			if (label.CmpNoCase("DepthFunc") == 0)
			{
				long value = prop->GetValue();
				reinterpret_cast<DKMaterial::RenderingProperty*>(materialNode->GetData())->depthFunc = (DKMaterial::RenderingProperty::DepthFunc)value;
			}
			else if (label.CmpNoCase("DepthWrite") == 0)
			{
				reinterpret_cast<DKMaterial::RenderingProperty*>(materialNode->GetData())->depthWrite = (bool)prop->GetValue();
			}
			break;		
		}
	}

	UpdateTreeItem(materialNode->GetId());

	if (materialNode->type == MaterialNodeItemData::TypeShader)
		UpdateEditor(reinterpret_cast<DKMaterial::ShaderSource*>(materialNode->GetData()));
}

