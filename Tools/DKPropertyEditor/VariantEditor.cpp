#include "stdafx.h"
#include "VariantEditor.h"
#include <wx/clipbrd.h>
#include <wx/calctrl.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <string.h>

wxDEFINE_EVENT(VARIANT_EDITOR_MODIFIED, VariantEditorEvent);

////////////////////////////////////////////////////////////////////////////////
#pragma mark - VariantNode
class VariantNode
{
public:
	enum VType
	{
		VTypeUndefined = 0,
		VTypeInteger,
		VTypeFloat,
		VTypeVector2,
		VTypeVector2X,
		VTypeVector2Y,
		VTypeVector3,
		VTypeVector3X,
		VTypeVector3Y,
		VTypeVector3Z,
		VTypeVector4,
		VTypeVector4X,
		VTypeVector4Y,
		VTypeVector4Z,
		VTypeVector4W,
		VTypeMatrix2,
		VTypeMatrix20,
		VTypeMatrix21,
		VTypeMatrix22,
		VTypeMatrix23,
		VTypeMatrix3,
		VTypeMatrix30,
		VTypeMatrix31,
		VTypeMatrix32,
		VTypeMatrix33,
		VTypeMatrix34,
		VTypeMatrix35,
		VTypeMatrix36,
		VTypeMatrix37,
		VTypeMatrix38,
		VTypeMatrix4,
		VTypeMatrix40,
		VTypeMatrix41,
		VTypeMatrix42,
		VTypeMatrix43,
		VTypeMatrix44,
		VTypeMatrix45,
		VTypeMatrix46,
		VTypeMatrix47,
		VTypeMatrix48,
		VTypeMatrix49,
		VTypeMatrix4A,
		VTypeMatrix4B,
		VTypeMatrix4C,
		VTypeMatrix4D,
		VTypeMatrix4E,
		VTypeMatrix4F,
		VTypeQuaternion,
		VTypeQuaternionX,
		VTypeQuaternionY,
		VTypeQuaternionZ,
		VTypeQuaternionW,
		VTypeRational,
		VTypeString,
		VTypeDateTime,
		VTypeData,
		VTypeArray,
		VTypePairs,
	};

