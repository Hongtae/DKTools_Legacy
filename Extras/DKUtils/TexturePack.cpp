#include "TexturePack.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKUtils;


TexturePack::TexturePack(void)
{
}

TexturePack::~TexturePack(void)
{
}

DKObject<TexturePack> TexturePack::Create(DKTexture2D* tex, const DKString& name)
{
	if (tex)
	{
		const Frame frame = {DKMatrix3::identity, DKRect(0,0,1,1), tex->Resolution()};
		DKObject<TexturePack> pack = DKObject<TexturePack>::New();
		pack->texture = tex;
		pack->frames.Update(name, frame);
		return pack;
	}
	return NULL;
}

DKObject<TexturePack> TexturePack::Create(const DKXMLElement* plist, DKResourcePool* pool)
{
	if (plist->name.Compare(L"plist") == 0)
	{
		for (size_t i = 0; i < plist->nodes.Count(); ++i)
		{
			if (plist->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement &&
				plist->nodes.Value(i).SafeCast<DKXMLElement>()->name.Compare(L"dict") == 0)
			{
				// dict
				const DKXMLElement* dict = plist->nodes.Value(i).SafeCast<DKXMLElement>();
				
				struct DictionaryParser		// dict 의 <key>와 다음의 element 를 파싱함
				{
					typedef DKMap<DKString, const DKXMLElement*> Dict;
					DictionaryParser(Dict& d, const DKXMLElement* e) : dict(d)
					{
						if (e->name.Compare(L"dict") == 0)
						{
							for (size_t i = 0; i < e->nodes.Count(); ++i)
							{
								if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement &&
									e->nodes.Value(i).SafeCast<DKXMLElement>()->name.Compare(L"key") == 0)
								{
									const DKXMLElement* key = e->nodes.Value(i).SafeCast<DKXMLElement>();
									DKString keyString = L"";
									for (size_t j = 0; j < key->nodes.Count(); ++j)
									{
										if (key->nodes.Value(j)->Type() == DKXMLNode::NodeTypePCData)
										{
											keyString += key->nodes.Value(j).SafeCast<DKXMLPCData>()->value;
										}
									}
									if (keyString.Length() > 0)
									{
										// 바로 다음 Element 찾음
										for (i++; i < e->nodes.Count(); ++i)
										{
											if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement)
											{
												dict.Insert(keyString, e->nodes.Value(i).SafeCast<DKXMLElement>());
												break;
											}
										}
									}
								}
							}
						}
					}
					Dict& dict;
				};
				
				DKMap<DKString, const DKXMLElement*> dictionary;
				DictionaryParser p(dictionary, dict);
				
				// dictionary 로부터 파싱
				struct FrameParser
				{
					struct PixelFrame
					{
						DKString file;
						bool	rotated;	// true 면 시계방향으로 90도 회전함
						DKRect	frame;		// 텍스쳐에서의 위치 (회전한 후, 유효영역)
						DKRect	inset;		// 실제 이미지 좌표상에서의 유효 영역
						DKSize	size;		// 실제 이미지 크기
					};
					typedef DKMap<DKString, const DKXMLElement*> Dict;
					FrameParser(const Dict& dict) : result(false)
					{
						bool frames = false;
						bool metadata = false;

						dict.EnumerateForward([&](const Dict::Pair& pair)
											  {
												  if (pair.key.Compare(L"frames") == 0)
												  {
													  if (frames == false)
														  frames = ParseFrames(pair.value);
												  }
												  else if (pair.key.Compare(L"metadata") == 0)
												  {
													  if (metadata == false)
														  metadata = ParseMetadata(pair.value);
												  }
											  });
						
						if (frames && metadata && atlasFile.Length() > 0 && atlasSize.width > 0 && atlasSize.height > 0)
						{
							result = true;
						}
					}
					DKString GetPCData(const DKXMLElement* e) const
					{
						DKString ret = L"";
						for (size_t i = 0; i < e->nodes.Count(); ++i)
						{
							if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypePCData)
								ret += e->nodes.Value(i).SafeCast<DKXMLPCData>()->value;
						}
						return ret;
					}
					DKRect ParseRect(const DKString& str) const
					{
						DKRect rc;
						DKString str2(str);
						str2.Replace(L"{", NULL);
						str2.Replace(L"}", NULL);
						DKString::IntegerArray nums = str2.ToIntegerArray(L",");
						if (nums.Count() > 3)
						{
							rc = DKRect(nums.Value(0), nums.Value(1), nums.Value(2), nums.Value(3));
						}
						return rc;
					}
					DKSize ParseSize(const DKString& str) const
					{
						DKSize s;
						
						DKString str2(str);
						str2.Replace(L"{", NULL);
						str2.Replace(L"}", NULL);
						DKString::IntegerArray nums = str2.ToIntegerArray(L",");
						if (nums.Count() > 1)
						{
							s = DKSize(nums.Value(0), nums.Value(1));
						}
						return s;
					}
					bool ParseFrameInfo(const DKXMLElement* dict, PixelFrame& pf)
					{
						if (dict->name.Compare(L"dict") == 0)
						{
							Dict tmp;
							DictionaryParser p(tmp, dict);
							Dict::Pair* pFrame = tmp.Find(L"frame");
							Dict::Pair* pRotated = tmp.Find(L"rotated");
							Dict::Pair* pSrcRect = tmp.Find(L"sourceColorRect");
							Dict::Pair* pSrcSize = tmp.Find(L"sourceSize");
							if (pFrame && pSrcRect && pSrcSize)
							{
								DKRect frameRect = ParseRect(GetPCData(pFrame->value));
								DKRect srcRect = ParseRect(GetPCData(pSrcRect->value));
								DKSize srcSize = ParseSize(GetPCData(pSrcSize->value));
								if (frameRect.IsValid() && srcRect.IsValid() && srcSize.width > 0 && srcSize.height > 0)
								{
									pf.frame = frameRect;
									pf.inset = srcRect;
									pf.size = srcSize;
									
									if (pRotated && pRotated->value->name.Compare(L"true") == 0)
										pf.rotated = true;
									else
										pf.rotated = false;
									return true;
								}
							}
						}
						return false;
					}
					bool ParseFrames(const DKXMLElement* dict)
					{
						if (dict->name.Compare(L"dict") == 0)
						{
							Dict tmp;
							DictionaryParser p(tmp, dict);

							tmp.EnumerateForward([this](Dict::Pair& pair)
												 {
													 PixelFrame pf;
													 if (ParseFrameInfo(pair.value, pf))
													 {
														 pf.file = pair.key;
														 this->frames.Add(pf);
													 }
												 });
							return true;
						}
						return false;
					}
					bool ParseMetadata(const DKXMLElement* dict)
					{
						if (dict->name.Compare(L"dict") == 0)
						{
							Dict tmp;
							DictionaryParser p(tmp, dict);
							Dict::Pair* pFileName = tmp.Find(L"realTextureFileName");
							Dict::Pair* pSize = tmp.Find(L"size");
							if (pFileName && pSize)
							{
								this->atlasFile = GetPCData(pFileName->value);
								this->atlasSize = ParseSize(GetPCData(pSize->value));
								return true;
							}
						}
						return false;
					}
					DKArray<PixelFrame> frames;
					DKString atlasFile;
					DKSize atlasSize;
					bool result;
				} parser(dictionary);
				
				if (parser.result)
				{
					DKObject<DKTexture2D> tex = pool->LoadResource(parser.atlasFile).SafeCast<DKTexture2D>();
					if (tex)
					{
						DKObject<TexturePack> pack = DKObject<TexturePack>::New();
						
						for (size_t i = 0; i < parser.frames.Count(); ++i)
						{
							const FrameParser::PixelFrame& pf = parser.frames.Value(i);
							
							Frame frame;
							frame.offset.origin.x = pf.inset.origin.x / pf.size.width;
							frame.offset.origin.y = 1.0 - (pf.inset.origin.y + pf.inset.size.height) / pf.size.height;
							frame.offset.size.width = pf.inset.size.width / pf.size.width;
							frame.offset.size.height = pf.inset.size.height / pf.size.height;

							DKAffineTransform2 texTM(DKLinearTransform2().Scale(pf.frame.size.Vector()));
							if (pf.rotated)
							{
								texTM *= DKLinearTransform2(-DKL_PI * 0.5);
								texTM.Translate(pf.frame.origin.x, parser.atlasSize.height - pf.frame.origin.y);
							}
							else
							{
								texTM.Translate(pf.frame.origin.x, parser.atlasSize.height - pf.frame.origin.y - pf.frame.size.height);
							}
							texTM *= DKLinearTransform2(1.0 / parser.atlasSize.width, 1.0 / parser.atlasSize.height);

							frame.texTM = texTM.Matrix3();
							frame.resolution = pf.size;
							
							pack->frames.Insert(pf.file, frame);
						}
						pack->texture = tex;						
						return pack;
					}
				}
			}
		}
	}
	return NULL;
}

