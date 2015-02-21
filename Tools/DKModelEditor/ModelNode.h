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

DKModel* GetModel(void);


////////////////////////////////////////////////////////////////////////////////
// 트리 아이템 (ModelTree 와 ModelProperties 에서 참조함)
// ModelTree 에서는 트리 노드 아이템의 데이터로 사용 및 객체 생성, 제거함
// ModelProperties 에서는 참조만 한다.
//
// TypeModel: 모델 관련 속성 및 물리객체 속성을 제어를 한다. (이름, 무게, rigid,soft형식 등등)
// TypeMesh: 매시 관련된 제어만 함. 버퍼합치기 나누기 스트림, 머티리얼 편집 등
// TypeShape: DKCollisionShape 의 속성을 편집 (위치, 각도, 스케일 등등)
// TypeConstraint: DKConstraint 의 속성을 편집 (트랜스폼, 각도limit 등)
//
// 주의:
//    1. Constraint 는 Rigid 객체들끼리만 사용가능함.
//    2. Constraint 의 bodyA 는 부모객체중 Rigid 한것
//    3. Constraint 의 bodyB 는 현재 객체 (Rigid 여야만 한다)
//    4. 시뮬레이션 환경에서는 Constraint 에서 정의된 트랜스폼이 가장 우선임.

class ModelNodeModelData;
class ModelNodeItemData : public wxTreeItemData
{
public:
	enum NodeType
	{
		TypeModel = 0,			// DKModel, dummy model
		TypeRigidBody,			// DKRigidBody
		TypeSoftBody,			// DKSoftBody
		TypeMesh,				// DKMesh
		TypeConstraint,			// DKConstraints
		TypeShape,				// DKCollisionShape (RigidBody 의 자식노드)
		TypeMax,
	};
	ModelNodeItemData(NodeType t, ModelNodeModelData* p) : type(t), parent(p) {}

	const NodeType type;
	ModelNodeModelData* parent;

	virtual DKModel* GetModel(void)					{return NULL;}

	virtual wxColour GetItemColor(void) const = 0;
	virtual const wchar_t* GetName(void) const = 0;
	virtual void SetName(const wchar_t* name) = 0;
	virtual bool IsNameEditable(void) const = 0;
	virtual DKNSTransform Transform(void) const = 0;		// Local-Transform
	virtual void SetTransform(const DKNSTransform& t) = 0;
};

class ModelNodeModelData : public ModelNodeItemData
{
public:
	ModelNodeModelData(DKModel* m, ModelNodeModelData* p)
		: ModelNodeItemData(TypeModel, p), model(m)
	{
		wxASSERT( model );
	}
	ModelNodeModelData(NodeType t, DKModel* m, ModelNodeModelData* p)
		: ModelNodeItemData(t, p), model(m)
	{
		wxASSERT( model );
	}

	DKObject<DKModel> model;
	DKModel* GetModel(void)						{return model;}

	const wchar_t* GetName(void) const			{return model->Name();}
	void SetName(const wchar_t* name)			{model->SetName(name);}
	bool IsNameEditable(void) const				{return true;}
	wxColour GetItemColor(void) const			{return wxColour(0,0,0);}
	DKNSTransform Transform(void) const			{return model->LocalTransform();}
	void SetTransform(const DKNSTransform& t)	{model->SetLocalTransform(t);}
};

class ModelNodeRigidBodyData : public ModelNodeModelData
{
public:
	ModelNodeRigidBodyData(DKRigidBody* rb, ModelNodeModelData* p)
		: ModelNodeModelData(TypeRigidBody, rb, p), body(rb)
	{
		wxASSERT( body );
	}

	DKObject<DKRigidBody> body;

	wxColour GetItemColor(void) const		{return wxColour(255,0,255);}
};

class ModelNodeSoftBodyData : public ModelNodeModelData
{
public:
	ModelNodeSoftBodyData(DKSoftBody* rb, ModelNodeModelData* p)
		: ModelNodeModelData(TypeSoftBody, rb, p), body(rb)
	{
		wxASSERT( body );
	}

	DKObject<DKSoftBody> body;