	VariantNode(VariantNode* p, const DKString& k)
		: parent(p), key(k), type(VTypeUndefined), stringValue(L""), dataValue(NULL)
	{
	}
	VariantNode(VariantNode* p, const DKString& k, const DKVariant& v)
		: parent(p), key(k), type(VTypeUndefined), stringValue(L""), dataValue(NULL)
	{
		SetVariant(k, v);
	}
	VariantNode(VariantNode* p, const DKString& k, VType t)
		: parent(p), key(k), type(t), stringValue(L""), dataValue(NULL)
	{
	}
	~VariantNode(void)
	{
		for (size_t i = 0; i < children.Count(); ++i)
		{
			VariantNode* node = children.Value(i);
			delete node;
		}
	}
	void SetVariant(const DKString& k, const DKVariant& v)
	{
		for (size_t i = 0; i < children.Count(); ++i)
		{
			VariantNode* node = children.Value(i);
			delete node;
		}
		children.Clear();

		this->type = VTypeUndefined;
		this->numericValue.SetZero();
		this->stringValue = L"";
		this->dataValue = NULL;

		const DKString vectorMemberKey[] = {L"X", L"Y", L"Z", L"W"};
		const DKString matrixMemberKey[] = {
			L"Row.1 Col.1", L"Row.1 Col.2", L"Row.1 Col.3", L"Row.1 Col.4",
			L"Row.2 Col.1", L"Row.2 Col.2", L"Row.2 Col.3", L"Row.2 Col.4",
			L"Row.3 Col.1", L"Row.3 Col.2", L"Row.3 Col.3", L"Row.3 Col.4",
			L"Row.4 Col.1", L"Row.4 Col.2", L"Row.4 Col.3", L"Row.4 Col.4"
		};

		this->key = k;
		switch (v.ValueType())
		{
		case DKVariant::TypeInteger:
			this->type = VTypeInteger;
			this->numericValue.Integer() = v.Integer();
			break;
		case DKVariant::TypeFloat:
			this->type = VTypeFloat;
			this->numericValue.Real() = v.Float();
			break;
		case DKVariant::TypeVector2:
			this->type = VTypeVector2;
			for (size_t i = 0; i < 2; ++i)
			{
				VariantNode* node = new VariantNode(this, vectorMemberKey[i], static_cast<VType>(VTypeVector2 + i + 1));
				node->numericValue.Real() = v.Vector2().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeVector3:
			this->type = VTypeVector3;
			for (size_t i = 0; i < 3; ++i)
			{
				VariantNode* node = new VariantNode(this, vectorMemberKey[i], static_cast<VType>(VTypeVector3 + i + 1));
				node->numericValue.Real() = v.Vector3().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeVector4:
			this->type = VTypeVector4;
			for (size_t i = 0; i < 4; ++i)
			{
				VariantNode* node = new VariantNode(this, vectorMemberKey[i], static_cast<VType>(VTypeVector4 + i + 1));
				node->numericValue.Real() = v.Vector4().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeMatrix2:
			this->type = VTypeMatrix2;
			for (size_t i = 0; i < 4; ++i)
			{
				VariantNode* node = new VariantNode(this, matrixMemberKey[(i/2)*4 + i%2], static_cast<VType>(VTypeMatrix2 + i + 1));
				node->numericValue.Real() = v.Matrix2().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeMatrix3:
			this->type = VTypeMatrix3;
			for (size_t i = 0; i < 9; ++i)
			{
				VariantNode* node = new VariantNode(this, matrixMemberKey[(i/3)*4 + i%3], static_cast<VType>(VTypeMatrix3 + i + 1));
				node->numericValue.Real() = v.Matrix3().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeMatrix4:
			this->type = VTypeMatrix4;
			for (size_t i = 0; i < 16; ++i)
			{
				VariantNode* node = new VariantNode(this, matrixMemberKey[i], static_cast<VType>(VTypeMatrix4 + i + 1));
				node->numericValue.Real() = v.Matrix4().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeQuaternion:
			this->type = VTypeQuaternion;
			for (size_t i = 0; i < 4; ++i)
			{
				VariantNode* node = new VariantNode(this, vectorMemberKey[i], static_cast<VType>(VTypeQuaternion + i + 1));
				node->numericValue.Real() = v.Quaternion().val[i];
				children.Add(node);
			}
			break;
		case DKVariant::TypeRational:
			this->type = VTypeRational;
			this->numericValue.SetRational(v.Rational());
			break;
		case DKVariant::TypeString:
			this->type = VTypeString;
			this->stringValue = v.String();
			break;
		case DKVariant::TypeDateTime:
			this->type = VTypeDateTime;
			this->numericValue.SetDateTime(v.DateTime());
			break;	
		case DKVariant::TypeData:
			this->type = VTypeData;
			if (v.Data().Length() > 0)
				this->dataValue = DKBuffer::Create(&v.Data()).SafeCast<DKData>();
			else
				this->dataValue = NULL;
			break;
		case DKVariant::TypeArray:
			this->type = VTypeArray;
			for (size_t i = 0; i < v.Array().Count(); ++i)
			{
				VariantNode* node = new VariantNode(this, DKString::Format("[%llu]", i), v.Array().Value(i));
				children.Add(node);
			}
			break;
		case DKVariant::TypePairs:
			this->type = VTypePairs;
			if (v.Pairs().Count() > 0)
			{
				v.Pairs().EnumerateForward([&](const DKVariant::VPairs::Pair& pair)
										   {
											   VariantNode* child = new VariantNode(this, pair.key);
											   child->SetVariant(pair.key, pair.value);
											   children.Add(child);
										   });
				this->SortChildren();
			}
			break;
		default:
			this->type = VTypeUndefined;
			break;
		}
	}
	VariantNode* AddItem(const DKVariant& v, wxDataViewModel* model)
	{
		wxASSERT(this->type == VTypeArray);

		VariantNode* node = new VariantNode(this, DKString::Format("[%llu]", this->children.Count()), v);
		this->children.Add(node);

		model->ItemAdded(wxDataViewItem(this), wxDataViewItem(node));
		return node;
	}
	VariantNode* AddItem(const DKString& key, const DKVariant& v, wxDataViewModel* model)
	{
		wxASSERT(this->type == VTypePairs);

		for (size_t i = 0; i < this->children.Count(); ++i)
		{
			if (this->children.Value(i)->key.Compare(key) == 0)
				return NULL;
		}

		VariantNode* node = new VariantNode(this, key, v);
		this->children.Add(node);
		this->SortChildren();

		model->ItemAdded(wxDataViewItem(this), wxDataViewItem(node));
		return node;
	}
	void ReorderItem(size_t indexOld, size_t indexNew, wxDataViewModel* model)
	{
		wxASSERT(this->type == VTypeArray);
		wxASSERT(indexOld < this->children.Count());
		wxASSERT(indexNew < this->children.Count());
		if (indexOld == indexNew)
			return;

		VariantNode* node = this->children.Value(indexOld);
		this->children.Remove(indexOld);
		this->children.Insert(node, indexNew);

		wxDataViewItemArray itemsChanged;

		size_t indexBegin = Min<size_t>(indexOld, indexNew);
		size_t indexEnd = Max<size_t>(indexOld, indexNew);

		for (size_t i = indexBegin; i <= indexEnd; ++i)
		{
			VariantNode* node = this->children.Value(i);
			node->key = DKString::Format("[%llu]", i);
			itemsChanged.Add(wxDataViewItem(node));
		}

		if (itemsChanged.Count() > 0)
			model->ItemsChanged(itemsChanged);
	}
	bool RemoveItem(const DKString& key, wxDataViewModel* model)
	{
		wxASSERT(this->type == VTypePairs);

		for (size_t i = 0; i < this->children.Count(); ++i)
		{
			VariantNode* node = this->children.Value(i);
			if (node->key.Compare(key) == 0)
			{
				this->children.Remove(i);
				node->parent = NULL;

				model->ItemDeleted(wxDataViewItem(this), wxDataViewItem(node));
				delete node;
				return true;
			}
		}

		return false;
	}
	bool RemoveItem(int index, wxDataViewModel* model)
	{
		wxASSERT(this->type == VTypeArray);
		wxASSERT(index < this->children.Count());

		VariantNode* node = this->children.Value(index);
		this->children.Remove(index);
		node->parent = NULL;

		model->ItemDeleted(wxDataViewItem(this), wxDataViewItem(node));
		delete node;
		return true;
	}
	void SortChildren(void)
	{
		wxASSERT(this->type == VTypePairs);
		struct VariantNodeKeyCmp
		{
			static bool KeyCompare(const VariantNode* lhs, const VariantNode* rhs)
			{
				int c = lhs->key.CompareNoCase(rhs->key);
				if (c == 0)
					c = lhs->key.Compare(rhs->key);
				return c < 0;
			}
		};
		this->children.Sort(VariantNodeKeyCmp::KeyCompare);
	}
	void Revert(const VariantNode* node, wxDataViewModel* model)
	{
		wxASSERT(node);
		wxASSERT(model);

		bool changed = false;
		this->key = node->key;

		wxDataViewItemArray itemsAdded;
		wxDataViewItemArray itemsRemoved;

		if (changed == false)
		{
			changed =
					this->key.Compare(node->key) ||
					this->type != node->type ||
					this->stringValue != node->stringValue ||
					this->dataValue != node->dataValue ||
					this->numericValue != node->numericValue;
		}

		// 2012-2-27 : 객체가 먼저 삭제된 후 model 에 통보하면 오류가 나는 버그가 있음.
		DKArray<VariantNode*> removedChildren;		// 나중에 지우기 위해 잠시 보관용 컨테이너
		removedChildren.Reserve(this->children.Count());

		if (node->children.Count() > 0)
		{
			typedef DKMap<DKString, VariantNode*> VariantNodeMap;
			VariantNodeMap childrenMap;
			for (size_t i = 0; i < this->children.Count(); ++i)
			{
				if (childrenMap.Insert(children.Value(i)->key, children.Value(i)) == false)
				{
					wxFAIL_MSG("Duplicated children found!");
				}
			}
			children.Clear();

			// 재사용 가능한 노드 가져오기 (같은 이름, 같은 타입을 가진 노드)
			for (size_t i = 0; i < node->children.Count(); ++i)
			{
				const DKString& key = node->children.Value(i)->key;
				VariantNode* child = NULL;
				VariantNodeMap::Pair* p = childrenMap.Find(key);
				if (p && p->value->type == node->children.Value(i)->type)
				{
					child = p->value;
					child->Revert(node->children.Value(i), model);
					childrenMap.Remove(key);
				}
				else
				{
					child = node->children.Value(i)->Duplicate(this);
					itemsAdded.Add(wxDataViewItem(child));	
				}
				children.Add(child);
			}
			// 필요없는 노드 제거
			childrenMap.EnumerateForward([&](VariantNodeMap::Pair& pair)
										 {
											 VariantNode* child = pair.value;
											 itemsRemoved.Add(wxDataViewItem(child));
											 child->parent = NULL;
											 removedChildren.Add(child);
										 });
		}
		else
		{
			for (size_t i = 0; i < children.Count(); ++i)
			{
				VariantNode* child = children.Value(i);
				itemsRemoved.Add(wxDataViewItem(child));
				child->parent = NULL;
				removedChildren.Add(child);
			}
			children.Clear();
		}

		if (itemsRemoved.Count() > 0)
			model->ItemsDeleted(wxDataViewItem(this), itemsRemoved);
		if (itemsAdded.Count() > 0)
			model->ItemsAdded(wxDataViewItem(this), itemsAdded);

		if (changed)
		{
			this->key = node->key;
			this->type = node->type;
			this->numericValue = node->numericValue;
			this->stringValue = node->stringValue;
			this->dataValue = node->dataValue;
			model->ItemChanged(wxDataViewItem(this));
		}

		for (size_t i = 0; i < removedChildren.Count(); ++i)
		{
			VariantNode* child = removedChildren.Value(i);
			delete child;
		}
	}
	VariantNode* Duplicate(VariantNode* parent) const
	{
		VariantNode* node = new VariantNode(parent, this->key);
		node->type = this->type;
		node->numericValue = this->numericValue;
		node->stringValue = this->stringValue;
		node->dataValue = this->dataValue;
		node->children.Reserve(this->children.Count());
		for (size_t i = 0; i < this->children.Count(); ++i)
		{
			node->children.Add(this->children.Value(i)->Duplicate(node));
		}
		return node;
	}
	DKVariant GetVariant(void) const
	{
		DKVariant v(DKVariant::TypeUndefined);

		switch (this->type)
		{
		case VTypeInteger:
			wxASSERT_MSG(children.Count() == 0, "this node cannot have children");
			v.SetValueType(DKVariant::TypeInteger);
			v.Integer() = this->numericValue.Integer();
			break;
		case VTypeFloat:
			wxASSERT_MSG(children.Count() == 0, "this node cannot have children");
			v.SetValueType(DKVariant::TypeFloat);
			v.Float() = this->numericValue.Real();
			break;
		case VTypeVector2:
			wxASSERT_MSG(this->children.Count() == 2, "this node should have 2 children");
			v.SetValueType(DKVariant::TypeVector2);
			for (size_t i = 0; i < 2; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeVector2 + i + 1));
				v.Vector2().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeVector3:
			wxASSERT_MSG(this->children.Count() == 3, "this node should have 3 children");
			v.SetValueType(DKVariant::TypeVector3);
			for (size_t i = 0; i < 3; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeVector3 + i + 1));
				v.Vector3().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeVector4:
			wxASSERT_MSG(this->children.Count() == 4, "this node should have 4 children");
			v.SetValueType(DKVariant::TypeVector4);
			for (size_t i = 0; i < 4; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeVector4 + i + 1));
				v.Vector4().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeMatrix2:
			wxASSERT_MSG(this->children.Count() == 4, "this node should have 4 children");
			v.SetValueType(DKVariant::TypeMatrix2);
			for (size_t i = 0; i < 4; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeMatrix2 + i + 1));
				v.Matrix2().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeMatrix3:
			wxASSERT_MSG(this->children.Count() == 9, "this node should have 9 children");
			v.SetValueType(DKVariant::TypeMatrix3);
			for (size_t i = 0; i < 9; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeMatrix3 + i + 1));
				v.Matrix3().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeMatrix4:
			wxASSERT_MSG(this->children.Count() == 16, "this node should have 16 children");
			v.SetValueType(DKVariant::TypeMatrix4);
			for (size_t i = 0; i < 16; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeMatrix4 + i + 1));
				v.Matrix4().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeQuaternion:
			wxASSERT_MSG(this->children.Count() == 4, "this node should have 4 children");
			v.SetValueType(DKVariant::TypeQuaternion);
			for (size_t i = 0; i < 4; ++i)
			{
				wxASSERT(this->children.Value(i)->type == static_cast<VType>(VTypeQuaternion + i + 1));
				v.Quaternion().val[i] = this->children.Value(i)->numericValue.Real();
			}
			break;
		case VTypeRational:
			wxASSERT_MSG(children.Count() == 0, "this node cannot have children");
			v.SetValueType(DKVariant::TypeRational);
			v.Rational() = this->numericValue.Rational();
			break;
		case VTypeString:
			wxASSERT_MSG(children.Count() == 0, "this node cannot have children");
			v.SetValueType(DKVariant::TypeString);
			v.String() = this->stringValue;
			break;
		case VTypeDateTime:
			wxASSERT_MSG(children.Count() == 0, "this node cannot have children");
			v.SetValueType(DKVariant::TypeDateTime);
			v.DateTime() = this->numericValue.DateTime();
			break;			
		case VTypeData:
			wxASSERT_MSG(children.Count() == 0, "this node cannot have children");
			v.SetValueType(DKVariant::TypeData);
			v.Data().SetContent(this->dataValue);
			break;
		case VTypeArray:
			v.SetValueType(DKVariant::TypeArray);
			for (size_t i = 0; i < this->children.Count(); ++i)
			{
				v.Array().Add(this->children.Value(i)->GetVariant());
			}
			break;
		case VTypePairs:
			v.SetValueType(DKVariant::TypePairs);
			for (size_t i = 0; i < this->children.Count(); ++i)
			{
				const VariantNode* node = this->children.Value(i);
				if (v.Pairs().Insert(node->key, node->GetVariant()) == false)
				{
					wxFAIL_MSG("Duplicated key found!");
				}
			}
			break;
		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			v.SetValueType(DKVariant::TypeFloat);
			v.Float() = this->numericValue.Real();
			break;
		}
		return v;
	}
	bool IsEqual(const VariantNode* node) const
	{
		wxASSERT(node != NULL);

		if (this == node)
			return true;

		if (this->type == node->type &&
			this->key == node->key &&
			this->numericValue == node->numericValue &&
			this->stringValue == node->stringValue &&
			this->dataValue == node->dataValue &&
			this->children.Count() == node->children.Count())
		{
			for (size_t i = 0; i < this->children.Count(); ++i)
			{
				if (this->children.Value(i)->IsEqual(node->children.Value(i)) == false)
					return false;
			}
			return true;
		}
		return false;
	}
	bool Validate(void) const
	{
		bool canHaveNumericValue = false;
		bool canHaveStringValue = false;
		bool canHaveDataValue = false;
		int validNumberOfChildren = 0;		// 0 보다 작으면 임의의 개수를 가짐

		switch (this->type)
		{
		case VTypeUndefined:
			break;
		case VTypeInteger:
			canHaveNumericValue = true;
			break;
		case VTypeFloat:
			canHaveNumericValue = true;
			break;
		case VTypeVector2:
			validNumberOfChildren = 2;
			break;
		case VTypeVector3:
			validNumberOfChildren = 3;
			break;
		case VTypeVector4:
			validNumberOfChildren = 4;
			break;
		case VTypeMatrix2:
			validNumberOfChildren = 4;
			break;
		case VTypeMatrix3:
			validNumberOfChildren = 9;
			break;
		case VTypeMatrix4:
			validNumberOfChildren = 16;
			break;
		case VTypeQuaternion:
			validNumberOfChildren = 4;
			break;
		case VTypeRational:
			canHaveNumericValue = true;
			break;
		case VTypeString:
			canHaveStringValue = true;
			break;
		case VTypeDateTime:
			canHaveNumericValue = true;
			break;
		case VTypeData:
			canHaveDataValue = true;
			break;
		case VTypeArray:
			validNumberOfChildren = -1;
			for (size_t i = 0; i < children.Count(); ++i)
			{
				size_t index = static_cast<size_t>(children.Value(i)->key.ToUnsignedInteger());
				if (i != index)
				{
					wxFAIL_MSG("index mismatch!");
					return false;
				}
			}
			break;
		case VTypePairs:
			validNumberOfChildren = -1;
			break;
		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			canHaveNumericValue = true;
			break;
		default:
			wxFAIL_MSG("Unknown type!");
			break;
		}

		if (!canHaveNumericValue && !this->numericValue.IsZero())
		{
			wxFAIL_MSG("this type cannot have non-zero numericValue");
			return false;
		}
		if (!canHaveStringValue && stringValue.Length() > 0)
		{
			wxFAIL_MSG("this type cannot have stringValue");
			return false;
		}
		if (!canHaveDataValue && dataValue != NULL)
		{
			wxFAIL_MSG("this type cannot have dataValue");
			return false;
		}
		if (validNumberOfChildren)
		{
			if (validNumberOfChildren > 0 && children.Count() != validNumberOfChildren)
			{
				wxFAIL_MSG("this type has invalid children");
				return false;
			}
		}
		else if (children.Count() > 0)
		{
			wxFAIL_MSG("this type cannot have children");
			return false;
		}

		// 자식 노드 이름 검사.
		DKSet<DKString> childrenKeySet;
		for (size_t i = 0; i < children.Count(); ++i)
		{
			DKString key = children.Value(i)->key;
			if (key.Length() > 0)
			{
				if (childrenKeySet.Contains(key))
				{
					wxFAIL_MSG("Duplicated key!");
					return false;
				}
				childrenKeySet.Insert(key);
			}
			else
			{
				wxFAIL_MSG("Invalid key!");
				return false;
			}
		}
		for (size_t i = 0; i < children.Count(); ++i)
		{
			if (children.Value(i)->Validate() == false)
				return false;
		}
		return true;
	}

	bool SetKey(const DKString& k, wxDataViewModel* model)
	{
		wxASSERT(parent);
		wxASSERT(parent->type == VTypePairs);
		wxASSERT(model);

		if (IsKeyEditable() && k.Compare(this->key))
		{
			wxASSERT_MSG(parent->children.Count() != 0, "parent must have at least 1 child (self)");
			for (size_t i = 0; i < parent->children.Count(); ++i)
			{
				if (parent->children.Value(i)->key.Compare(k) == 0)
					return false;
			}
			this->key = k;
			return true;
		}
		return false;
	}

