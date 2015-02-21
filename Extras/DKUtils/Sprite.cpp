#include "Sprite.h"
#include <math.h>

using namespace DKFoundation;
using namespace DKFramework;

namespace DKUtils
{
	namespace
	{
		template <typename KEY> void UpdateAnimatedValue(Sprite::AnimatedValue<KEY>& v, float t)
		{
			if (v.anim)
			{
				DKObject<DKOperation> op = NULL;

				v.key.time += Max<float>(t * v.speed, 0.0f);
				if (v.anim->AnimatedKey(v.key))
				{
					float duration = v.anim->Duration();
					if (v.key.time >= duration)
					{
						if (v.repeat > 0 && duration > 0)
						{
							int d = (int)floor(v.key.time / duration);
							if (v.repeat < d)
							{
								op = v.finishCallback;
								v.anim = NULL;
								v.repeat = 0;
								v.key.time = 0;
								v.finishCallback = NULL;
							}
							else
							{
								v.key.time -= duration * d;
								v.repeat -= d;

								DKASSERT_DESC_DEBUG(v.key.time >= -0.0, "key must be positive");
								DKASSERT_DESC_DEBUG(v.repeat >= 0, "repeat cannot be negative");
							}
						}
						else
						{
							op = v.finishCallback;
							v.anim = NULL;
							v.repeat = 0;
							v.key.time = 0;
							v.finishCallback = NULL;
						}
					}
				}
				else
				{
					op = v.finishCallback;
					v.anim = NULL;
					v.repeat = 0;
					v.key.time = 0;
					v.finishCallback = NULL;
				}

				if (op)
					DKRunLoop::CurrentRunLoop()->PostOperation(op, 0);
			}
		}
	}
}

using namespace DKUtils;

Sprite::Sprite(TexturePack* tex, const DKPoint& center, const DKSize& size)
	: textureAtlas(tex)
	, transform(DKMatrix3::identity)
	, inverseTransform(DKMatrix3::identity)
	, hidden(false)
	, state(StateNormal)
	, mouseDeviceId(-1)
	, parent(NULL)
{
	this->center.key = SpriteAnimKey2(0,center.x,center.y);
	this->offset.key = SpriteAnimKey2(0,0,0);
	this->size.key = SpriteAnimKey2(0,size.width,size.height);
	this->scale.key = SpriteAnimKey1(0,1);
	this->rotate.key = SpriteAnimKey1(0,0);
	this->diffuse.key = SpriteAnimKey3(0,1,1,1);
	this->alpha.key = SpriteAnimKey1(0,1);
	this->textureIndex.key = SpriteAnimKey1(0,0);

	DKAffineTransform2 trans;
	trans.Translate(this->size.Value() * -0.5f);
	trans.Multiply(DKLinearTransform2().Scale(this->scale.Value()).Rotate(this->rotate.Value()));
	trans.Translate(this->offset.Value() + this->center.Value());

	transform = trans.Matrix3();
	trans.Inverse();
	inverseTransform = trans.Matrix3();
}

Sprite::Sprite(TexturePack* tex, const DKString& key, const DKPoint& center, const DKSize& size)
	: textureAtlas(tex)
	, transform(DKMatrix3::identity)
	, inverseTransform(DKMatrix3::identity)
	, hidden(false)
	, state(StateNormal)
	, mouseDeviceId(-1)
	, parent(NULL)
{
	this->center.key = SpriteAnimKey2(0,center.x,center.y);
	this->offset.key = SpriteAnimKey2(0,0,0);
	this->size.key = SpriteAnimKey2(0,size.width,size.height);
	this->scale.key = SpriteAnimKey1(0,1);
	this->rotate.key = SpriteAnimKey1(0,0);
	this->diffuse.key = SpriteAnimKey3(0,1,1,1);
	this->alpha.key = SpriteAnimKey1(0,1);
	this->textureIndex.key = SpriteAnimKey1(0,0);
	this->textures.Value(StateNormal).Add(key);
	this->textures.Value(StateHighlighted).Add(key);
	this->textures.Value(StateDisabled).Add(key);

	DKAffineTransform2 trans;
	trans.Translate(this->size.Value() * -0.5f);
	trans.Multiply(DKLinearTransform2().Scale(this->scale.Value()).Rotate(this->rotate.Value()));
	trans.Translate(this->offset.Value() + this->center.Value());

	transform = trans.Matrix3();
	trans.Inverse();
	inverseTransform = trans.Matrix3();
}

Sprite::~Sprite(void)
{
	for (size_t i = 0; i < children.Count(); ++i)
	{
		children.Value(i)->parent = NULL;
	}
}

