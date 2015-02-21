#include "StdAfx.h"
#include "Polygonizer.h"

Polygonizer::Polygonizer(void)
	: skipTriangle(false)
	, searchVerts(DKArraySortAscending<VertIndex>)
{
}

Polygonizer::~Polygonizer(void)
{
}

void Polygonizer::Polygonize(const Range& range, Picker* picker)
{
	for (int z = range.beginZ - 1; z <= range.endZ; ++z)
	{
		bool skipZ = z < range.beginZ || z >= range.endZ;
		for (int y = range.beginY - 1; y <= range.endY; ++y)
		{
			bool skipY = y < range.beginY || y >= range.endY;
			for (int x = range.beginX - 1; x <= range.endX; ++x)
			{
				bool skipX = x < range.beginX || x >= range.endX;

				voxels[0] = picker->Invoke(  x,   y,   z);
				voxels[1] = picker->Invoke(1+x,   y,   z);
				voxels[2] = picker->Invoke(1+x,   y, 1+z);
				voxels[3] = picker->Invoke(  x,   y, 1+z);
				voxels[4] = picker->Invoke(  x, 1+y,   z);
				voxels[5] = picker->Invoke(1+x, 1+y,   z);
				voxels[6] = picker->Invoke(1+x, 1+y, 1+z);
				voxels[7] = picker->Invoke(  x, 1+y, 1+z);

				basePosition = DKVector3(x, y, z) + DKVector3(0.5, 0.5, 0.5);
				skipTriangle = skipX || skipY || skipZ;

				int levels = 0;
				for (int i = 0; i < 8; ++i)
				{
					if (voxels[i].level == 0)
						levels |= 1 << i;
				}
				this->PolygonizeSurface(levels);
			}
		}
	}
}

void Polygonizer::Finalize(void)
{
	searchVerts.Clear();
	triangleVertices.Reserve(outputVerts.Count());

	for (TempVertex& v : outputVerts)
	{
		VolumeVertex vert;
		vert.position = v.position;
		v.normal.Normalize();
		v.color /= v.colorDivFactor;
#if VOXEL_16BIT_NORMAL
		vert.normal[0] = static_cast<short>( v.normal.x * 255.0 );
		vert.normal[1] = static_cast<short>( v.normal.y * 255.0 );
		vert.normal[2] = static_cast<short>( v.normal.z * 255.0 );
		vert.normal[3] = 0;
#else
		vert.normal = v.normal;
#endif
		vert.color = v.color.RGBA32Value();

		triangleVertices.Add(vert);
	}
	outputVerts.Clear();
}

void Polygonizer::Reset(void)
{
	searchVerts.Clear();
	outputVerts.Clear();
	triangleIndices.Clear();
	triangleVertices.Clear();
}

DKVector3 Polygonizer::Interpolate(const DKVector3& p1, const DKVector3& p2, CubeIndex c1, CubeIndex c2)
{
	DKVoxel32& v1 = voxels[c1];
	DKVoxel32& v2 = voxels[c2];

	DKVector3 res;
	if (v1.level > 0)
	{
		DKASSERT_DEBUG(v2.level == 0);

		float val = static_cast<float>(v1.level) / 255.0f;
		res =  p1 + ((p2 - p1) * val);
	}
	else
	{
		DKASSERT_DEBUG(v1.level == 0);
		DKASSERT_DEBUG(v2.level > 0);

		float val = static_cast<float>(v2.level) / 255.0f;
		res = p2 + ((p1 - p2) * val);
	}

	// 0.001 이하 절삭
	int x = res.x * 1000;
	int y = res.y * 1000;
	int z = res.z * 1000;

	return DKVector3(x,y,z) * 0.001;
}

void Polygonizer::GenerateTriangle(Vertex& v1, Vertex& v2, Vertex& v3)
{
	if (v1.pos == v2.pos)
		return;
	if (v2.pos == v3.pos)
		return;
	if (v3.pos == v1.pos)
		return;
	
	const DKVector3 p1 = v2.pos - v1.pos;
	const DKVector3 p2 = v3.pos - v1.pos;

	DKVector3 normal = DKVector3::Cross(p1,p2).Normalize();

	Vertex* v[3] = {&v1, &v2, &v3};

	for (int i = 0; i < 3; ++i)
	{
		TempVertex vert;
		vert.position = basePosition + v[i]->pos;
		vert.normal = normal;
		vert.colorDivFactor = 1.0f;

		DKVoxel32& voxel1 = voxels[(int)v[i]->idx1];
		DKVoxel32& voxel2 = voxels[(int)v[i]->idx2];

		DKColor::RGBA32 rgba1 = {voxel1.data[0], voxel1.data[1], voxel1.data[2]};
		DKColor::RGBA32 rgba2 = {voxel2.data[0], voxel2.data[1], voxel2.data[2]};

		float lv1 = static_cast<float>(voxel1.level) / 255.0f;
		float lv2 = static_cast<float>(voxel2.level) / 255.0f;
		DKColor c1 = rgba1;
		DKColor c2 = rgba2;
		c1 *= (lv1 / (lv1 + lv2));
		c2 *= (lv2 / (lv1 + lv2));

		vert.color = (c1 + c2);

		VertIndex pos = {vert.position, 0};
		auto index = searchVerts.Find(pos);
		if (index == searchVerts.invalidIndex)
		{
			pos.index = static_cast<unsigned int>(outputVerts.Add(vert));
			searchVerts.Insert(pos);
		}
		else
		{
			pos.index = searchVerts.Value(index).index;
			TempVertex& v = outputVerts.Value(pos.index);
			v.normal += vert.normal;
			v.color += vert.color;
			v.colorDivFactor += 1.0f;
		}
		if (!skipTriangle)
			triangleIndices.Add(pos.index);
	}
}