	bool SetType(DKVariant::Type t, wxDataViewModel* model)
	{
		wxASSERT(model);

		if (IsTypeEditable() && t != ValueType())
		{
			DKString k = this->key;
			wxString v = ValueString();

			// array <-> map 으로 변환할때는 자식노드들은 모두 재활용 한다.
			if ((this->type == VTypePairs && t == DKVariant::TypeArray) ||
				(this->type == VTypeArray && t == DKVariant::TypePairs))
			{
				wxDataViewItemArray itemsChanged;

				if (this->type == VTypeArray)	// array -> map
				{
					for (size_t i = 0; i < children.Count(); ++i)
					{
						VariantNode* child = this->children.Value(i);
						child->key = DKString::Format("item %llu", i);
						itemsChanged.Add(wxDataViewItem(child));
					}
					this->type = VTypePairs;
				}
				else		// map -> array
				{
					for (size_t i = 0; i < children.Count(); ++i)
					{
						VariantNode* child = this->children.Value(i);
						child->key = DKString::Format("[%llu]", i);
						itemsChanged.Add(wxDataViewItem(child));
					}
					this->type = VTypeArray;
				}

				if (itemsChanged.Count() > 0)
					model->ItemsChanged(itemsChanged);
			}
			else
			{
				wxDataViewItemArray itemsAdded;
				wxDataViewItemArray itemsRemoved;

				DKArray<VariantNode*> removedChildren;		// 나중에 지우기 위해 잠시 보관용 컨테이너
				removedChildren.Reserve(this->children.Count());

				for (size_t i = 0; i < this->children.Count(); ++i)
				{
					VariantNode* child = this->children.Value(i);
					itemsRemoved.Add(wxDataViewItem(child));
					child->parent = NULL;
					removedChildren.Add(child);
				}
				this->children.Clear();
				if (itemsRemoved.Count() > 0)
					model->ItemsDeleted(wxDataViewItem(this), itemsRemoved);

				DKVariant value(t);
				switch (t)
				{
				case DKVariant::TypeInteger:
					switch (this->ValueType())
					{
					case DKVariant::TypeInteger:
						value.Integer() = this->numericValue.Integer();
						break;
					case DKVariant::TypeFloat:
						value.Integer() = this->numericValue.Real();
						break;
					case DKVariant::TypeRational:
						value.Integer() = this->numericValue.Rational().Numerator() / this->numericValue.Rational().Denominator();
						break;
					case DKVariant::TypeString:
						value.Integer() = this->stringValue.ToInteger();
						break;
					default:
						value.Integer() = 0;
						break;
					}
					break;
				case DKVariant::TypeFloat:
					switch (this->ValueType())
					{
					case DKVariant::TypeInteger:
						value.Float() = this->numericValue.Integer();
						break;
					case DKVariant::TypeFloat:
						value.Float() = this->numericValue.Real();
						break;
					case DKVariant::TypeRational:
						value.Float() = static_cast<double>(this->numericValue.Rational().Numerator()) / this->numericValue.Rational().Denominator();
						break;
					case DKVariant::TypeString:
						value.Float() = this->stringValue.ToRealNumber();
						break;
					default:
						value.Float() = 0;
						break;
					}
					break;
				case DKVariant::TypeRational:
					switch (this->ValueType())
					{
					case DKVariant::TypeInteger:
						value.Rational() = DKVariant::VRational(this->numericValue.Integer());
						break;
					case DKVariant::TypeFloat:
						value.Rational() = DKVariant::VRational(this->numericValue.Real());
						break;
					default:
						value.Rational() = DKVariant::VRational(0);
					}
					break;
				case DKVariant::TypeString:
					switch (this->ValueType())
					{
					case DKVariant::TypeInteger:
						value.String() = DKString::Format("%d", this->numericValue.Integer());
						break;
					case DKVariant::TypeFloat:
						value.String() = DKString::Format("%g", this->numericValue.Real());
						break;
					case DKVariant::TypeRational:
						value.String() = DKString::Format("%g", static_cast<double>(this->numericValue.Rational().Numerator()) / this->numericValue.Rational().Denominator());
						break;
					case DKVariant::TypeString:
						value.String() = this->stringValue;
						break;
					default:
						value.String() = L"";
						break;
					}
					break;
				default:
					value.SetValueType(t);
					break;
				}			
				SetVariant(k, value);

				for (size_t i = 0; i < this->children.Count(); ++i)
				{
					VariantNode* child = this->children.Value(i);
					itemsAdded.Add(wxDataViewItem(child));
				}

				if (itemsAdded.Count() > 0)
					model->ItemsAdded(wxDataViewItem(this), itemsAdded);

				// 보관했던 자식들 제거.
				for (size_t i = 0; i < removedChildren.Count(); ++i)
				{
					VariantNode* child = removedChildren.Value(i);
					delete child;
				}
			}
			return true;
		}
		return false;
	}

	// SetValue: 바뀐게 없다면 false 를 리턴함.
	// true 를 리턴하게 되면 EVT_DATAVIEW_ITEM_VALUE_CHANGED 메시지가 간다
	bool SetValue(const DKString& v, wxDataViewModel* model)
	{
		wxASSERT(model);
		wxASSERT(IsValueEditable());
		wxASSERT(this->Validate());

		if (IsValueEditable())
		{
			DKVariant::Type t = this->ValueType();
			if (t == DKVariant::TypeInteger)
			{
				DKVariant::VInteger value = v.ToInteger();
				if (this->numericValue.Integer() != value)
				{
					this->numericValue.Integer() = value;
					return true;
				}
			}
			else if (t == DKVariant::TypeFloat)
			{
				DKVariant::VFloat value = v.ToRealNumber();
				if (this->numericValue.Real() != value)
				{
					this->numericValue.Real() = value;
					return true;
				}
			}
			else if (t == DKVariant::TypeRational)
			{
				DKString::IntegerArray intArray = v.ToIntegerArray(L"/");
				DKVariant::VRational::Integer val[2] = {0LL, 1LL};
				for (size_t i = 0; i < 2 && i < intArray.Count(); ++i)
					val[i] = intArray.Value(i);
				DKVariant::VRational value(val[0], val[1]);
				DKVariant::VRational old = this->numericValue.Rational();
				
				if (old.Numerator() != value.Numerator() || old.Denominator() != value.Denominator())
				{
					this->numericValue.SetRational(value);
					return true;
				}
			}
			else if (t == DKVariant::TypeString)
			{
				if (this->stringValue.Compare(v))
				{
					this->stringValue = v;
					return true;
				}
			}
			else
			{
				wxFAIL_MSG("Wrong Value Type!");
			}
		}
		return false;
	}

	bool SetDataValue(DKData* data, wxDataViewModel* model)
	{
		wxASSERT(model);
		wxASSERT_MSG(this->type == VTypeData, "Type must be VTypeData");
		
		this->dataValue = data;

		return true;
	}

	bool IsKeyEditable(void) const
	{
		if (parent && parent->type == VTypePairs)
			return true;
		return false;
	}

	bool IsTypeEditable(void) const
	{
		switch (this->type)
		{
		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			return false;
			break;
		}
		return true;
	}

	bool IsValueEditable(void) const // 에디터에서 바로 수정 가능한지 여부
	{
		switch (this->type)
		{
		case VTypeInteger:
		case VTypeFloat:
		case VTypeString:
		case VTypeRational:		// 문자열 형식으로 수정할 수 있음. (예: "123/45")
			return true;
			break;
		case VTypeVector2:		// 행렬,벡터 등은 트리 열어서 하나씩 수정해야함
		case VTypeVector3:
		case VTypeVector4:
		case VTypeMatrix2:
		case VTypeMatrix3:
		case VTypeMatrix4:
		case VTypeQuaternion:
			return false;
			break;
		case VTypeArray:
		case VTypePairs:
		case VTypeDateTime:		// 직접 수정할 수 없음.
		case VTypeData:			// 직접 수정은 안되며, 파일 선택만 가능
			return false;
			break;
		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			return true;
			break;
		}
		return false;
	}

