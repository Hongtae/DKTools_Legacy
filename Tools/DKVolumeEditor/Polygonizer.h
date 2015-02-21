#pragma once
#include <DK.h>
using namespace DKFoundation;
using namespace DKFramework;

#define VOXEL_16BIT_NORMAL	0

struct VolumeVertex
{
	DKVector3 position;
#if VOXEL_16BIT_NORMAL
	short normal[4];			// 4-byte alignment 를 맞추기 위해 [4] 로 맞춘것
#else
	DKVector3 normal;
#endif
	DKColor::RGBA32 color;
};

class Polygonizer : public DKVoxelPolygonizer
{
public:
	Polygonizer(void);
	~Polygonizer(void);

	struct Range
	{
		int beginX, endX;
		int beginY, endY;
		int beginZ, endZ;
	};
	typedef DKFunctionSignature<DKVoxel32 (int, int, int)> Picker;

	void Polygonize(const Range& range, Picker* picker);
	void Finalize(void);
	void Reset(void);

	DKArray<unsigned int> triangleIndices;
	DKArray<VolumeVertex> triangleVertices;

protected:
	void GenerateTriangle(Vertex&, Vertex&, Vertex&);
	DKVector3 Interpolate(const DKVector3& p1, const DKVector3& p2, CubeIndex c1, CubeIndex c2);

private:
	struct TempVertex	// 임시 버텍스 (중복된 정점을 합친 값이 저장됨)
	{
		DKVector3 position;
		DKVector3 normal;
		DKColor color;
		float colorDivFactor;
	};
	struct VertIndex	// 위치(position)로 중복된 정점을 찾는 용도
	{
		DKVector3 position;
		unsigned int index;
		int Compare(const VertIndex& rhs) const
		{
			float k = position.z - rhs.position.z;
			if (fabs(k) > FLT_EPSILON)		return k > 0 ? 1 : -1;
			k = position.y - rhs.position.y;
			if (fabs(k) > FLT_EPSILON)		return k > 0 ? 1 : -1;
			k = position.x - rhs.position.x;
			if (fabs(k) > FLT_EPSILON)		return k > 0 ? 1 : -1;
			return 0;
		}
		bool operator > (const VertIndex& rhs) const {return Compare(rhs) > 0;}
		bool operator < (const VertIndex& rhs) const {return Compare(rhs) < 0;}
		bool operator == (const VertIndex& rhs) const {return Compare(rhs) == 0;}
	};

	DKVoxel32 voxels[8];
	DKVector3 basePosition;
	bool skipTriangle;	// true 면 노멀,컬러 계산만 한다. (경계면을 부드럽게 하는 용도임)
	DKArray<TempVertex> outputVerts;
	DKOrderedArray<VertIndex> searchVerts;
};
