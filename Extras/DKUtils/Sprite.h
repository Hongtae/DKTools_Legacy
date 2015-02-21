#pragma once
#include <DK.h>
#include "TexturePack.h"

////////////////////////////////////////////////////////////////////////////////
//
// Sprite
//
// 2D 스프라이트 객체, 버튼으로도 사용 가능함
//
////////////////////////////////////////////////////////////////////////////////

namespace DKUtils
{
	struct SpriteAnimKey1
	{
		SpriteAnimKey1(void) : time(0), value(0) {}
		SpriteAnimKey1(float t, float v) : time(t), value(v) {}
		float time;
		float value;
	};
	struct SpriteAnimKey2
	{
		SpriteAnimKey2(void) : time(0), value1(0), value2(0) {}
		SpriteAnimKey2(float t, float v1, float v2) : time(t), value1(v1), value2(v2) {}
		float time;
		float value1;
		float value2;
	};
	struct SpriteAnimKey3
	{
		SpriteAnimKey3(void) : time(0), value1(0), value2(0), value3(0) {}
		SpriteAnimKey3(float t, float v1, float v2, float v3) : time(t), value1(v1), value2(v2), value3(v3) {}
		float time;
		float value1;
		float value2;
		float value3;
	};

	template <typename KEY>
	class SpriteAnimation
	{
	public:
		enum Interpolate
		{
			Linear = 0,
			CatmullRom,
			UniformCubic,
			Bezier,				// 4 개씩 끊어서 보간한다. 0-1-2-3 / 3-4-5-6 / 6-7-8-9 ... 로 보간함
		};
		typedef DKFoundation::DKOrderedArray<KEY> KeyFrameArray;
		typedef typename KeyFrameArray::Index ArrayIndex;

		SpriteAnimation(Interpolate intp = Linear)
			: keys(SpriteAnimKeyOrder), interpolate(intp)
		{
		}
		bool AnimatedKey(KEY& k) const
		{
			size_t numKeys = keys.Count();
			if (numKeys > 0)
			{
				if (numKeys > 1)	// 곡선은 시작점,끝점이 다를 수 있으므로 보간을 꼭 해야한다. (UniformCubic 의 경우 특히)
				{
					ArrayIndex index = keys.FindApprox(k, false);	// k.time 보다 같거나 작은거 찾음 (n, n+1 사이의 값이 된다)
					if (index == keys.invalidIndex)		// 맨 처음꺼
					{
						InterpolateKey(0, 1, numKeys - 1, k);
					}
					else if (index + 1 == numKeys)		// 맨 마지막꺼
					{
						InterpolateKey(index-1, index, numKeys - 1, k);
					}
					else								// index 와 index+1 사이의 값
					{
						InterpolateKey(index, index+1, numKeys - 1, k);
					}
				}
				else
				{
					k = keys.Value(0);
				}
				return true;
			}
			return false;
		}
		float Duration(void) const
		{
			size_t numKeys = keys.Count();
			if (numKeys > 0)
			{
				return keys.Value(numKeys-1).time;
			}
			return 0.0;
		}
		KeyFrameArray keys;
		Interpolate interpolate;