	DKVariant::Type ValueType(void) const
	{
		switch (this->type)
		{
		case VTypeInteger:		return DKVariant::TypeInteger; break;
		case VTypeFloat:		return DKVariant::TypeFloat; break;
		case VTypeVector2:		return DKVariant::TypeVector2; break;
		case VTypeVector3:		return DKVariant::TypeVector3; break;
		case VTypeVector4:		return DKVariant::TypeVector4; break;
		case VTypeMatrix2:		return DKVariant::TypeMatrix2; break;
		case VTypeMatrix3:		return DKVariant::TypeMatrix3; break;
		case VTypeMatrix4:		return DKVariant::TypeMatrix4; break;
		case VTypeQuaternion:	return DKVariant::TypeQuaternion; break;
		case VTypeRational:		return DKVariant::TypeRational; break;
		case VTypeString:		return DKVariant::TypeString; break;
		case VTypeDateTime:		return DKVariant::TypeDateTime; break;
		case VTypeData:			return DKVariant::TypeData; break;
		case VTypeArray:		return DKVariant::TypeArray; break;
		case VTypePairs:		return DKVariant::TypePairs; break;

		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			return DKVariant::TypeFloat;
			break;
		}
		return DKVariant::TypeUndefined;
	}
	bool IsSubNode(void) const
	{
		switch (this->type)
		{
		case VTypeUndefined:
		case VTypeInteger:
		case VTypeFloat:
		case VTypeVector2:
		case VTypeVector3:
		case VTypeVector4:
		case VTypeMatrix2:
		case VTypeMatrix3:
		case VTypeMatrix4:
		case VTypeQuaternion:
		case VTypeRational:
		case VTypeString:
		case VTypeDateTime:
		case VTypeData:
		case VTypeArray:
		case VTypePairs:
			return false;
			break;
		}
		return false;
	}
	bool IsContainer(void) const
	{
		switch (this->type)
		{
		case VTypeVector2:
		case VTypeVector3:
		case VTypeVector4:
		case VTypeMatrix2:
		case VTypeMatrix3:
		case VTypeMatrix4:
		case VTypeQuaternion:
		case VTypeArray:
		case VTypePairs:
			return true;
			break;
		}
		return false;
	}
	wxString ValueEditingLabel(void) const
	{
		wxASSERT(IsValueEditable());

		switch (this->type)
		{
		case VTypeRational:
			{
				DKVariant::VRational r = this->numericValue.Rational();
				if (r.Denominator() == 1)
					return wxLongLong(r.Numerator()).ToString();
				else
					return wxString::Format("%lld / %lld", r.Numerator(), r.Denominator());
			}
			break;
		case VTypeInteger:
		case VTypeFloat:
		case VTypeString:
		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			return ValueString();
			break;
		}
		wxFAIL_MSG("ERROR?");
		return "ERROR??";
	}
	wxString ValueString(void) const
	{
		switch (this->type)
		{
		case VTypeUndefined:
			return "n/a";
			break;
		case VTypeInteger:
			return wxLongLong(this->numericValue.Integer()).ToString();
			break;
		case VTypeFloat:
			return wxString::Format("%.32g", this->numericValue.Real());
			break;
		case VTypeVector2:
			wxASSERT_MSG(children.Count() == 2, "children count must be 2");
			return wxString::Format("%.16g, %.16g", children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real());
			break;
		case VTypeVector3:
			wxASSERT_MSG(children.Count() == 3, "children count must be 3");
			return wxString::Format("%.16g, %.16g, %.16g",
				children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(), children.Value(2)->numericValue.Real());
			break;
		case VTypeVector4:		
			wxASSERT_MSG(children.Count() == 4, "children count must be 4");
			return wxString::Format("%.16g, %.16g, %.16g, %.16g",
				children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(), children.Value(2)->numericValue.Real(), children.Value(3)->numericValue.Real());
			break;
		case VTypeMatrix2:
			wxASSERT_MSG(children.Count() == 4, "children count must be 4");
			if (DKMatrix2(
				children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(),
				children.Value(2)->numericValue.Real(), children.Value(3)->numericValue.Real()).IsIdentity())
				return "2x2 identity matrix";
			return "2x2 matrix";
			break;
		case VTypeMatrix3:
			wxASSERT_MSG(children.Count() == 9, "children count must be 9");
			if (DKMatrix3(
				children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(), children.Value(2)->numericValue.Real(),
				children.Value(3)->numericValue.Real(), children.Value(4)->numericValue.Real(), children.Value(5)->numericValue.Real(),
				children.Value(6)->numericValue.Real(), children.Value(7)->numericValue.Real(), children.Value(8)->numericValue.Real()).IsIdentity())
				return "3x3 identity matrix";
			return "3x3 matrix";
			break;
		case VTypeMatrix4:
			wxASSERT_MSG(children.Count() == 16, "children count must be 16");
			if (DKMatrix4(
				children.Value(0x0)->numericValue.Real(), children.Value(0x1)->numericValue.Real(), children.Value(0x2)->numericValue.Real(), children.Value(0x3)->numericValue.Real(),
				children.Value(0x4)->numericValue.Real(), children.Value(0x5)->numericValue.Real(), children.Value(0x6)->numericValue.Real(), children.Value(0x7)->numericValue.Real(),
				children.Value(0x8)->numericValue.Real(), children.Value(0x9)->numericValue.Real(), children.Value(0xa)->numericValue.Real(), children.Value(0xb)->numericValue.Real(),
				children.Value(0xc)->numericValue.Real(), children.Value(0xd)->numericValue.Real(), children.Value(0xe)->numericValue.Real(), children.Value(0xf)->numericValue.Real()).IsIdentity())
				return "4x4 identity matrix";
			return "4x4 matrix";
			break;
		case VTypeQuaternion:
			wxASSERT_MSG(children.Count() == 4, "children count must be 4");
			if (DKQuaternion(children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(), children.Value(2)->numericValue.Real(), children.Value(3)->numericValue.Real()).IsIdentity())
				return wxString::Format("%.16g, %.16g, %.16g, %.16g (identity)",
				children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(), children.Value(2)->numericValue.Real(), children.Value(3)->numericValue.Real());
			return wxString::Format("%.16g, %.16g, %.16g, %.16g",
				children.Value(0)->numericValue.Real(), children.Value(1)->numericValue.Real(), children.Value(2)->numericValue.Real(), children.Value(3)->numericValue.Real());
			break;
		case VTypeRational:
			if (this->numericValue.Rational().IsNumeric())
			{
				DKVariant::VRational r = this->numericValue.Rational();
				if (r.Denominator() == 1)
					return wxLongLong(r.Numerator()).ToString();
				else
					return wxString::Format("%lld / %lld (%.32g)", r.Numerator(), r.Denominator(), static_cast<double>(r.Numerator()) / r.Denominator());
			}
			else
			{
				DKVariant::VRational r = this->numericValue.Rational();
				if (r.IsPositiveInfinity())
					return "+Infinity";
				else if (r.IsNegativeInfinity())
					return "-Infinity";
				return "Indeterminate (NaN)";
			}
			break;
		case VTypeString:
			return (const wchar_t*)stringValue;
			break;
		case VTypeDateTime:
			return (const wchar_t*)this->numericValue.DateTime().Format(DKDateTime::DateFormatWithWeekdayShort, DKDateTime::TimeFormat24HourWithTimezone);
			break;
		case VTypeData:
			if (dataValue)
				return wxString::Format("%llu bytes", dataValue->Length());
			return "empty";
			break;
		case VTypeArray:		
			return wxString::Format("%llu items", children.Count());
			break;
		case VTypePairs:
			return wxString::Format("%llu pairs", children.Count());
			break;
		case VTypeVector2X:
		case VTypeVector2Y:
		case VTypeVector3X:
		case VTypeVector3Y:
		case VTypeVector3Z:
		case VTypeVector4X:
		case VTypeVector4Y:
		case VTypeVector4Z:
		case VTypeVector4W:
		case VTypeMatrix20:
		case VTypeMatrix21:
		case VTypeMatrix22:
		case VTypeMatrix23:
		case VTypeMatrix30:
		case VTypeMatrix31:
		case VTypeMatrix32:
		case VTypeMatrix33:
		case VTypeMatrix34:
		case VTypeMatrix35:
		case VTypeMatrix36:
		case VTypeMatrix37:
		case VTypeMatrix38:
		case VTypeMatrix40:
		case VTypeMatrix41:
		case VTypeMatrix42:
		case VTypeMatrix43:
		case VTypeMatrix44:
		case VTypeMatrix45:
		case VTypeMatrix46:
		case VTypeMatrix47:
		case VTypeMatrix48:
		case VTypeMatrix49:
		case VTypeMatrix4A:
		case VTypeMatrix4B:
		case VTypeMatrix4C:
		case VTypeMatrix4D:
		case VTypeMatrix4E:
		case VTypeMatrix4F:
		case VTypeQuaternionX:
		case VTypeQuaternionY:
		case VTypeQuaternionZ:
		case VTypeQuaternionW:
			return wxString::Format("%g", this->numericValue.Real());
			break;
		}
		wxFAIL_MSG("ERROR?");
		return "ERROR??";
	}

	class NumericValue
	{
	public:
		NumericValue(void)										{memset(&value, 0, sizeof(value));}
		NumericValue(const NumericValue& n)						{memcpy(&value, &n.value, sizeof(value));}
		NumericValue(DKVariant::VInteger v)						{memset(&value, 0, sizeof(value));value.integer = v;}
		NumericValue(DKVariant::VFloat v)						{memset(&value, 0, sizeof(value));value.real = v;}
		
		DKVariant::VInteger& Integer(void)						{return value.integer;}
		const DKVariant::VInteger& Integer(void) const			{return value.integer;}
		DKVariant::VFloat& Real(void)							{return value.real;}
		const DKVariant::VFloat& Real(void) const				{return value.real;}
		DKVariant::VRational Rational(void) const				{return DKVariant::VRational(value.rational.n, value.rational.d);}
		void SetRational(const DKVariant::VRational& r)			{value.rational.n = r.Numerator(); value.rational.d = r.Denominator();}
		DKVariant::VDateTime DateTime(void) const				{return DKVariant::VDateTime(value.time.s, value.time.ms);}
		void SetDateTime(const DKVariant::VDateTime& d)			{value.time.s = d.SecondsSinceEpoch(); value.time.ms = d.Microsecond();}

		bool operator == (const NumericValue& n) const			{return Compare(n) == 0;}
		bool operator != (const NumericValue& n) const			{return Compare(n) != 0;}
		NumericValue& operator = (const NumericValue& n)		{return SetValue(n);}

		bool IsZero(void) const									{return Compare(ZeroValue()) == 0;}		
		NumericValue& SetZero(void)								{memset(&value, 0, sizeof(value)); return *this;}
		NumericValue& SetValue(const NumericValue& n)			{memcpy(&value, &n.value, sizeof(value)); return *this;}	
		int Compare(const NumericValue& n) const				{return memcmp(&value, &n.value, sizeof(value));}
		
		static const NumericValue& ZeroValue(void)
		{
			static const NumericValue zeroValue = NumericValue().SetZero();
			return zeroValue;
		}
	private:
		union
		{
			DKVariant::VInteger	integer;		// VInteger
			DKVariant::VFloat	real;			// VFloat
			struct {long long n, d;} rational;
			struct {long long s; int ms;} time;
		} value;
	} numericValue;
	DKString				stringValue;	// VString
	DKObject<DKData>		dataValue;		// 공유가능한 값
	VType					type;
	DKString				key;
	VariantNode*			parent;
	DKArray<VariantNode*>	children;
};

////////////////////////////////////////////////////////////////////////////////
#pragma mark - VariantObject Implementation
VariantObject::VariantObject(void)
{
	node = new VariantNode(NULL, L"root", DKVariant(DKVariant::TypeUndefined));
}

VariantObject::VariantObject(const DKVariant& v)
{
	node = new VariantNode(NULL, L"root", v);
}

VariantObject::VariantObject(const VariantObject& v)
{
	wxASSERT(v.node);	
	node = v.node->Duplicate(NULL);
}

VariantObject::VariantObject(const VariantNode* p)
{
	wxASSERT(p);
	node = p->Duplicate(NULL);
}

VariantObject::~VariantObject(void)
{
	wxASSERT(node);
	delete node;
}

DKVariant VariantObject::Value(void) const
{
	wxASSERT(node);
	return node->GetVariant();
}

DKVariant::Type VariantObject::Type(void) const
{
	wxASSERT(node);
	return node->ValueType();
}

bool VariantObject::IsEqual(const VariantObject* vo) const
{
	wxASSERT(node);
	wxASSERT(vo);
	wxASSERT(vo->node);
	return this->node->IsEqual(vo->node);
}

////////////////////////////////////////////////////////////////////////////////
#pragma mark - VariantDataModel

namespace
{
	const struct VariantColumnTypeInfo
	{
		DKVariant::Type vtype;
		wxString string;
	} columnStringValues[] =
	{
		{DKVariant::TypeUndefined,	"undefined"},
		{DKVariant::TypeInteger,	"integer"},
		{DKVariant::TypeFloat,		"real"},
		{DKVariant::TypeVector2,	"vector2"},
		{DKVariant::TypeVector3,	"vector3"},
		{DKVariant::TypeVector4,	"vector4"},
		{DKVariant::TypeMatrix2,	"matrix2"},
		{DKVariant::TypeMatrix3,	"matrix3"},
		{DKVariant::TypeMatrix4,	"matrix4"},
		{DKVariant::TypeQuaternion,	"quaternion"},
		{DKVariant::TypeRational,	"rational"},
		{DKVariant::TypeString,		"text"},
		{DKVariant::TypeDateTime,	"date-time"},
		{DKVariant::TypeData,		"binary data"},
		{DKVariant::TypeArray,		"array"},
		{DKVariant::TypePairs,		"pairs"},
	};
}

class VariantDataModel : public wxDataViewModel
{
public:
	enum DataViewColumn
	{
		DataViewColumnName = 0,
		DataViewColumnType,
		DataViewColumnValue,
		DataViewColumnMax,
	};
	
	static wxString VariantTypeToString(DKVariant::Type t)
	{
		const int numColumns = sizeof(columnStringValues) / sizeof(VariantColumnTypeInfo);
		for (int i = 0; i < numColumns; ++i)
		{
			if (columnStringValues[i].vtype == t)
				return columnStringValues[i].string;
		}
		wxFAIL_MSG("Unknown type?");
		return "unknown";
	}

	static DKVariant::Type StringToVariantType(const wxString& s)
	{
		const int numColumns = sizeof(columnStringValues) / sizeof(VariantColumnTypeInfo);
		for (int i = 0; i < numColumns; ++i)
		{
			if (columnStringValues[i].string.Cmp(s) == 0)
				return columnStringValues[i].vtype;
		}
		wxFAIL_MSG("Unknown type?");
		return DKVariant::TypeUndefined;
	}

	VariantDataModel(void)
		: rootNode(new VariantNode(NULL, L"root"))
		, readonly(true)
		, valueEditing(false)
	{
	}

	~VariantDataModel(void)
	{
		delete rootNode;
	}

	void SetVariant(const VariantObject* v, bool ro)
	{
		rootNode->Revert(v->node, this);
		readonly = ro;
	}