void Sprite::Update(double timeDelta, DKTimeTick tick)
{
	if (hidden || state == StateDisabled)
		mouseDeviceId = -1;

	UpdateAnimatedValue(center, timeDelta);
	UpdateAnimatedValue(offset, timeDelta);
	UpdateAnimatedValue(size, timeDelta);
	UpdateAnimatedValue(scale, timeDelta);
	UpdateAnimatedValue(rotate, timeDelta);
	UpdateAnimatedValue(diffuse, timeDelta);
	UpdateAnimatedValue(alpha, timeDelta);
	UpdateAnimatedValue(textureIndex, timeDelta);

	DKAffineTransform2 trans;
	trans.Translate(this->size.Value() * -0.5f);
	trans.Multiply(DKLinearTransform2().Scale(this->scale.Value()).Rotate(this->rotate.Value()));
	trans.Translate(this->offset.Value() + this->center.Value());

	transform = trans.Matrix3();
	trans.Inverse();
	inverseTransform = trans.Matrix3();

	for (size_t i = 0; i < children.Count(); ++i)
		children.Value(i)->Update(timeDelta, tick);
}

void Sprite::Render(const DKRenderer& renderer, const DKMatrix3& tm, const DKColor& color) const
{
	displayCache.texture = NULL;
	displayCache.frameList.Clear();

	RenderFrame(renderer, tm, color, displayCache);

	if (displayCache.texture && displayCache.frameList.Count() > 0)
	{
		displayCache.texture->RenderDisplayFrame(displayCache.frameList, &renderer,
			DKColor(static_cast<float>(displayCache.color[0]) / 255.0,
			static_cast<float>(displayCache.color[1]) / 255.0,
			static_cast<float>(displayCache.color[2]) / 255.0,
			static_cast<float>(displayCache.color[3]) / 255.0),
			DKBlendState::defaultAlpha);		
	}
	displayCache.texture = NULL;
	displayCache.frameList.Clear();	
}

void Sprite::RenderFrame(const DKRenderer& renderer, const DKMatrix3& tm, const DKColor& color, ColorFrameList& frames) const
{
	if (hidden)
		return;

	DKVector3 v = this->diffuse.Value();
	DKColor color2 = DKColor(color.r * v.x, color.g * v.y, color.b * v.z, color.a * this->alpha.Value());
	unsigned char cc[4] = {
		Clamp<unsigned char>(color2.r * 255.0f, 0, 0xff),
		Clamp<unsigned char>(color2.g * 255.0f, 0, 0xff),
		Clamp<unsigned char>(color2.b * 255.0f, 0, 0xff),
		Clamp<unsigned char>(color2.a * 255.0f, 0, 0xff)
	};
	if (cc[3] > 0)
	{
		DKMatrix3 m = transform * tm;		
		if (textureAtlas)
		{
			const DKMap<State, TextureKeyArray>::Pair* p = textures.Find(this->state);
			if (p && p->value.Count() > 0)
			{
				if (frames.texture != this->textureAtlas || reinterpret_cast<unsigned int*>(&frames.color)[0] != reinterpret_cast<unsigned int*>(cc)[0])
				{
					if (frames.texture && frames.frameList.Count() > 0)
					{
						frames.texture->RenderDisplayFrame(frames.frameList, &renderer,
							DKColor(static_cast<float>(frames.color[0]) / 255.0,
							static_cast<float>(frames.color[1]) / 255.0,
							static_cast<float>(frames.color[2]) / 255.0,
							static_cast<float>(frames.color[3]) / 255.0),
							DKBlendState::defaultAlpha);
					}
					frames.frameList.Clear();
					frames.texture = this->textureAtlas;
					reinterpret_cast<unsigned int*>(frames.color)[0] = reinterpret_cast<unsigned int*>(cc)[0];
				}				

				int texIndex = static_cast<int>(textureIndex.key.value) % static_cast<int>(p->value.Count());

				float w = size.key.value1;
				float h = size.key.value2;

				textureAtlas->AddDisplayFrame(DKRect(0, 0, w, h), m, DKRect(0,0,1,1), DKMatrix3::identity, p->value.Value(texIndex), frames.frameList);
			}
		}

		// 자식 프레임은 역순으로 그림
		for (size_t n = children.Count(); n > 0; n--)
		{
			children.Value(n - 1)->RenderFrame(renderer, m, color2, frames);
		}
	}
}

DKPoint Sprite::ConvertPoint(const DKPoint& pt) const
{
	return pt.Vector().Transform(inverseTransform);
}

bool Sprite::IsPointInside(const DKPoint& pt) const
{
	float w = size.key.value1;
	float h = size.key.value2;
	return DKRect(0, 0, w, h).IsPointInside(pt);
}

DKOperation* Sprite::AscendantAction(void)
{
	if (hidden || state == StateDisabled)
		return NULL;

	if (buttonAction)
		return buttonAction;

	if (parent)
		return parent->AscendantAction();
	return NULL;
}