	private:
		static bool SpriteAnimKeyEqual(const KEY& lhs, const KEY& rhs) {return lhs.time == rhs.time;}
		static bool SpriteAnimKeyOrder(const KEY& lhs, const KEY& rhs) {return lhs.time < rhs.time;}
		void InterpolateKey(ArrayIndex idx1, ArrayIndex idx2, ArrayIndex last, KEY& k) const
		{
			using namespace DKFoundation;
			switch (interpolate)
			{
			case Linear:
				{
					const KEY& key0 = keys.Value(idx1);
					const KEY& key1 = keys.Value(idx2);

					float start = key0.time;
					float length = key1.time - start;
					LinearInterpolate(key0, key1, Clamp<float>((k.time - start) / length, 0.0, 1.0), interpolate, k);
				}
				break;
			case CatmullRom:
			case UniformCubic:
				{
					int index0 = idx1 > 0 ? idx1 - 1 : 0;
					int index1 = idx1;
					int index2 = idx2;
					int index3 = idx2 + 1 < last ? idx2 + 1 : last;

					const KEY& key0 = keys.Value(index0);
					const KEY& key1 = keys.Value(index1);
					const KEY& key2 = keys.Value(index2);
					const KEY& key3 = keys.Value(index3);

					float start = key1.time;
					float length = key2.time - start;
					SplineInterpolate(key0, key1, key2, key3, Clamp<float>((k.time - start) / length, 0.0, 1.0), interpolate, k);
				}
				break;
			case Bezier:
				{
					int d1 = idx1 / 3;

					int index0 = d1 * 3;
					int index1 = Min<int>(d1 * 3 + 1, last);
					int index2 = Min<int>(d1 * 3 + 2, last);
					int index3 = Min<int>(d1 * 3 + 3, last);

					const KEY& key0 = keys.Value(index0);
					const KEY& key1 = keys.Value(index1);
					const KEY& key2 = keys.Value(index2);
					const KEY& key3 = keys.Value(index3);

					float start = key0.time;
					float length = key3.time - start;
					SplineInterpolate(key0, key1, key2, key3, Clamp<float>((k.time - start) / length, 0.0, 1.0), interpolate, k);
				}
				break;
			}
		}
		static void LinearInterpolate(const SpriteAnimKey1& k1, const SpriteAnimKey1& k2, float t, Interpolate p, SpriteAnimKey1& out)
		{
			out.value = k1.value * (1.0f - t) + k2.value * t;
		}
		static void LinearInterpolate(const SpriteAnimKey2& k1, const SpriteAnimKey2& k2, float t, Interpolate p, SpriteAnimKey2& out)
		{
			out.value1 = k1.value1 * (1.0f - t) + k2.value1 * t;
			out.value2 = k1.value2 * (1.0f - t) + k2.value2 * t;
		}
		static void LinearInterpolate(const SpriteAnimKey3& k1, const SpriteAnimKey3& k2, float t, Interpolate p, SpriteAnimKey3& out)
		{
			out.value1 = k1.value1 * (1.0f - t) + k2.value1 * t;
			out.value2 = k1.value2 * (1.0f - t) + k2.value2 * t;
			out.value3 = k1.value3 * (1.0f - t) + k2.value3 * t;
		}
		static void SplineInterpolate(const SpriteAnimKey1& k1, const SpriteAnimKey1& k2, const SpriteAnimKey1& k3, const SpriteAnimKey1& k4, float t, Interpolate p, SpriteAnimKey1& out)
		{
			using namespace DKFramework;
			DKSpline spline(k1.value, k2.value, k3.value, k4.value);
			float result = 0.0f;
			switch (p)
			{
			case CatmullRom:
				result = spline.Interpolate(t, DKSpline::CatmullRom);
				break;
			case UniformCubic:
				result = spline.Interpolate(t, DKSpline::UniformCubic);
				break;
			case Bezier:
				result = spline.Interpolate(t, DKSpline::Bezier);
				break;
			default:
				break;
			}
			out.value = result;
		}
		static void SplineInterpolate(const SpriteAnimKey2& k1, const SpriteAnimKey2& k2, const SpriteAnimKey2& k3, const SpriteAnimKey2& k4, float t, Interpolate p, SpriteAnimKey2& out)
		{
			using namespace DKFramework;
			DKSpline2 spline(DKVector2(k1.value1, k1.value2), DKVector2(k2.value1, k2.value2), DKVector2(k3.value1, k3.value2), DKVector2(k4.value1, k4.value2));
			DKVector2 result(0,0);
			switch (p)
			{
			case CatmullRom:
				result = spline.Interpolate(t, DKSpline::CatmullRom);
				break;
			case UniformCubic:
				result = spline.Interpolate(t, DKSpline::UniformCubic);
				break;
			case Bezier:
				result = spline.Interpolate(t, DKSpline::Bezier);
				break;
			default:
				break;
			}
			out.value1 = result.x;
			out.value2 = result.y;
		}
		static void SplineInterpolate(const SpriteAnimKey3& k1, const SpriteAnimKey3& k2, const SpriteAnimKey3& k3, const SpriteAnimKey3& k4, float t, Interpolate p, SpriteAnimKey3& out)
		{
			using namespace DKFramework;
			DKSpline3 spline(DKVector3(k1.value1, k1.value2, k1.value3), DKVector3(k2.value1, k2.value2, k2.value3), DKVector3(k3.value1, k3.value2, k3.value3), DKVector3(k4.value1, k4.value2, k4.value3));
			DKVector3 result(0,0,0);
			switch (p)
			{
			case CatmullRom:
				result = spline.Interpolate(t, DKSpline::CatmullRom);
				break;
			case UniformCubic:
				result = spline.Interpolate(t, DKSpline::UniformCubic);
				break;
			case Bezier:
				result = spline.Interpolate(t, DKSpline::Bezier);
				break;
			default:
				break;
			}
			out.value1 = result.x;
			out.value2 = result.y;
			out.value3 = result.z;
		}
	};