	DKObject<VariantObject> GetVariant(void) const
	{
		DKObject<VariantObject> vo = DKOBJECT_NEW VariantObject(rootNode);
		return vo;
	}

	bool IsReadOnly(void) const
	{
		return readonly;
	}

	wxDataViewItem RootItem(void) const
	{
		return wxDataViewItem(rootNode);
	}

	bool BeginValueEditing(void)
	{
		wxASSERT(!valueEditing);
		if (readonly)
			return false;

		valueEditing = true;
		return true;
	}

	void EndValueEditing(void)
	{
		wxASSERT(valueEditing);
		wxASSERT(!readonly);

		valueEditing = false;
	}

protected:
/*
	bool HasDefaultCompare(void) const
	{
		return true;
	}
*/
	int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const
	{
		VariantNode *node1 = (VariantNode*)item1.GetID();
		VariantNode *node2 = (VariantNode*)item2.GetID();

		int ret = 0;
		switch (column)
		{
		case DataViewColumnName:
			ret = node1->key.CompareNoCase(node2->key);
			if (ret == 0)
				ret = node1->key.Compare(node2->key);
			break;
		case DataViewColumnType:
			ret = static_cast<int>(node1->type) - static_cast<int>(node2->type);
			break;
		case DataViewColumnValue:
			ret = node1->ValueString().Cmp(node2->ValueString());
			break;
		}
		if (ascending)
			return ret;
		return -ret;
	}
	bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
	{
		wxASSERT(item.IsOk());

		DKLog("[%s] column:%d variant:\"%ls\" (type:%ls)\n", DKLIB_FUNCTION_NAME, col, (const wchar_t*)variant.GetString(), (const wchar_t*)variant.GetType());
		
		VariantNode *node = (VariantNode*)item.GetID();

		switch (col)
		{
		case VariantDataModel::DataViewColumnName:
			if (node->SetKey((const wchar_t*)variant.GetString(), this))
			{
				wxASSERT(node->parent->ValueType() == DKVariant::TypePairs);
				node->parent->SortChildren();
				return true;
			}
			return false;
			break;
		case VariantDataModel::DataViewColumnType:
			{
				// 2012-02-14: 윈도우에서는 string 형식으로 오고 맥에서는 long 형식으로 온다. (나중에 바뀔수 있음)
				DKVariant::Type vtype = DKVariant::TypeUndefined;
				if (variant.GetType().CmpNoCase("string") == 0)
				{
					vtype = StringToVariantType(variant.GetString());
				}
				else
				{
					vtype = columnStringValues[variant.GetLong()].vtype;
				}
				return node->SetType(vtype, this);
			}
			break;
		case VariantDataModel::DataViewColumnValue:
			return node->SetValue((const wchar_t*)variant.GetString(), this);
			break;
		default:
			wxFAIL_MSG("Unknown column!");
			break;
		}
		return false;
	}

	void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
	{
		wxASSERT(item.IsOk());

		VariantNode *node = (VariantNode*)item.GetID();

		switch (col)
		{
		case DataViewColumnName:
			variant = (const wchar_t*)node->key;
			break;
		case DataViewColumnType:
			variant = VariantTypeToString(node->ValueType());
			break;
		case DataViewColumnValue:
			if (this->valueEditing)
				variant = node->ValueEditingLabel();
			else
				variant = node->ValueString();
			break;
		default:
			wxLogError("%s: wrong column %d", DKLIB_FUNCTION_NAME, col);
			wxASSERT(0);
			break;
		}
	}

    bool GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const
    {
		wxASSERT(item.IsOk());

		if (IsEnabled(item, col))
			return false;

		switch (col)
		{
		case DataViewColumnName:
			attr.SetColour(*wxBLUE);
			//attr.SetItalic(true);
			break;
		case DataViewColumnType:
			attr.SetColour(*wxBLUE);
			attr.SetItalic(true);
			break;
		case DataViewColumnValue:
			attr.SetColour(*wxBLUE);
			attr.SetItalic(true);
			break;
		default:
			wxFAIL_MSG("Unknown column!");
			break;
		}
		return true;
    }

	bool IsEnabled(const wxDataViewItem& item, unsigned int col) const
	{
		wxASSERT(item.IsOk());

		VariantNode *node = (VariantNode*)item.GetID();

		switch (col)
		{
		case DataViewColumnName:
			return node->IsKeyEditable();
			break;
		case DataViewColumnType:
			return node->IsTypeEditable();
			break;
		case DataViewColumnValue:
			return node->IsValueEditable();
			break;
		default:
			wxFAIL_MSG("Unknown column!");
			break;
		}
		return false;
	}

	bool IsContainer(const wxDataViewItem& item) const
	{
		if (!item.IsOk())
			return true;

		VariantNode *node = (VariantNode*)item.GetID();
		return node->IsContainer();
	}

	bool HasContainerColumns(const wxDataViewItem& item) const
	{
		if (item.IsOk())
			return true;
		return false;
	}

	unsigned int GetColumnCount(void) const
	{
		return DataViewColumnMax;
	}

	wxString GetColumnType(unsigned int col) const
	{
		return "string";
	}

	wxDataViewItem GetParent(const wxDataViewItem& item) const
	{
		if (!item.IsOk())
			return wxDataViewItem(0);

		VariantNode *node = (VariantNode*)item.GetID();
		return wxDataViewItem( node->parent );
	}

	unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
	{
		VariantNode *node = (VariantNode*)item.GetID();
		if (!node)
		{
			children.Add( wxDataViewItem(rootNode) );
			return 1;
		}

		unsigned int c = (unsigned int)node->children.Count();
		for (unsigned int i = 0; i < c; ++i)
		{
			children.Add( wxDataViewItem(node->children.Value(i)) );
		}
		return c;
	}
private:
	bool readonly;
	bool valueEditing;		// DataViewColumnValue 컬럼 편집시 true, (편집용 텍스트가 따로 존재할 수 있음, GetValue 변경용)
	VariantNode* rootNode;
};


////////////////////////////////////////////////////////////////////////////////
#pragma mark - VariantEditor Implementation

BEGIN_EVENT_TABLE(VariantEditor, wxWindow)
	EVT_SIZE(									VariantEditor::OnSize)
	EVT_CLOSE(									VariantEditor::OnClose)
	EVT_CONTEXT_MENU(							VariantEditor::OnContextMenu)

	EVT_MENU(wxID_NEW,							VariantEditor::OnItemNew)
	EVT_MENU(wxID_CUT,							VariantEditor::OnItemCut)
	EVT_MENU(wxID_COPY,							VariantEditor::OnItemCopy)
	EVT_MENU(wxID_PASTE,						VariantEditor::OnItemPaste)
	EVT_MENU(wxID_DUPLICATE,					VariantEditor::OnItemDuplicate)
	EVT_MENU(wxID_DELETE,						VariantEditor::OnItemDelete)

	EVT_MENU(UICommandFileImport,				VariantEditor::OnItemFileImport)
	EVT_MENU(UICommandFileExport,				VariantEditor::OnItemFileExport)
	EVT_MENU(UICommandDataImport,				VariantEditor::OnDataItemImport)
	EVT_MENU(UICommandDataExport,				VariantEditor::OnDataItemExport)
	EVT_MENU(UICommandArrayReorder,				VariantEditor::OnArrayItemReorder)
	EVT_MENU(UICommandDateTimeSetValue,			VariantEditor::OnDateTimeItemSetValue)

	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY,		VariantEditor::OnDataViewItemActivated)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY,	VariantEditor::OnDataViewItemContextMenu)
	EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY,	VariantEditor::OnDataViewItemStartEditing)
	EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY,	VariantEditor::OnDataViewItemEditingDone)
	EVT_DATAVIEW_ITEM_VALUE_CHANGED(wxID_ANY,	VariantEditor::OnDataViewItemValueChanged)
	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY,		VariantEditor::OnDataViewItemBeginDrag)
	EVT_DATAVIEW_ITEM_DROP_POSSIBLE(wxID_ANY,	VariantEditor::OnDataViewItemDropPossible)
	EVT_DATAVIEW_ITEM_DROP(wxID_ANY,			VariantEditor::OnDataViewItemDrop)
END_EVENT_TABLE()


VariantEditor::VariantEditor(void)
	: dataView(NULL)
	, dataModel(NULL)
	, enableDragSource(false)
	, enableDropTarget(false)
{
	InitDataView();
}

VariantEditor::VariantEditor(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
	: wxWindow(parent, id, pos, size, style, name)
	, dataView(NULL)
	, dataModel(NULL)
	, enableDragSource(false)
	, enableDropTarget(false)
{
	InitDataView();
}

VariantEditor::~VariantEditor(void)
{
}

wxString VariantEditor::VariantTypeToString(DKVariant::Type t)
{
	return VariantDataModel::VariantTypeToString(t);
}

DKVariant::Type VariantEditor::StringToVariantType(const wxString& s)
{
	return VariantDataModel::StringToVariantType(s);
}

void VariantEditor::OnSize(wxSizeEvent& e)
{
	dataView->SetSize(e.GetSize());
}

void VariantEditor::OnClose(wxCloseEvent& e)
{
}

void VariantEditor::OnContextMenu(wxContextMenuEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

void VariantEditor::OnItemNew(wxCommandEvent& e)
{
	// 현재 선택된 map, array 을 expand 한다.
	// 현재 선택된게 map, array 가 아니면 error.
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);

	size_t numSel = selections.Count();
	wxASSERT(numSel == 1);

	VariantNode* selectedNode = ((VariantNode*)selections[0].GetID());

	DKVariant::Type valueType = selectedNode->ValueType();

	VariantNode* node = NULL;

	if (valueType == DKVariant::TypeArray)
	{
		// 새로 추가한 다음 선택.
		node = selectedNode->AddItem(DKVariant::TypeUndefined, this->dataModel.get());
	}
	else if (valueType == DKVariant::TypePairs)
	{
		// 새로 추가한 다음 선택.
		for (size_t i = 0; node == NULL; ++i)
		{
			DKString key = (i > 0) ? DKString::Format("new item %llu", i+1) : L"new item";
			node = selectedNode->AddItem(key, DKVariant::TypeUndefined, this->dataModel.get());
		}
		DKLog("new value(key:%ls) added.\n", (const wchar_t*)node->key);
	}
	else
	{
		wxFAIL_MSG("invalid type");
	}
	if (node)
	{
		this->dataView->UnselectAll();
		this->dataView->Expand(wxDataViewItem(selectedNode));
		this->dataView->Select(wxDataViewItem(node));

		VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
		evt.SetEventObject(this);
		ProcessWindowEvent(evt);
		
		if (node->IsKeyEditable())
			this->dataView->EditItem(wxDataViewItem(node), this->dataView->GetColumn(VariantDataModel::DataViewColumnName));
	}
}

void VariantEditor::OnItemCut(wxCommandEvent& e)
{
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);

	size_t numSel = selections.Count();

	DKVariant clipboardNodeArray(DKVariant::TypeArray);

	DKSet<VariantNode*> selectedNodes;

	for (size_t i = 0; i < numSel; ++i)
	{
		VariantNode* node = (VariantNode*)selections.Item(i).GetID();
		clipboardNodeArray.Array().Add(node->GetVariant());
		selectedNodes.Insert(node);
	}

	if (clipboardNodeArray.Array().Count() > 0)
	{
		if (SetClipboardData(clipboardNodeArray) == size_t(-1))
		{
			wxMessageBox("failed to open clipboard", "Error");
			return;
		}

		DKArray<VariantNode*> itemsToDelete;
		itemsToDelete.Reserve(selectedNodes.Count());
		for (size_t i = 0; i < numSel; ++i)
		{
			VariantNode* node = ((VariantNode*)selections.Item(i).GetID());
			wxASSERT(node->parent != NULL);

			// 선택된 아이템들중 종속관계가 있는것 찾는다.
			// 자식노드와 부모노드가 모두 선택되어있을경우, 부모노드만 삭제.
			bool parentSelected = false;
			for (VariantNode* n = node->parent; n != NULL; n = n->parent)
			{
				if (selectedNodes.Contains(n))
				{
					parentSelected = true;
					break;
				}
			}
			if (parentSelected == false)
				itemsToDelete.Add(node);
		}

		size_t numDeleted = 0;
		wxASSERT(itemsToDelete.Count() > 0);
		for (size_t i = 0; i < itemsToDelete.Count(); ++i)
		{
			VariantNode* node = itemsToDelete.Value(i);
			wxASSERT(node->parent);

			if (node->parent->ValueType() == DKVariant::TypeArray)
			{
				for (size_t k = 0; k < node->parent->children.Count(); ++k)
				{
					if (node->parent->children.Value(k) == node)
					{
						bool result = node->parent->RemoveItem(k, this->dataModel.get());
						wxASSERT(result);
						if (result)
							numDeleted++;
						break;
					}
				}
			}
			else if (node->parent->ValueType() == DKVariant::TypePairs)
			{
				bool result = node->parent->RemoveItem(node->key, this->dataModel.get());
				wxASSERT(result);
				if (result)
					numDeleted++;
			}
			else
			{
				wxFAIL_MSG("invalid type!");
			}
		}
		if (numDeleted > 0)
		{
			VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
			evt.SetEventObject(this);
			ProcessWindowEvent(evt);
		}
	}
}

