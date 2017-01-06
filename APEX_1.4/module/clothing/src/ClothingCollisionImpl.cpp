/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ClothingCollisionImpl.h"
#include "ClothingActorImpl.h"

using namespace nvidia;
using namespace apex;
using namespace clothing;


ClothingCollisionImpl::ClothingCollisionImpl(ResourceList& list, ClothingActorImpl& owner) :
mOwner(owner),
mInRelease(false),
mId(-1)
{
	list.add(*this);
}


void ClothingCollisionImpl::release()
{
	if (mInRelease)
		return;
	mInRelease = true;
	mOwner.releaseCollision(*this);
}


void ClothingCollisionImpl::destroy()
{
	delete this;
}


void ClothingPlaneImpl::setPlane(const PxPlane& plane)
{
	mPlane = plane;
	mOwner.notifyCollisionChange();
}


void ClothingSphereImpl::setPosition(const PxVec3& position)
{
	mPosition = position;
	mOwner.notifyCollisionChange();
}


void ClothingSphereImpl::setRadius(float radius)
{
	mRadius = radius;
	mOwner.notifyCollisionChange();
}


uint32_t ClothingTriangleMeshImpl::lockTriangles(const uint32_t** ids, const PxVec3** triangles)
{
	mLock.lock();
	uint32_t numTriangles = mIds.size();
	if (ids != NULL)
	{
		*ids = (numTriangles > 0) ? &mIds[0] : NULL; 
	}
	if (triangles != NULL)
	{
		*triangles = (numTriangles > 0) ? &mTriangles[0] : NULL; 
	}
	return numTriangles;
};


uint32_t ClothingTriangleMeshImpl::lockTrianglesWrite(const uint32_t** ids, PxVec3** triangles)
{
	mLock.lock();
	uint32_t numTriangles = mIds.size();
	if (ids != NULL)
	{
		*ids = (numTriangles > 0) ? &mIds[0] : NULL; 
	}
	if (triangles != NULL)
	{
		*triangles = (numTriangles > 0) ? &mTriangles[0] : NULL; 
	}

	mOwner.notifyCollisionChange();
	return numTriangles;
};


void ClothingTriangleMeshImpl::unlockTriangles()
{
	mLock.unlock();
};


void ClothingTriangleMeshImpl::addTriangle(uint32_t id, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2)
{
	mLock.lock();
	ClothingTriangle& tri = mAddedTriangles.insert();
	tri.v[0] = v0;
	tri.v[1] = v1;
	tri.v[2] = v2;
	tri.id = id;
	mLock.unlock();

	mOwner.notifyCollisionChange();
}


void ClothingTriangleMeshImpl::addTriangles(const uint32_t* ids, const PxVec3* triangleVertices, uint32_t numTriangles)
{
	mLock.lock();
	for (uint32_t i = 0; i < numTriangles; ++i)
	{
		ClothingTriangle& tri = mAddedTriangles.insert();
		tri.v[0] = triangleVertices[3*i+0];
		tri.v[1] = triangleVertices[3*i+1];
		tri.v[2] = triangleVertices[3*i+2];
		tri.id = ids[i];
	}
	mLock.unlock();

	mOwner.notifyCollisionChange();
}


void ClothingTriangleMeshImpl::removeTriangle(uint32_t id)
{
	mLock.lock();
	mRemoved.pushBack(id);
	mLock.unlock();

	mOwner.notifyCollisionChange();
}


void ClothingTriangleMeshImpl::removeTriangles(const uint32_t* ids, uint32_t numTriangles)
{
	mLock.lock();
	mRemoved.reserve(mRemoved.size() + numTriangles);
	for (uint32_t i = 0; i < numTriangles; ++i)
	{
		mRemoved.pushBack(ids[i]);
	}
	mLock.unlock();

	mOwner.notifyCollisionChange();
}


void ClothingTriangleMeshImpl::clearTriangles()
{
	mLock.lock();
	mTriangles.clear();
	mIds.clear();
	mLock.unlock();

	mOwner.notifyCollisionChange();
}


void ClothingTriangleMeshImpl::setPose(PxMat44 pose)
{
	mPose = pose;

	mOwner.notifyCollisionChange();
}


void ClothingTriangleMeshImpl::sortAddAndRemoves()
{
	if (mRemoved.size() > 1)
	{
		nvidia::sort<uint32_t>(&mRemoved[0], mRemoved.size());
	}

	if (mAddedTriangles.size() > 1)
	{
		nvidia::sort<ClothingTriangle>(&mAddedTriangles[0], mAddedTriangles.size());
	}
}