void Sprite::MouseDown(int deviceId, const DKPoint& pos)
{
	if (mouseDeviceId < 0 && state == StateNormal)
	{
		if (this->IsPointInside(pos))
		{
			for (size_t i = 0; i < children.Count(); ++i)
			{
				Sprite* s = children.Value(i);
				if (s->mouseDeviceId < 0 && s->state == StateNormal && s->hidden == false)
				{
					DKPoint p = s->ConvertPoint(pos);
					if (s->IsPointInside(p))
					{
						s->MouseDown(deviceId, p);
						break;
					}
				}
			}

			state = StateHighlighted;
			mouseDeviceId = deviceId;
			if (highlightAction)
				DKRunLoop::CurrentRunLoop()->PostOperation(highlightAction);
		}
	}
}

void Sprite::MouseUp(int deviceId, const DKPoint& pos)
{
	if (mouseDeviceId == deviceId)
	{
		if (this->state == StateHighlighted)
		{
			if (this->IsPointInside(pos))
			{
				if (buttonAction)
					DKRunLoop::CurrentRunLoop()->PostOperation(buttonAction);
			}
			this->state = StateNormal;
		}
		mouseDeviceId = -1;
	}

	for (size_t i = 0; i < children.Count(); ++i)
	{
		Sprite* s = children.Value(i);
		s->MouseUp(deviceId, s->ConvertPoint(pos));
	}
}

void Sprite::MouseMove(int deviceId, const DKPoint& pos)
{
	if (mouseDeviceId == deviceId)
	{
		bool inside = this->IsPointInside(pos);
		if (state == StateNormal)
		{
			if (inside)
				state = StateHighlighted;
		}
		else if (state == StateHighlighted)
		{
			if (inside == false)
				state = StateNormal;
		}
	}

	for (size_t i = 0; i < children.Count(); ++i)
	{
		Sprite* s = children.Value(i);
		s->MouseMove(deviceId, s->ConvertPoint(pos));
	}
}

void Sprite::ResetMouse(void)
{
	mouseDeviceId = -1;
	if (state != StateDisabled)
		state = StateNormal;

	for (size_t i = 0; i < children.Count(); ++i)
	{
		children.Value(i)->ResetMouse();
	}
}

Sprite* Sprite::FindDescendant(const DKString& name)
{
	if (this->name == name)
		return this;

	for (size_t i = 0; i < children.Count(); ++i)
	{
		Sprite* s = children.Value(i)->FindDescendant(name);
		if (s)
			return s;
	}
	return NULL;
}

const Sprite* Sprite::FindDescendant(const DKString& name) const
{
	if (this->name == name)
		return this;

	for (size_t i = 0; i < children.Count(); ++i)
	{
		const Sprite* s = children.Value(i)->FindDescendant(name);
		if (s)
			return s;
	}
	return NULL;
}

bool Sprite::IsDescendantOf(const Sprite* s) const
{
	if (s == this)
		return true;

	if (parent)
		return parent->IsDescendantOf(s);

	return false;
}

Sprite* Sprite::ParentNode(void)
{
	return parent;
}

const Sprite* Sprite::ParentNode(void) const
{
	return parent;
}

size_t Sprite::NumberOfChildren(void) const
{
	return children.Count();
}

Sprite* Sprite::ChildNodeAtIndex(unsigned int index)
{
	return children.Value(index);
}

const Sprite* Sprite::ChildNodeAtIndex(unsigned int index) const
{
	return children.Value(index);
}

void Sprite::RemoveFromParent(void)
{
	if (parent)
	{
		for (size_t i = 0; i < parent->children.Count(); ++i)
		{
			if (parent->children.Value(i) == this)
			{
				parent->children.Remove(i);
				return;
			}
		}
	}
}

void Sprite::AddChild(Sprite* s)
{
	if (s && s != this)
	{
		DKObject<Sprite> sp = s;	// 부모에게서 삭제될때 객체가 지워지는거 막기 위함
		sp->RemoveFromParent();
		sp->parent = this;
		children.Insert(sp, 0);		// 맨 앞에 붙여야 위로 간다.
	}
}

void Sprite::OrderFront(void)
{
	if (parent)
	{
		for (size_t i = 0; i < parent->children.Count(); ++i)
		{
			if (parent->children.Value(i) == this)
			{
				DKObject<Sprite> obj = parent->children.Value(i);
				parent->children.Remove(i);
				parent->children.Insert(obj, 0);	// 맨 앞에 추가
				return;
			}
		}
	}
}

void Sprite::OrderBack(void)
{
	if (parent)
	{
		for (size_t i = 0; i < parent->children.Count(); ++i)
		{
			if (parent->children.Value(i) == this)
			{
				DKObject<Sprite> obj = parent->children.Value(i);
				parent->children.Remove(i);
				parent->children.Add(obj);		// 맨 뒤에 추가
				return;
			}
		}		
	}
}