void VariantEditor::OnItemCopy(wxCommandEvent& e)
{
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);

	size_t numSel = selections.Count();

	DKVariant clipboardNodeArray(DKVariant::TypeArray);

	for (size_t i = 0; i < numSel; ++i)
	{
		VariantNode* node = (VariantNode*)selections.Item(i).GetID();
		clipboardNodeArray.Array().Add(node->GetVariant());
	}

	if (clipboardNodeArray.Array().Count() > 0)
	{
		if (SetClipboardData(clipboardNodeArray) == size_t(-1))
		{
			wxMessageBox("failed to open clipboard", "Error");
		}
	}
}

void VariantEditor::OnItemPaste(wxCommandEvent& e)
{
	DKLog("[%s] (this event must be fired before mainWindow's)\n", DKLIB_FUNCTION_NAME);

	// 현재 선택된 map, array 을 expand 한다.
	// 현재 선택된게 map, array 가 아니면 error.
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);

	size_t numSel = selections.Count();
	wxASSERT(numSel == 1);

	VariantNode* selectedNode = ((VariantNode*)selections[0].GetID());
	DKVariant::Type valueType = selectedNode->ValueType();

	DKVariant data(DKVariant::TypeUndefined);
	if (GetClipboardData(data) != size_t(-1))
	{
		if (data.ValueType() == DKVariant::TypeArray)
		{
			wxDataViewItemArray addedItems;

			if (valueType == DKVariant::TypeArray)
			{
				// 새로 추가한 다음 선택.
				for (size_t i = 0; i < data.Array().Count(); ++i)
				{
					VariantNode* node = selectedNode->AddItem(data.Array().Value(i), this->dataModel.get());
					addedItems.Add(wxDataViewItem(node));
				}
			}
			else if (valueType == DKVariant::TypePairs)
			{
				// 새로 추가한 다음 선택.
				size_t indexBegin = 0;
				for (size_t i = 0; i < data.Array().Count(); ++i)
				{
					VariantNode* node = NULL;
					while (node == NULL)
					{
						DKString key = (indexBegin > 0) ? DKString::Format("new item %llu", indexBegin+1) : L"new item";
						node = selectedNode->AddItem(key, data.Array().Value(i), this->dataModel.get());
						indexBegin++;
					}
					addedItems.Add(wxDataViewItem(node));
				}
			}
			else
			{
				wxFAIL_MSG("invalid type");
			}

			DKLog("[%s] %llu items added.\n", DKLIB_FUNCTION_NAME, addedItems.Count());

			if (addedItems.Count() > 0)
			{
				this->dataView->UnselectAll();
				this->dataView->Expand(wxDataViewItem(selectedNode));
				this->dataView->SetSelections(addedItems);

				VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
				evt.SetEventObject(this);
				ProcessWindowEvent(evt);
			}
		}
	}
	else
	{
		DKLog("[%s] Cannot open clipboard.\n", DKLIB_FUNCTION_NAME);
		wxMessageBox("Cannot open clipboard", "Error");
	}
}

void VariantEditor::OnItemDuplicate(wxCommandEvent& e)
{
	DKLog("[%s] (this event must be fired before mainWindow's)\n", DKLIB_FUNCTION_NAME);
}

void VariantEditor::OnItemDelete(wxCommandEvent& e)
{
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);
	size_t numSel = selections.Count();

	wxASSERT(numSel > 0);

	DKArray<VariantNode*> itemsToDelete;
	itemsToDelete.Reserve(numSel);

	DKSet<VariantNode*> selectedNodes;
	for (size_t i = 0; i < numSel; ++i)
		selectedNodes.Insert(((VariantNode*)selections.Item(i).GetID()));	// 선택된 모든 노드 복사 (빠른 검색용)

	for (size_t i = 0; i < numSel; ++i)
	{
		VariantNode* node = ((VariantNode*)selections.Item(i).GetID());
		wxASSERT(node->parent != NULL);

		// 선택된 아이템들중 종속관계가 있는것 찾는다.
		// 자식노드와 부모노드가 모두 선택되어있을경우, 부모노드만 삭제.
		bool parentSelected = false;
		for (VariantNode* n = node->parent; n != NULL; n = n->parent)
		{
			if (selectedNodes.Contains(n))
			{
				parentSelected = true;
				break;
			}
		}
		if (parentSelected == false)
			itemsToDelete.Add(node);
	}

	size_t numDeleted = 0;
	wxASSERT(itemsToDelete.Count() > 0);
	for (size_t i = 0; i < itemsToDelete.Count(); ++i)
	{
		VariantNode* node = itemsToDelete.Value(i);
		wxASSERT(node->parent);

		if (node->parent->ValueType() == DKVariant::TypeArray)
		{
			for (size_t k = 0; k < node->parent->children.Count(); ++k)
			{
				if (node->parent->children.Value(k) == node)
				{
					bool result = node->parent->RemoveItem(k, this->dataModel.get());
					wxASSERT(result);
					if (result)
						numDeleted++;
					break;
				}
			}
		}
		else if (node->parent->ValueType() == DKVariant::TypePairs)
		{
			bool result = node->parent->RemoveItem(node->key, this->dataModel.get());
			wxASSERT(result);
			if (result)
				numDeleted++;
		}
		else
		{
			wxFAIL_MSG("invalid type!");
		}
	}
	if (numDeleted > 0)
	{
		VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
		evt.SetEventObject(this);
		ProcessWindowEvent(evt);
	}
}

void VariantEditor::OnItemFileImport(wxCommandEvent& e)
{
	DKLog("[%s] (this event must be fired before mainWindow's)\n", DKLIB_FUNCTION_NAME);
}

void VariantEditor::OnItemFileExport(wxCommandEvent& e)
{
	DKLog("[%s] (this event must be fired before mainWindow's)\n", DKLIB_FUNCTION_NAME);
}

void VariantEditor::OnDataItemImport(wxCommandEvent& e)
{
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);
	size_t numSel = selections.Count();
	wxASSERT(numSel == 1);
	VariantNode* selectedNode = ((VariantNode*)selections[0].GetID());
	wxASSERT(selectedNode->ValueType() == DKVariant::TypeData);

	const wxString fileFilter = "All files (*.*)|*.*";
	wxFileDialog file(this, "Select PropertySet", wxEmptyString, wxEmptyString, fileFilter, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (file.ShowModal() == wxID_OK)
	{
		wxString filePath = file.GetPath();

		DKLog("Import data from file:%ls.\n", (const wchar_t*)filePath);

		DKObject<DKData> dataImported = DKBuffer::Create((const wchar_t*)filePath).SafeCast<DKData>();
		if (dataImported)
		{
			if (dataImported->Length() > 0)
			{
				selectedNode->dataValue = dataImported;
				this->dataModel->ItemChanged(wxDataViewItem(selectedNode));
				VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
				evt.SetEventObject(this);
				ProcessWindowEvent(evt);
			}
			else
			{
				wxMessageBox("file is empty", "Warning");
			}
		}
		else
		{
			wxMessageBox("Cannot import file.", "Error");
		}
	}
}

void VariantEditor::OnDataItemExport(wxCommandEvent& e)
{
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);
	size_t numSel = selections.Count();
	wxASSERT(numSel == 1);
	VariantNode* selectedNode = ((VariantNode*)selections[0].GetID());
	wxASSERT(selectedNode->ValueType() == DKVariant::TypeData);

	DKObject<DKData> dataToExport = selectedNode->dataValue;
	wxASSERT(dataToExport);
	wxASSERT(dataToExport->Length() > 0);

	const wxString fileFilter = "All files (*.*)|*.*";
	wxFileDialog file(this, "Select PropertySet", wxEmptyString, wxEmptyString, fileFilter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (file.ShowModal() == wxID_OK)
	{
		wxString filePath = file.GetPath();

		if (dataToExport->WriteToFile((const wchar_t*)filePath, true))
		{
			DKLog("Export data to file:%ls (%llu bytes).\n", (const wchar_t*)filePath, dataToExport->Length());
		}
		else
		{
			wxMessageBox("Cannot save file.", "Error");
		}
	}
}

void VariantEditor::OnArrayItemReorder(wxCommandEvent& e)
{
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);

	size_t numSel = selections.Count();
	wxASSERT(numSel == 1);

	VariantNode* selectedNode = ((VariantNode*)selections[0].GetID());
	VariantNode* parent = selectedNode->parent;
	wxASSERT(parent);
	wxASSERT(parent->children.Count() > 1);

	size_t currentIndex = 0;
	wxString* indexStrings = new wxString[parent->children.Count()];
	for (size_t i = 0; i < parent->children.Count(); ++i)
	{
		if (parent->children.Value(i) == selectedNode)
			currentIndex = i;
		indexStrings[i] = wxString::Format("[%llu]", i);
	}
	wxSingleChoiceDialog dlg(this, "select index", "this is caption", 
		parent->children.Count(), indexStrings);

	dlg.SetSelection(currentIndex);
	if (dlg.ShowModal() == wxID_OK)
	{
		int newIndex = dlg.GetSelection();
		DKLog("index %u -> %u change.\n", currentIndex, newIndex);
		if (newIndex != currentIndex)
		{
			parent->ReorderItem(currentIndex, newIndex, this->dataModel.get());

			VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
			evt.SetEventObject(this);
			ProcessWindowEvent(evt);
		}
	}

	delete[] indexStrings;
}