	class Sprite
	{
	public:
		template <typename KEY>
		struct AnimatedValue
		{
			AnimatedValue(void) : speed(1.0), repeat(0) {}
			typedef SpriteAnimation<KEY> Animation;
			DKFoundation::DKObject<Animation> anim;
			KEY key;
			float speed;
			int repeat;
			DKFoundation::DKObject<DKFoundation::DKOperation> finishCallback;
			void SetAnimation(Animation* anim, int repeat, float speed, DKFoundation::DKOperation* callback)
			{
				this->anim = anim;
				this->repeat = repeat;
				this->speed = speed;
				this->key.time = 0.0;
				this->finishCallback = callback;
			}
		};
		struct AnimValue1 : public AnimatedValue<SpriteAnimKey1>
		{
			float Value(void) const {return key.value;}
			void SetValue(float f)	{key.value = f;}
		};
		struct AnimValue2 : public AnimatedValue<SpriteAnimKey2>
		{
			DKFramework::DKVector2 Value(void) const			{return DKFramework::DKVector2(key.value1, key.value2);}
			void SetValue(const DKFramework::DKVector2& v)		{key.value1 = v.x; key.value2 = v.y;}
			void SetValue(float v1, float v2)					{key.value1 = v1; key.value2 = v2;}
		};
		struct AnimValue3 : public AnimatedValue<SpriteAnimKey3>
		{
			DKFramework::DKVector3 Value(void) const			{return DKFramework::DKVector3(key.value1, key.value2, key.value3);}
			void SetValue(const DKFramework::DKVector3& v)		{key.value1 = v.x; key.value2 = v.y; key.value3 = v.z;}
			void SetValue(float v1, float v2, float v3)			{key.value1 = v1; key.value2 = v2; key.value3 = v3;}
		};

		enum State
		{
			StateNormal = 0,
			StateHighlighted,
			StateDisabled,
		};

		typedef DKFoundation::DKArray<DKFoundation::DKString> TextureKeyArray;

		Sprite(TexturePack* tex, const DKFramework::DKPoint& center, const DKFramework::DKSize& size);
		Sprite(TexturePack* tex, const DKFoundation::DKString& key, const DKFramework::DKPoint& center, const DKFramework::DKSize& size);
		~Sprite(void);

		void Update(double timeDelta, DKFoundation::DKTimeTick tick);
		void Render(const DKFramework::DKRenderer& renderer, const DKFramework::DKMatrix3& tm, const DKFramework::DKColor& color = DKFramework::DKColor(1,1,1,1)) const;

		// 마우스 이벤트의 pos 는 부모좌표계로 넣어야 함.
		void MouseDown(int deviceId, const DKFramework::DKPoint& pos);
		void MouseUp(int deviceId, const DKFramework::DKPoint& pos);
		void MouseMove(int deviceId, const DKFramework::DKPoint& pos);

		void ResetMouse(void);

		DKFramework::DKPoint ConvertPoint(const DKFramework::DKPoint& pt) const;
		bool IsPointInside(const DKFramework::DKPoint& pt) const;

		// 중심점 위치 = offset + center
		// 크기 = size * scale
		AnimValue2 center;
		AnimValue2 offset;
		AnimValue2 size;
		AnimValue1 scale;
		AnimValue1 rotate;
		AnimValue3 diffuse;
		AnimValue1 alpha;
		AnimValue1 textureIndex;

		bool	hidden;
		State	state;

		DKFoundation::DKString name;
		DKFoundation::DKObject<DKFoundation::DKOperation> highlightAction;
		DKFoundation::DKObject<DKFoundation::DKOperation> buttonAction;
		DKFoundation::DKMap<State, TextureKeyArray> textures;

		Sprite* FindDescendant(const DKFoundation::DKString& name);
		const Sprite* FindDescendant(const DKFoundation::DKString& name) const;
		bool IsDescendantOf(const Sprite* s) const;
		Sprite* ParentNode(void);
		const Sprite* ParentNode(void) const;
		size_t NumberOfChildren(void) const;
		Sprite* ChildNodeAtIndex(unsigned int index);
		const Sprite* ChildNodeAtIndex(unsigned int index) const;
		void RemoveFromParent(void);
		void AddChild(Sprite* s);

		void OrderFront(void);
		void OrderBack(void);

		void SetHidden(bool h) {this->hidden = h;}
		void SetState(State s) {this->state = s;}

	private:
		DKFramework::DKMatrix3 transform;
		DKFramework::DKMatrix3 inverseTransform;

		Sprite* parent;
		typedef DKFoundation::DKArray<DKFoundation::DKObject<Sprite>> SpriteArray;
		SpriteArray children;
		int mouseDeviceId;
		DKFoundation::DKObject<TexturePack> textureAtlas;
		struct ColorFrameList
		{
			const TexturePack* texture;
			TexturePack::DisplayFrameList frameList;
			unsigned char color[4];
		};
		mutable ColorFrameList displayCache;

		DKFoundation::DKOperation* AscendantAction(void);
		// 모든 노드를 돌면서 같은 텍스쳐와 같은 색상일 경우 합쳐서 그린다.
		void RenderFrame(const DKFramework::DKRenderer& renderer, const DKFramework::DKMatrix3& tm, const DKFramework::DKColor& color, ColorFrameList& frames) const;
	};

	typedef Sprite::AnimValue1::Animation SpriteAnimation1;
	typedef Sprite::AnimValue2::Animation SpriteAnimation2;
	typedef Sprite::AnimValue3::Animation SpriteAnimation3;
}
