/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef MESH_PAINTER_H
#define MESH_PAINTER_H

#include "ApexUsingNamespace.h"
#include "PxVec3.h"
#include "PxBounds3.h"

#if PX_WINDOWS_FAMILY

#include <vector>

namespace nvidia
{
namespace apex
{
class RenderDebugInterface;
class ClothingPhysicalMesh;
}
}

namespace SharedTools
{
struct DistTriPair;
struct PaintFloatBuffer;
struct PaintFlagBuffer;

class MeshPainter
{
public:
	MeshPainter();
	~MeshPainter();
	void clear();

	void initFrom(const nvidia::apex::ClothingPhysicalMesh* mesh);
	void initFrom(const physx::PxVec3* vertices, int numVertices, int vertexStride, const uint32_t* indices, int numIndices, int indexStride);

	void clearIndexBufferRange();
	void addIndexBufferRange(uint32_t start, uint32_t end);

	//void allocateFloatBuffer(uint32_t id);

	void setFloatBuffer(unsigned int id, float* buffer, int stride);
	void setFlagBuffer(unsigned int id, unsigned int* buffer, int stride);

	void* getFloatBuffer(uint32_t id);

	const std::vector<physx::PxVec3> getVertices() const
	{
		return mVertices;
	}
	const std::vector<uint32_t>  getIndices() const
	{
		return mIndices;
	}

	void changeRadius(float paintRadius);
	void setRayAndRadius(const physx::PxVec3& rayOrig, const physx::PxVec3& rayDir, float paintRadius, int brushMode, float falloffExponent, float scaledTargetValue, float targetColor);
	bool raycastHit()
	{
		return !mLastRaycastNormal.isZero();
	}

	void paintFloat(unsigned int id, float min, float max, float target) const;
	void paintFlag(unsigned int id, unsigned int flag, bool useAND) const;

	void smoothFloat(uint32_t id, float smoothingFactor, uint32_t numIterations) const;
	void smoothFloatFast(uint32_t id, uint32_t numIterations) const;

	void drawBrush(nvidia::apex::RenderDebugInterface* batcher) const;

private:
	PaintFloatBuffer& MeshPainter::getInternalFloatBuffer(unsigned int id);
	PaintFlagBuffer& MeshPainter::getInternalFlagBuffer(unsigned int id);

	void complete();
	void computeNormals();
	void createNeighborInfo();
	bool rayCast(int& triNr, float& t) const;
	bool rayTriangleIntersection(const physx::PxVec3& orig, const physx::PxVec3& dir, const physx::PxVec3& a,
	                             const physx::PxVec3& b, const physx::PxVec3& c, float& t, float& u, float& v) const;

	void computeSiblingInfo(float distanceThreshold);

	physx::PxVec3 getTriangleCenter(int triNr) const;
	physx::PxVec3 getTriangleNormal(int triNr) const;
	void collectTriangles() const;
	bool isValidRange(int vertexNumber) const;

	std::vector<physx::PxVec3> mVertices;
	std::vector<bool> mVerticesDisabled;
	std::vector<uint32_t> mIndices;
	struct IndexBufferRange
	{
		bool isOverlapping(const IndexBufferRange& other) const;
		uint32_t start;
		uint32_t end;
	};
	std::vector<IndexBufferRange> mIndexRanges;
	std::vector<int> mNeighbors;
	mutable std::vector<int> mTriMarks;
	mutable std::vector<DistTriPair> mCollectedTriangles;
	mutable std::vector<uint32_t> mCollectedVertices;
	mutable std::vector<float> mCollectedVerticesFloats;
	mutable std::vector<uint32_t> mSmoothingCollectedIndices;

	std::vector<physx::PxVec3> mNormals;
	std::vector<physx::PxVec3> mTetraNormals;

	std::vector<PaintFloatBuffer> mFloatBuffers;
	std::vector<PaintFlagBuffer> mFlagBuffers;

	mutable int mCurrentMark;

	physx::PxVec3 mRayOrig, mRayDir;
	float mPaintRadius;
	mutable float mTargetValue;
	float mScaledTargetValue;
	int mBrushMode;
	float mFalloffExponent;
	float mBrushColor;

	mutable int32_t mLastTriangle;
	mutable physx::PxVec3 mLastRaycastPos;
	mutable physx::PxVec3 mLastRaycastNormal;

	std::vector<int32_t> mFirstSibling;
	std::vector<int32_t> mSiblings;
};



struct DistTriPair
{
	void set(int triNr, float dist)
	{
		this->triNr = triNr;
		this->dist = dist;
	}
	bool operator < (const DistTriPair& f) const
	{
		return dist < f.dist;
	}
	int triNr;
	float dist;
};



struct PaintFloatBuffer
{
	float& operator[](int i)  const
	{
		return *(float*)((char*)buffer + i * stride);
	}
	float& operator[](unsigned i)  const
	{
		return *(float*)((char*)buffer + i * stride);
	}
	unsigned int id;
	void* buffer;
	int stride;
	bool allocated;
};

struct PaintFlagBuffer
{
	unsigned int& operator[](int i) const
	{
		return *(unsigned int*)((char*)buffer + i * stride);
	}
	unsigned int& operator[](unsigned i) const
	{
		return *(unsigned int*)((char*)buffer + i * stride);
	}

	unsigned int id;
	void* buffer;
	int stride;
};

} // namespace SharedTools


#endif // PX_WINDOWS_FAMILY

#endif