void VariantEditor::OnDateTimeItemSetValue(wxCommandEvent& e)
{
	class DateTimePicker : public wxDialog
	{
	public:
		DateTimePicker(wxWindow* parent, const wxLongLong& dt, int style)
			: wxDialog(parent, wxID_ANY, "Choose date time")
			, value(dt)
		{
			datePicker = new wxDatePickerCtrl(this, wxID_ANY, dt, wxDefaultPosition, wxDefaultSize, style);
			datePicker->SetRange(wxDateTime(1, wxDateTime::Jan, 1970), wxDefaultDateTime);
			timePicker = new wxTimePickerCtrl(this, wxID_ANY);
			timePicker->SetValue(dt);
			dateTimeStringUTC = new wxStaticText(this, wxID_ANY, this->DateTimeString(true));
			dateTimeStringLocal = new wxStaticText(this, wxID_ANY, this->DateTimeString(false));

			const wxSizerFlags flags = wxSizerFlags().Centre().Border();
			wxFlexGridSizer* const sizerMain = new wxFlexGridSizer(2);
			sizerMain->Add(datePicker, flags);
			sizerMain->Add(timePicker, flags);
			sizerMain->Add(new wxStaticText(this, wxID_ANY, "ISO-8601 UTC:"), flags);
			sizerMain->Add(dateTimeStringUTC, flags);
			sizerMain->Add(new wxStaticText(this, wxID_ANY, "ISO-8601 Local:"), flags);
			sizerMain->Add(dateTimeStringLocal, flags);

			wxStdDialogButtonSizer* sizerBtns = new wxStdDialogButtonSizer;
			sizerBtns->AddButton(new wxButton(this, wxID_OK));
			sizerBtns->AddButton(new wxButton(this, wxID_CANCEL));
			sizerBtns->Realize();

			wxSizer* sizerTop = new wxBoxSizer(wxVERTICAL);
			sizerTop->Add(sizerMain, flags);
			sizerTop->Add(sizerBtns, flags);

			SetSizerAndFit(sizerTop);

			datePicker->Bind(wxEVT_DATE_CHANGED, &DateTimePicker::OnDateTimeChanged, this);
			timePicker->Bind(wxEVT_TIME_CHANGED, &DateTimePicker::OnDateTimeChanged, this);
		}
		wxString DateTimeString(bool utc)
		{
			long long sec = this->value.GetValue() / 1000UL + WX_TIME_BASE_OFFSET;
			return (const wchar_t*)DKDateTime(sec, 0).FormatISO8601(utc);
		}
		void OnDateTimeChanged(wxDateEvent& event)
		{
			wxDateTime dt(this->value);
			if (event.GetId() == this->datePicker->GetId())
			{
				if (dt.ParseISODate(event.GetDate().FormatISODate()) == false)
				{
					wxFAIL_MSG("ParseISODate error?");
				}
			}
			else if (event.GetId() == this->timePicker->GetId())
			{
				if (dt.ParseISOTime(event.GetDate().FormatISOTime()) == false)
				{
					wxFAIL_MSG("ParseISOTime error?");
				}
			}
			else
			{
				wxASSERT_MSG(0, "Unknown control id");
				return;
			}
			this->value = dt.GetValue();
			dateTimeStringUTC->SetLabel(this->DateTimeString(true));
			dateTimeStringLocal->SetLabel(this->DateTimeString(false));
		}
		wxDatePickerCtrlBase*	datePicker;
		wxTimePickerCtrlBase*	timePicker;
		wxStaticText*			dateTimeStringUTC;
		wxStaticText*			dateTimeStringLocal;
		wxLongLong				value;
	};

	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);
	size_t numSel = selections.Count();
	wxASSERT(numSel == 1);
	VariantNode* selectedNode = ((VariantNode*)selections[0].GetID());
	wxASSERT(selectedNode->ValueType() == DKVariant::TypeDateTime);

	long long tick1 = selectedNode->GetVariant().DateTime().SecondsSinceEpoch();

	DateTimePicker picker(this, wxLongLong((tick1 - WX_TIME_BASE_OFFSET) * 1000UL), wxDP_DEFAULT | wxDP_SHOWCENTURY | wxDP_DROPDOWN);
	if (picker.ShowModal() == wxID_OK)
	{
		long long tick2 = picker.value.GetValue() / 1000UL + WX_TIME_BASE_OFFSET;
/*
		wxDateTime dt2 = dt1;
		if (dt2.ParseISODate(picker.datePicker->GetValue().FormatISODate()) == false)
		{
			wxFAIL_MSG("ParseISODate error?");
		}
		if (dt2.ParseISOTime(picker.timePicker->GetValue().FormatISOTime()) == false)
		{
			wxFAIL_MSG("ParseISOTime error?");
		}
		//dt2.MakeUTC();
		tick2 = dt2.GetTicks();

		if (tick2 == -1)
		{
			// (time_t)((m_time / (long)TIME_T_FACTOR).ToLong()) + WX_TIME_BASE_OFFSET;
			tick2 = (dt2.GetValue().GetValue() / 1000UL) + WX_TIME_BASE_OFFSET;
		}
*/

		if (tick1 != tick2)	// modified
		{
			selectedNode->numericValue.SetDateTime(DKDateTime(tick2, 0));
			this->dataModel->ItemChanged(wxDataViewItem(selectedNode));
			VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
			evt.SetEventObject(this);
			ProcessWindowEvent(evt);
		}
	}
}

void VariantEditor::SetVariant(const VariantObject* v, bool readonly)
{
	wxASSERT(dataModel);

	dataModel->SetVariant(v, readonly);
	dataView->Expand(dataModel->RootItem());	
}

DKObject<VariantObject> VariantEditor::GetVariant(void) const
{
	return dataModel->GetVariant();
}

bool VariantEditor::IsReadOnly(void) const
{
	return dataModel->IsReadOnly();
}

void VariantEditor::InitDataView(void)
{
	wxASSERT(dataView == NULL);
	wxASSERT(dataModel == NULL);

	dataModel = new VariantDataModel;
	dataView = new wxDataViewCtrl(this, wxID_ANY, wxPoint(0,0), wxDefaultSize, wxDV_MULTIPLE);
	dataView->AssociateModel(dataModel.get());

	wxArrayString types;

	const int numColumns = sizeof(columnStringValues) / sizeof(VariantColumnTypeInfo);
	for (int i = 0; i < numColumns; ++i)
	{
		types.Add(columnStringValues[i].string);
	}
	
	wxDataViewRenderer *render0 = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE);
	wxDataViewRenderer *render1 = new wxDataViewChoiceRenderer(types, wxDATAVIEW_CELL_EDITABLE, wxALIGN_LEFT);
	wxDataViewRenderer *render2 = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE);

	wxDataViewColumn* column0 = new wxDataViewColumn("key", render0, VariantDataModel::DataViewColumnName, 200, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	wxDataViewColumn* column1 = new wxDataViewColumn("type", render1, VariantDataModel::DataViewColumnType, 100, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
	wxDataViewColumn* column2 = new wxDataViewColumn("value", render2, VariantDataModel::DataViewColumnValue, 200, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);

	dataView->AppendColumn(column0);
	dataView->AppendColumn(column1);
	dataView->AppendColumn(column2);

	enableDragSource = true;
	enableDropTarget = true;

	// 아이템 drag & drop 지원
	if (enableDragSource)
		dataView->EnableDragSource(wxDF_UNICODETEXT);
	if (enableDropTarget)
		dataView->EnableDropTarget(wxDF_UNICODETEXT);
}

void VariantEditor::OnDataViewItemActivated(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);	
}

void VariantEditor::OnDataViewItemContextMenu(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);	

	//wxDataViewItem item = e.GetItem();
	//VariantNode *node = (VariantNode*)item.GetID();
	wxDataViewItemArray selections;
	this->dataView->GetSelections(selections);

	size_t numSel = selections.Count();
	DKSet<VariantNode*> selectedNodes;
	for (size_t i = 0; i < numSel; ++i)
		selectedNodes.Insert(((VariantNode*)selections.Item(i).GetID()));	// 선택된 모든 노드 복사 (빠른 검색용)

	wxMenu menu;
	if (numSel == 1) 
	{
		VariantNode* selectedNode = ((VariantNode*)selections.Item(0).GetID());
		DKVariant::Type valueType = selectedNode->ValueType();

		// 선택된 노드가 map 또는 array 일경우 새 아이템 추가가 가능
		if (valueType == DKVariant::TypePairs)
		{
			menu.Append(wxID_NEW, "New Pair");
			menu.AppendSeparator();
		}
		else if (valueType == DKVariant::TypeArray)
		{
			menu.Append(wxID_NEW, "New Item");
			menu.AppendSeparator();
		}
		else if (valueType == DKVariant::TypeData)
		{
			// 한개만 선택되었고 형식이 DKVariant::TypeData 일 경우, data import 가 가능

			bool enableDataImport = true;
			// export 는 현재 노드가 값을 가지고 있는지 확인..
			bool enableDataExport = false;

			menu.Append(UICommandDataImport, "Import Data From File...");
			menu.Append(UICommandDataExport, "Export Data To File...");
			if (selectedNode->dataValue)
			{
				wxASSERT_MSG(selectedNode->dataValue->Length() > 0, "size cannot be zero.");
				menu.Enable(UICommandDataExport, true);
			}
			else
			{
				menu.Enable(UICommandDataExport, false);
			}
			menu.AppendSeparator();
		}
		else if (valueType == DKVariant::TypeDateTime)
		{
			menu.Append(UICommandDateTimeSetValue, "Set Date Time");
			menu.AppendSeparator();
		}
		if (selectedNode->parent)
		{
			VariantNode* parent = selectedNode->parent;
			if (parent->ValueType() == DKVariant::TypeArray)
			{
				menu.Append(UICommandArrayReorder, "Reorder");
				menu.AppendSeparator();
				menu.Enable(UICommandArrayReorder, parent->children.Count() > 1);
			}
		}
	}

	menu.Append(wxID_CUT, "Cut");
	menu.Append(wxID_COPY, "Copy");
	menu.Append(wxID_PASTE, "Paste");
	menu.Append(wxID_DUPLICATE, "Duplicate");
	menu.AppendSeparator();

	if (numSel == 1)
	{
		VariantNode* selectedNode = ((VariantNode*)selections.Item(0).GetID());
		DKVariant::Type valueType = selectedNode->ValueType();

		menu.Append(UICommandFileImport, "Import...");
		menu.Append(UICommandFileExport, "Export...");
		menu.Enable(UICommandFileExport, valueType != DKVariant::TypeUndefined);
		menu.AppendSeparator();
	}

	menu.Append(wxID_DELETE, "Delete");

	bool enableCut = false;
	bool enableCopy = false;
	bool enablePaste = false;
	bool enableDuplicate = false;
	bool enableDelete = false;

	if (numSel == 1)
	{
		// 한개가 선택되었으며, array, map 일 경우에만 paste 가 가능함. (새 아이템 추가형식으로 붙여넣음)
		VariantNode* selectedNode = ((VariantNode*)selections.Item(0).GetID());
		DKVariant::Type valueType = selectedNode->ValueType();
		if (valueType == DKVariant::TypeArray || valueType == DKVariant::TypePairs)
		{
			// 클립보드에 데이터가 있는지 확인함.
			DKVariant vdata(DKVariant::TypeUndefined);
			if (GetClipboardData(vdata) != size_t(-1))
			{
				if (vdata.ValueType() == DKVariant::TypeArray && vdata.Array().Count() > 0)
					enablePaste = true;
			}
		}
	}
	if (numSel > 0)
	{
		enableCopy = true;

		bool parentSelected = false;
		bool rootSelected = false;

		for (size_t i = 0; i < numSel; ++i)
		{
			VariantNode* node = ((VariantNode*)selections.Item(i).GetID());
			if (node->parent)
			{
				if (parentSelected == false)
				{
					for (VariantNode* n = node->parent; n != NULL; n = n->parent)
					{
						if (selectedNodes.Contains(n))
						{
							parentSelected = true;
							break;
						}
					}
				}
			}
			else
			{
				rootSelected = true;
			}
			if (parentSelected && rootSelected)
				break;
		}

		// delete, duplicate 가능여부 확인
		if (parentSelected == false && rootSelected == false)
		{
			enableDelete = true;
			enableDuplicate = true;
			for (size_t i = 0; i < numSel; ++i)
			{
				VariantNode* node = ((VariantNode*)selections.Item(i).GetID());
				wxASSERT(node->parent);
				// 선택된 것들의 부모가 모두 array, map 일경우에만 duplicate, delete 가 가능
				if (node->parent->ValueType() != DKVariant::TypePairs && node->parent->ValueType() != DKVariant::TypeArray)
				{
					enableDelete = false;
					enableDuplicate = false;
					break;
				}
			}
		}
		// cut 가능여부 확인
		if (parentSelected == false && rootSelected == false)
			enableCut = true;
	}
	menu.Enable(wxID_CUT, enableCut);
	menu.Enable(wxID_COPY, enableCopy);
	menu.Enable(wxID_PASTE, enablePaste);
	menu.Enable(wxID_DUPLICATE, enableDuplicate);
	menu.Enable(wxID_DELETE, enableDelete);

	PopupMenu(&menu, e.GetPosition());
}