DKObject<TexturePack> TexturePack::Create(DKStream* stream, DKResourcePool* pool)
{
	DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, stream);
	if (doc)
	{
		return Create(doc->RootElement(), pool);
	}
	return NULL;
}

DKSize TexturePack::TextureSize(const DKString& key) const
{
	const FrameMap::Pair* p = frames.Find(key);
	if (p)
	{
		return p->value.resolution;
	}
	return DKSize(0,0);
}

bool TexturePack::AddDisplayFrame(const DKRect& posRect, const DKMatrix3& posTM, const DKRect& texRect, const DKMatrix3& texTM, const DKString& key, DisplayFrameList& verts) const
{
	const FrameMap::Pair* p = frames.Find(key);
	if (p)
	{
		const DKRect& offset = p->value.offset;
		const DKMatrix3 texAtlasTM = texTM * p->value.texTM;
		
		const DKRect rc(
						posRect.origin.x + offset.origin.x * posRect.size.width,
						posRect.origin.y + offset.origin.y * posRect.size.height,
						posRect.size.width * offset.size.width,
						posRect.size.height * offset.size.height);
		
		DKVector2 pos[4] = {
			DKVector2(rc.origin.x, rc.origin.y).Transform(posTM),										// left-bottom
			DKVector2(rc.origin.x, rc.origin.y + rc.size.height).Transform(posTM),						// left-top
			DKVector2(rc.origin.x + rc.size.width, rc.origin.y + rc.size.height).Transform(posTM),		// right-top
			DKVector2(rc.origin.x + rc.size.width, rc.origin.y).Transform(posTM)						// right-bottom
		};
		DKVector2 tex[4] = {
			DKVector2(texRect.origin.x, texRect.origin.y).Transform(texAtlasTM),											// left-bottom
			DKVector2(texRect.origin.x, texRect.origin.y + texRect.size.height).Transform(texAtlasTM),						// left-top
			DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y + texRect.size.height).Transform(texAtlasTM),	// right-top
			DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y).Transform(texAtlasTM)						// right-bottom
		};
		
		DKRenderer::Vertex2D v[6] = {
			DKRenderer::Vertex2D(pos[0], tex[0]), DKRenderer::Vertex2D(pos[1], tex[1]), DKRenderer::Vertex2D(pos[3], tex[3]),
			DKRenderer::Vertex2D(pos[2], tex[2]), DKRenderer::Vertex2D(pos[3], tex[3]), DKRenderer::Vertex2D(pos[1], tex[1])
		};
		verts.Add(v, 6);
		return true;
	}
	return false;
}

void TexturePack::RenderDisplayFrame(const DisplayFrameList& verts, const DKRenderer* renderer, const DKColor& color, const DKBlendState& blend) const
{
	renderer->RenderPrimitive(DKPrimitive::TypeTriangles, verts, verts.Count(), texture, NULL, color, blend);
}

