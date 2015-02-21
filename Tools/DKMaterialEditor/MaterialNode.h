#pragma once
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>

#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;


DKMaterial* GetMaterial(void);


////////////////////////////////////////////////////////////////////////////////
// 트리 아이템 (MaterialTree 와 MaterialProperties 에서 참조)
// MaterialTree 에서 트리 노드 데이터로 생성 및 소거하며
// MaterialProperties 에서는 참조만 한다.

class MaterialNodeItemData : public wxTreeItemData
{
public:
	enum Type
	{
		TypeMaterial = 0,			// 글로벌 머티리얼
		TypeShader,					// 쉐이더 (머티리얼, 렌더링 프로퍼티)
		TypeSampler,				// 텍스쳐 (머티리얼)
		TypeUniform,				// 쉐이딩 프로퍼티 (머티리얼)
		TypeStream,					// 스트림 소스
		TypeProgram,				// 렌더링 프로퍼티 (머티리얼)
		TypeMax,
	};
	MaterialNodeItemData(Type t) : type(t) {}
	const Type type;
	virtual const wchar_t* GetName(void) const = 0;
	virtual bool SetName(const wchar_t*) = 0;
	virtual wxColour GetItemColor(void) const = 0;
	virtual void* GetData(void) const = 0;
	virtual void SetData(void* data) = 0;
};

class MaterialNodeMaterialData : public MaterialNodeItemData
{
public:
	MaterialNodeMaterialData(DKMaterial* mat) : MaterialNodeItemData(TypeMaterial), data(mat) {}
	DKMaterial*	data;
	const wchar_t* GetName(void) const
	{
		return data->Name();
	}
	bool SetName(const wchar_t* name)
	{
		data->SetName(name);
		return true;
	}
	void* GetData(void) const	{return data;}
	void SetData(void* data)	{this->data = reinterpret_cast<DKMaterial*>(data);}

	wxColour GetItemColor(void) const			{return wxColour(0,0,0);}
};

class MaterialNodeProgramData : public MaterialNodeItemData
{
public:
	MaterialNodeProgramData(DKMaterial::RenderingProperty* prop) : MaterialNodeItemData(TypeProgram), data(prop) {}
	DKMaterial::RenderingProperty* data;
	const wchar_t* GetName(void) const
	{
		return data->name;
	}
	bool SetName(const wchar_t* name)
	{
		data->name = name;
		return true;
	}
	void* GetData(void) const	{return data;}
	void SetData(void* data)	{this->data = reinterpret_cast<DKMaterial::RenderingProperty*>(data);}

	wxColour GetItemColor(void) const			{return wxColour(0,0,255);}
};

class MaterialNodeUniformData : public MaterialNodeItemData
{
public:
	MaterialNodeUniformData(const wchar_t* n, DKMaterial::ShadingProperty* prop) : MaterialNodeItemData(TypeUniform), name(n), data(prop) {}
	DKString name;
	DKMaterial::ShadingProperty* data;
	const wchar_t* GetName(void) const
	{
		wxASSERT(GetMaterial()->shadingProperties.Find(name));
		wxASSERT(data == &GetMaterial()->shadingProperties.Find(name)->value);
		return name;
	}
	bool SetName(const wchar_t* name)
	{
		if (GetMaterial()->shadingProperties.Find(name))
			return false;
		if (GetMaterial()->shadingProperties.Insert(name, *data))
		{
			GetMaterial()->shadingProperties.Remove(this->name);
			data = &GetMaterial()->shadingProperties.Value(name);
			this->name = name;
			return true;
		}
		return false;
	}
	void* GetData(void) const	{return data;}
	void SetData(void* data)	{this->data = reinterpret_cast<DKMaterial::ShadingProperty*>(data);}

	wxColour GetItemColor(void) const			{return wxColour(255,0,0);}
};

class MaterialNodeStreamData : public MaterialNodeItemData
{
public:
	MaterialNodeStreamData(const wchar_t* n, DKMaterial::StreamProperty* prop) : MaterialNodeItemData(TypeStream), name(n), data(prop) {}
	DKString name;
	DKMaterial::StreamProperty* data;
	const wchar_t* GetName(void) const
	{
		wxASSERT(GetMaterial()->streamProperties.Find(name));
		wxASSERT(data == &GetMaterial()->streamProperties.Find(name)->value);
		return name;
	}
	bool SetName(const wchar_t* name)
	{
		if (GetMaterial()->streamProperties.Find(name))
			return false;
		if (GetMaterial()->streamProperties.Insert(name, *data))
		{
			GetMaterial()->streamProperties.Remove(this->name);
			data = &GetMaterial()->streamProperties.Value(name);
			this->name = name;
			return true;
		}
		return false;
	}
	void* GetData(void) const	{return data;}
	void SetData(void* data)	{this->data = reinterpret_cast<DKMaterial::StreamProperty*>(data);}

	wxColour GetItemColor(void) const			{return wxColour(0,0,255);}
};

class MaterialNodeShaderData : public MaterialNodeItemData
{
public:
	MaterialNodeShaderData(DKMaterial::ShaderSource* src) : MaterialNodeItemData(TypeShader), data(src) {}
	DKMaterial::ShaderSource* data;
	const wchar_t* GetName(void) const
	{
		return data->name;
	}
	bool SetName(const wchar_t* name)
	{
		data->name = name;
		return true;
	}
	void* GetData(void) const	{return data;}
	void SetData(void* data)	{this->data = reinterpret_cast<DKMaterial::ShaderSource*>(data);}

	wxColour GetItemColor(void) const			{return wxColour(200,0,200);}
};

class MaterialNodeSamplerData : public MaterialNodeItemData
{
public:
	MaterialNodeSamplerData(DKMaterial::SamplerProperty* p) : MaterialNodeItemData(TypeSampler), data(p) {}
	DKString name;
	DKMaterial::SamplerProperty* data;
	const wchar_t* GetName(void) const
	{
		wxASSERT(GetMaterial()->samplerProperties.Find(name));
		wxASSERT(data == &GetMaterial()->samplerProperties.Find(name)->value);
		return name;
	}
	bool SetName(const wchar_t* name)
	{
		if (GetMaterial()->samplerProperties.Find(name))
			return false;
		if (GetMaterial()->samplerProperties.Insert(name, *data))
		{
			GetMaterial()->samplerProperties.Remove(this->name);
			data = &GetMaterial()->samplerProperties.Value(name);
			this->name = name;
			return true;
		}
		return false;
	}
	void* GetData(void) const	{return data;}
	void SetData(void* data)	{this->data = reinterpret_cast<DKMaterial::SamplerProperty*>(data);}

	wxColour GetItemColor(void) const			{return wxColour(0,100,0);}
};