void ClothingTriangleMeshImpl::update(const PxTransform& tm, const nvidia::Array<PxVec3>& allTrianglesOld, nvidia::Array<PxVec3>& allTrianglesOldTemp, nvidia::Array<PxVec3>& allTriangles)
{
	Array<PxVec3> trianglesOldTemp;
	Array<PxVec3> trianglesTemp;
	Array<uint32_t> idsTemp;

	mLock.lock();

	// sort all arrays to keep increasing id order
	sortAddAndRemoves();

	uint32_t triIdx = 0;
	uint32_t numTriangles = mIds.size();

	uint32_t removedIdx = 0;
	uint32_t numRemoved = mRemoved.size();

	uint32_t addedIdx = 0;
	uint32_t numAdded = mAddedTriangles.size();

	while (triIdx < numTriangles || removedIdx < numRemoved || addedIdx < numAdded)
	{
		PX_ASSERT(allTriangles.size() % 3 == 0);
		PX_ASSERT(allTrianglesOldTemp.size() % 3 == 0);

		uint32_t triangleId = (triIdx < numTriangles) ? mIds[triIdx] : UINT32_MAX;
		uint32_t removedId = (removedIdx < numRemoved) ? mRemoved[removedIdx] : UINT32_MAX;
		uint32_t addedId = (addedIdx < numAdded) ? mAddedTriangles[addedIdx].id : UINT32_MAX;

		if (triangleId < removedId && triangleId <= addedId)
		{
			// handle existing triangle

			// when a triangle with an already existing id is added, just update the value
			PxVec3* v[3];
			if (addedId == triangleId)
			{
				// new values from addTriangle
				v[0] = &mAddedTriangles[addedIdx].v[0];
				v[1] = &mAddedTriangles[addedIdx].v[1];
				v[2] = &mAddedTriangles[addedIdx].v[2];
				++addedIdx;
			}
			else
			{
				// old values or edited values from lockTrianglesWrite
				v[0] = &mTriangles[3*triIdx+0];
				v[1] = &mTriangles[3*triIdx+1];
				v[2] = &mTriangles[3*triIdx+2];
			}
			PxVec3 vGlobal[3] = {tm.transform(*v[0]), tm.transform(*v[1]), tm.transform(*v[2])};

			// update triangle collision object with local triangle position
			trianglesTemp.pushBack(*v[0]);
			trianglesTemp.pushBack(*v[1]);
			trianglesTemp.pushBack(*v[2]);
			idsTemp.pushBack(mIds[triIdx]);

			// write global triangle pos from last frame.
			// mId contains the offset in the global triangles array
			if (mId >= 0 && mId < (int32_t)allTrianglesOld.size())
			{
				allTrianglesOldTemp.pushBack(allTrianglesOld[mId + 3*triIdx+0]);
				allTrianglesOldTemp.pushBack(allTrianglesOld[mId + 3*triIdx+1]);
				allTrianglesOldTemp.pushBack(allTrianglesOld[mId + 3*triIdx+2]);
			}
			else
			{
				// if we cannot access an old buffer (e.g. when the simulation has changed)
				// we just use the current pos as old pos as well
				allTrianglesOldTemp.pushBack(vGlobal[0]);
				allTrianglesOldTemp.pushBack(vGlobal[1]);
				allTrianglesOldTemp.pushBack(vGlobal[2]);
			}

			// update internal global array
			allTriangles.pushBack(vGlobal[0]);
			allTriangles.pushBack(vGlobal[1]);
			allTriangles.pushBack(vGlobal[2]);
			++triIdx;
		}
		else if (addedId < removedId)
		{
			// handle new triangle

			// update triangle collision object with local triangle position
			trianglesTemp.pushBack(mAddedTriangles[addedIdx].v[0]);
			trianglesTemp.pushBack(mAddedTriangles[addedIdx].v[1]);
			trianglesTemp.pushBack(mAddedTriangles[addedIdx].v[2]);
			idsTemp.pushBack(addedId);

			// set old and new positions in global array
			PxVec3 v[3] = {tm.transform(mAddedTriangles[addedIdx].v[0]), tm.transform(mAddedTriangles[addedIdx].v[1]), tm.transform(mAddedTriangles[addedIdx].v[2])};
			allTrianglesOldTemp.pushBack(v[0]);
			allTrianglesOldTemp.pushBack(v[1]);
			allTrianglesOldTemp.pushBack(v[2]);
			allTriangles.pushBack(v[0]);
			allTriangles.pushBack(v[1]);
			allTriangles.pushBack(v[2]);
			++addedIdx;
		}
		else
		{
			// handle removal

			++removedIdx;
			if (removedId == triangleId)
			{
				// an existing triangle was removed
				++triIdx;
			}
			if (removedId == addedId)
			{
				// a newly added triangle is also removed
				++addedIdx;
			}
		}

	}

	mTriangles.swap(trianglesTemp);
	mIds.swap(idsTemp);

	mRemoved.clear();
	mAddedTriangles.clear();

	mLock.unlock();

	// use id to store the offset in the global triangles arrays
	PX_ASSERT(allTriangles.size() % 3 == 0);
	mId = (int32_t)allTriangles.size() - (int32_t)mTriangles.size();
}