	wxColour GetItemColor(void) const		{return wxColour(255,0,255);}
};

class ModelNodeMeshData : public ModelNodeModelData
{
public:
	ModelNodeMeshData(DKMesh* m, ModelNodeModelData* p)
		: ModelNodeModelData(TypeMesh, m, p), mesh(m), scale(m->Scale())
	{
		wxASSERT( mesh );
	}
	
	DKObject<DKMesh> mesh;
	DKVector3 scale;		// local-scale

	wxColour GetItemColor(void) const		{return wxColour(0,0,255);}
};

class ModelNodeShapeData : public ModelNodeItemData
{
public:
	ModelNodeShapeData(DKCollisionShape* s, const DKNSTransform& trans, ModelNodeModelData* p)
		: ModelNodeItemData(TypeShape, p), shape(s), transform(trans)
	{
		wxASSERT( parent != NULL );
		wxASSERT( shape != NULL );
	}

	DKObject<DKCollisionShape> shape;
	DKNSTransform transform;			// local-transform (해당 model로부터의)

	const wchar_t* GetName(void) const
	{
		wxASSERT(shape);
		switch (shape->type)
		{
		case DKCollisionShape::ShapeType::Box:					return L"box";
		case DKCollisionShape::ShapeType::Capsule:				return L"capsule";
		case DKCollisionShape::ShapeType::Cylinder:				return L"cylinder";
		case DKCollisionShape::ShapeType::Cone:					return L"cone";
		case DKCollisionShape::ShapeType::Sphere:				return L"sphere";
		case DKCollisionShape::ShapeType::MultiSphere:			return L"multi sphere";
		case DKCollisionShape::ShapeType::ConvexHull:			return L"convex hull";
		case DKCollisionShape::ShapeType::StaticPlane:			return L"static plane";
		case DKCollisionShape::ShapeType::StaticTriangleMesh:	return L"static triangle";
		case DKCollisionShape::ShapeType::Compound:				return L"compound";
		case DKCollisionShape::ShapeType::Empty:				return L"empty";
		case DKCollisionShape::ShapeType::Custom:				return L"custom";
		}
		return L"unknown";
	}
	void SetName(const wchar_t* name)			{wxFAIL_MSG("Invalid Node!");}
	bool IsNameEditable(void) const				{return false;}
	wxColour GetItemColor(void) const			{return wxColour(255,0,0);}
	DKNSTransform Transform(void) const			{return transform;}
	void SetTransform(const DKNSTransform& t)	{transform = t;}
};

class ModelNodeConstraintData : public ModelNodeItemData
{
public:
	ModelNodeConstraintData(DKConstraint* con, const DKNSTransform& trans, ModelNodeModelData* p)
		: ModelNodeItemData(TypeConstraint, p), constraint(con), transform(trans)
	{
		wxASSERT( parent != NULL );
		wxASSERT( constraint != NULL );
	}

	DKObject<DKConstraint> constraint;
	DKNSTransform transform;

	const wchar_t* GetName(void) const
	{
		switch (constraint->type)
		{
		case DKConstraint::LinkType::Fixed:					return L"fixed";
		case DKConstraint::LinkType::Gear:					return L"gear";
		case DKConstraint::LinkType::Point2Point:			return L"p2p";
		case DKConstraint::LinkType::Hinge:					return L"hinge";
		case DKConstraint::LinkType::ConeTwist:				return L"cone twist";
		case DKConstraint::LinkType::Generic6Dof:			return L"g6dof";
		case DKConstraint::LinkType::Generic6DofSpring:		return L"g6dof spring";
		case DKConstraint::LinkType::Slider:				return L"slider";
		case DKConstraint::LinkType::Custom:				return L"custom";
		}
		return L"unknown";
	}
	void SetName(const wchar_t* name)			{wxFAIL_MSG("Invalid Node!");}
	bool IsNameEditable(void) const				{return false;}
	wxColour GetItemColor(void) const			{return wxColour(255,0,0);}
	DKNSTransform Transform(void) const			{return transform;}
	void SetTransform(const DKNSTransform& t)	{transform = t;}
};