void VariantEditor::OnDataViewItemStartEditing(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);	

	if (dataModel->IsReadOnly())
	{
		DKLog("[%s] is read-only. (ignored)\n", DKLIB_FUNCTION_NAME);
		e.Veto();
	}
	else
	{
		wxDataViewItem item = e.GetItem();
		int col = e.GetColumn();
		if (col < 0 && e.GetDataViewColumn() != NULL)
		{
			for (int i = 0; i < this->dataView->GetColumnCount(); ++i)
			{
				if (this->dataView->GetColumn(i) == e.GetDataViewColumn())
				{
					col = i;
					break;
				}
			}
		}
		if (col < 0)
		{
			DKLog("[%s] unknown column! (ignored)\n", DKLIB_FUNCTION_NAME);
			e.Veto();
		}
		else
		{
			VariantNode *node = (VariantNode*)item.GetID();

			bool editable = false;
			switch (col)
			{
			case VariantDataModel::DataViewColumnName:
				editable = node->IsKeyEditable();
				break;
			case VariantDataModel::DataViewColumnType:
				editable = node->IsTypeEditable();
				break;
			case VariantDataModel::DataViewColumnValue:
				editable = node->IsValueEditable() && dataModel->BeginValueEditing();
				break;
			default:
				wxFAIL_MSG("Unknown column!");
			}
			if (editable)
			{
				DKLog("[%s] start editing col:%d\n", DKLIB_FUNCTION_NAME, col);
			}
			else
			{
				DKLog("[%s] column:%d is not editable.\n", DKLIB_FUNCTION_NAME, col);
				e.Veto();
			}
		}
	}
}

void VariantEditor::OnDataViewItemEditingDone(wxDataViewEvent& e)
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);	

	wxASSERT(!dataModel->IsReadOnly());

	wxDataViewItem item = e.GetItem();
	int col = e.GetColumn();
	if (col < 0 && e.GetDataViewColumn() != NULL)
	{
		for (int i = 0; i < this->dataView->GetColumnCount(); ++i)
		{
			if (this->dataView->GetColumn(i) == e.GetDataViewColumn())
			{
				col = i;
				break;
			}
		}
	}
	if (col < 0)
	{
		DKLog("[%s] unknown column! (ignored)\n", DKLIB_FUNCTION_NAME);
		e.Veto();
	}
	else
	{
		VariantNode *node = (VariantNode*)item.GetID();
		if ((col == 0 && !node->IsKeyEditable()) || (col == 2 && !node->IsValueEditable()))
		{
			DKLog("[%s] column:%d is not editable.\n", DKLIB_FUNCTION_NAME, col);
			e.Veto();
		}
		else
		{
			if (e.IsEditCancelled())
			{
				DKLog("[%s] editing cancelled column:%d\n", DKLIB_FUNCTION_NAME, col);
				e.Veto();
			}
			else
			{
				DKLog("[%s] editing done column:%d\n", DKLIB_FUNCTION_NAME, col);

				wxVariant oldValue;
				((wxDataViewModel&)*dataModel).GetValue(oldValue, item, col);
				const wxVariant& value = e.GetValue();

				if (oldValue.GetString().Cmp(value.GetString()))
				{
					DKLog("[%s] change: %ls -> %ls\n", DKLIB_FUNCTION_NAME, (const wchar_t*)oldValue.GetString(), (const wchar_t*)value.GetString());

					bool editable = false;
					switch (col)
					{
					case VariantDataModel::DataViewColumnName:
						editable = node->IsKeyEditable();
						break;
					case VariantDataModel::DataViewColumnType:
						editable = node->IsTypeEditable();
						break;
					case VariantDataModel::DataViewColumnValue:
						editable = node->IsValueEditable();
						break;
					default:
						wxFAIL_MSG("Unknown column!");
					}
					if (editable)
					{
						DKLog("[%s] editing done col:%d\n", DKLIB_FUNCTION_NAME, col);
					}
					else
					{
						wxFAIL_MSG("column is not editable!");
						DKLog("[%s] column:%d is not editable.\n", DKLIB_FUNCTION_NAME, col);
						e.Veto();
					}
				}
				else
				{
					DKLog("[%s] value not changed: %ls -> %ls\n", DKLIB_FUNCTION_NAME, (const wchar_t*)oldValue.GetString(), (const wchar_t*)value.GetString());
					e.Veto();
				}
			}

			if (col == VariantDataModel::DataViewColumnValue)
				this->dataModel->EndValueEditing();
		}
	}
}

void VariantEditor::OnDataViewItemValueChanged(wxDataViewEvent& e)
{
	wxDataViewItem item = e.GetItem();
	int col = e.GetColumn();
	if (col < 0 && e.GetDataViewColumn() != NULL)
	{
		for (int i = 0; i < this->dataView->GetColumnCount(); ++i)
		{
			if (this->dataView->GetColumn(i) == e.GetDataViewColumn())
			{
				col = i;
				break;
			}
		}
	}
	if (item.IsOk())
	{
		VariantNode *node = (VariantNode*)item.GetID();
		DKLog("[%s] item:\"%ls\" type:\"%ls\" value:\"%ls\" (column:%d)\n", DKLIB_FUNCTION_NAME,
			(const wchar_t*)node->key, 
			(const wchar_t*)VariantTypeToString(node->ValueType()),
			(const wchar_t*)node->ValueString(),
			col);

		if (col >= 0)
		{
			DKLog("*** Send notificaiton to parent! ***\n");

			VariantEditorEvent evt(VARIANT_EDITOR_MODIFIED, this->GetId(), this);
			evt.SetEventObject(this);
			ProcessWindowEvent(evt);
		}
	}
	else
	{
		DKLog("[%s] unknown item (column:%d).\n", DKLIB_FUNCTION_NAME, col);
	}
}

void VariantEditor::OnDataViewItemBeginDrag(wxDataViewEvent& e)
{
	if (!enableDragSource)
	{
		e.Veto();
		return;
	}

    wxDataViewItem item(e.GetItem());
	VariantNode *node = (VariantNode*)item.GetID();

	// array 의 노드인지 확인함.
	bool enableDrag = true;
	if (!enableDrag)
	{
		e.Veto();
		return;
	}

	//e.Allow();
	wxTextDataObject* obj = new wxTextDataObject;
	obj->SetText( (const wchar_t*)node->key );
	e.SetDataObject(obj);

	DKLog("%s[Editor:%p] (%ls)\n", DKLIB_FUNCTION_NAME, this, (const wchar_t*)obj->GetText());
}

void VariantEditor::OnDataViewItemDropPossible(wxDataViewEvent& e)
{
	if (!enableDropTarget)
	{
		e.Veto();
		return;
	}

    wxDataViewItem item(e.GetItem());
	VariantNode *node = (VariantNode*)item.GetID();

	DKLog("%s[Editor:%p] (node:%ls)\n", DKLIB_FUNCTION_NAME, this, (const wchar_t*)node->key);

	// 타겟이 array, map 일경우에만 이동이 가능하며, 현재 이동하려는 노드의 자식노드면 안됨.
	bool enableDrag = true;
	if (!enableDrag)
	{
		e.Veto();
		return;
	}
}

void VariantEditor::OnDataViewItemDrop(wxDataViewEvent& e)
{
	if (!enableDropTarget)
	{
		e.Veto();
		return;
	}

	DKLog("%s[Editor:%p]\n", DKLIB_FUNCTION_NAME, this);

    wxDataViewItem item(e.GetItem());
	VariantNode *node = (VariantNode*)item.GetID();

	bool enableDrag = true;
	if (!enableDrag)
	{
		e.Veto();
		return;
	}

    if (e.GetDataFormat() != wxDF_UNICODETEXT)
    {
        e.Veto();
        return;
    }

	wxTextDataObject obj;
    obj.SetData( wxDF_UNICODETEXT, e.GetDataSize(), e.GetDataBuffer() );


	// drop-target 이 map 일 경우 이름을 재사용하거나 변경여부를 확인해야한다.
	// 이름 변경할시 텍스트 박스 띄움.

	// 테스트코드
	wxTextEntryDialog dlg(this, "Enter item key", wxGetTextFromUserPromptStr, obj.GetText());
	if (dlg.ShowModal() == wxID_OK)
	{
		DKLog("%s[Editor:%p] name:%ls change to:%ls\n", DKLIB_FUNCTION_NAME, this, (const wchar_t*)obj.GetText(), (const wchar_t*)dlg.GetValue());
	}
	else
	{
		DKLog("%s[Editor:%p] name change cancelled.\n", DKLIB_FUNCTION_NAME, this, (const wchar_t*)obj.GetText());
	}
}

size_t VariantEditor::GetClipboardData(DKVariant& vout)
{
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDataFormat("DKVariant")))
		{
			wxCustomDataObject data(wxDataFormat("DKVariant"));
			wxTheClipboard->GetData(data);

			DKDataStream dataStream(DKData::StaticData(data.GetData(), data.GetSize()));
			if (vout.ImportStream(&dataStream))
			{
				return data.GetSize();
			}
			else
			{
				DKLog("[%s] DKVariant::ImportStream failed.\n", DKLIB_FUNCTION_NAME);
			}
		}
		else
		{
			DKLog("[%s] DataFormat not supported.\n", DKLIB_FUNCTION_NAME);
		}
		wxTheClipboard->Close();
	}
	else
	{
		DKLog("[%s] Cannot open clipboard.\n", DKLIB_FUNCTION_NAME);
	}
	return size_t(-1);
}

size_t VariantEditor::SetClipboardData(DKVariant& vin)
{
	DKBufferStream stream;
	if (vin.ExportStream(&stream))
	{
		const DKData* streamData = stream.DataSource();
		if (streamData && streamData->Length() > 0)
		{
			if (wxTheClipboard->Open())
			{
				wxCustomDataObject* data = new wxCustomDataObject(wxDataFormat("DKVariant"));
				data->SetData(streamData->Length(), streamData->LockShared());
				streamData->UnlockShared();

				wxTheClipboard->SetData(data);

				wxTheClipboard->Close();
				DKLog("[%s] %llu bytes copied.\n", DKLIB_FUNCTION_NAME, streamData->Length());

				return streamData->Length();
			}
			else
			{
				DKLog("[%s] Cannot open clipboard.\n", DKLIB_FUNCTION_NAME);
			}
		}
		else
		{
			DKLog("[%s] failed to export variant.\n", DKLIB_FUNCTION_NAME);
		}
	}
	else
	{
		DKLog("[%s] failed to export variant.\n", DKLIB_FUNCTION_NAME);
	}
	return size_t(-1);
}
