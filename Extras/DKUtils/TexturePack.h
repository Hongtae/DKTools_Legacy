#pragma once
#include <DK.h>

////////////////////////////////////////////////////////////////////////////////
//
// TexturePack 
//
// TexturePack 을 이용해서 생성한 Texture Atlas 를 읽어들임. (plist/xml 만 가능)
//
////////////////////////////////////////////////////////////////////////////////

namespace DKUtils
{
	class TexturePack
	{
	public:
		struct Frame
		{
			DKFramework::DKMatrix3	texTM;			// uv 좌표 행렬
			DKFramework::DKRect		offset;			// 위치값의 offset, (위치 rect.origin = rect.origin + offset.origin / rect.size, rect.size = rect.size * offset.size)
			DKFramework::DKSize		resolution;		// 원본 크기 픽셀 사이즈
		};
		
		typedef DKFoundation::DKArray<DKFramework::DKRenderer::Vertex2D> DisplayFrameList;
		
		~TexturePack(void);

		static DKFoundation::DKObject<TexturePack> Create(const DKFoundation::DKXMLElement* plist, DKFramework::DKResourcePool* pool);
		static DKFoundation::DKObject<TexturePack> Create(DKFoundation::DKStream* stream, DKFramework::DKResourcePool* pool);
		static DKFoundation::DKObject<TexturePack> Create(DKFramework::DKTexture2D* tex, const DKFoundation::DKString& name);

		DKFramework::DKSize TextureSize(const DKFoundation::DKString& key) const;		
		bool AddDisplayFrame(const DKFramework::DKRect& posRect, const DKFramework::DKMatrix3& posTM, const DKFramework::DKRect& texRect, const DKFramework::DKMatrix3& texTM, const DKFoundation::DKString& key, DisplayFrameList& verts) const;
		void RenderDisplayFrame(const DisplayFrameList& verts, const DKFramework::DKRenderer* renderer, const DKFramework::DKColor& color, const DKFramework::DKBlendState& blend) const;
		
	private:
		TexturePack(void);
		typedef DKFoundation::DKMap<DKFoundation::DKString, Frame> FrameMap;
		DKFoundation::DKObject<DKFramework::DKTexture2D> texture;
		FrameMap frames;
		DKFoundation::DKArray<DKFramework::DKRenderer::Vertex2D> vertices;
		friend class DKFoundation::DKObject<TexturePack>;
	};
}
