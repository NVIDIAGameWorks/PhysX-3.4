// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "CctCharacterControllerManager.h"
#include "CctBoxController.h"
#include "CctCapsuleController.h"
#include "CctObstacleContext.h"
#include "CmBoxPruning.h"
#include "GuDistanceSegmentSegment.h"
#include "GuDistanceSegmentBox.h"
#include "PsUtilities.h"
#include "PsMathUtils.h"
#include "PxRigidDynamic.h"
#include "PxScene.h"
#include "PxPhysics.h"
#include "PsFoundation.h"

using namespace physx;
using namespace Cct;

static const PxF32 gMaxOverlapRecover = 4.0f;	// PT: TODO: expose this

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CharacterControllerManager::CharacterControllerManager(PxScene& scene, bool lockingEnabled) :
	mScene									(scene),
	mRenderBuffer							(NULL),
	mDebugRenderingFlags					(0),
	mMaxEdgeLength							(1.0f),
	mTessellation							(false),
	mOverlapRecovery						(true),
	mPreciseSweeps							(true),
	mPreventVerticalSlidingAgainstCeiling	(false),
	mLockingEnabled							(lockingEnabled)
{
	// PT: register ourself as a deletion listener, to be called by the SDK whenever an object is deleted	
	PxPhysics& physics = scene.getPhysics();
	physics.registerDeletionListener(*this, PxDeletionEventFlag::eUSER_RELEASE);
}

CharacterControllerManager::~CharacterControllerManager()
{
	if(mRenderBuffer)
	{
		delete mRenderBuffer;
		mRenderBuffer = 0;
	}
}

void CharacterControllerManager::release() 
{
	// PT: TODO: use non virtual calls & move to dtor
	while(getNbControllers()!= 0)
		releaseController(*getController(0));

	while(getNbObstacleContexts()!= 0)
		mObstacleContexts[0]->release();

	PxPhysics& physics = mScene.getPhysics();
	physics.unregisterDeletionListener(*this);

	delete this;

	Ps::Foundation::decRefCount();
}

PxScene& CharacterControllerManager::getScene() const
{
	return mScene;
}

PxRenderBuffer& CharacterControllerManager::getRenderBuffer()
{
	if(!mRenderBuffer)
		mRenderBuffer = PX_NEW(Cm::RenderBuffer); 

	return *mRenderBuffer;
}

void CharacterControllerManager::setDebugRenderingFlags(PxControllerDebugRenderFlags flags)
{
	mDebugRenderingFlags = flags;

	if(!flags)
	{
		if(mRenderBuffer)
		{
			delete mRenderBuffer;
			mRenderBuffer = 0;
		}
	}
}

PxU32 CharacterControllerManager::getNbControllers() const
{
	return mControllers.size();
}

Controller** CharacterControllerManager::getControllers() 
{
	return mControllers.begin();
}

PxController* CharacterControllerManager::getController(PxU32 index) 
{
	if(index>=mControllers.size())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxControllerManager::getController(): out-of-range index");
		return NULL;
	}

	PX_ASSERT(mControllers[index]);
	return mControllers[index]->getPxController();
}

PxController* CharacterControllerManager::createController(const PxControllerDesc& desc)
{
	if(!desc.isValid())
		return NULL;

	Controller* newController = NULL;

	PxController* N = NULL;
	if(desc.getType()==PxControllerShapeType::eBOX)
	{
		BoxController* boxController = PX_NEW(BoxController)(desc, mScene.getPhysics(), &mScene);
		newController = boxController;
		N = boxController;
	}
	else if(desc.getType()==PxControllerShapeType::eCAPSULE)
	{
		CapsuleController* capsuleController = PX_NEW(CapsuleController)(desc, mScene.getPhysics(), &mScene);
		newController = capsuleController;
		N = capsuleController;
	}
	else PX_ALWAYS_ASSERT_MESSAGE( "INTERNAL ERROR - invalid CCT type, should have been caught by isValid().");

	if(newController)
	{
		mControllers.pushBack(newController);		
		newController->setCctManager(this);

		PxShape* shape = NULL;
		PxU32 nb = N->getActor()->getShapes(&shape, 1);
		PX_ASSERT(nb==1);
		PX_UNUSED(nb);
		mCCTShapes.insert(shape);
	}

	return N;
}

void CharacterControllerManager::releaseController(PxController& controller)
{
	for(PxU32 i = 0; i<mControllers.size(); i++)
	{
		if(mControllers[i]->getPxController() == &controller)
		{
			mControllers.replaceWithLast(i);
			break;
		}
	}

	PxShape* shape = NULL;
	PxU32 nb = controller.getActor()->getShapes(&shape, 1);
	PX_ASSERT(nb==1);
	PX_UNUSED(nb);
	mCCTShapes.erase(shape);

	if(controller.getType() == PxControllerShapeType::eCAPSULE) 
	{
		CapsuleController* cc = static_cast<CapsuleController*>(&controller);
		PX_DELETE(cc);
	} 
	else if(controller.getType() == PxControllerShapeType::eBOX) 
	{
		BoxController* bc = static_cast<BoxController*>(&controller);
		PX_DELETE(bc);
	} 
	else PX_ASSERT(0);
}

void CharacterControllerManager::purgeControllers()
{
	while(mControllers.size())
		releaseController(*mControllers[0]->getPxController());
}

void CharacterControllerManager::onRelease(const PxBase* observed, void* , PxDeletionEventFlag::Enum deletionEvent)
{
	PX_ASSERT(deletionEvent == PxDeletionEventFlag::eUSER_RELEASE);  // the only type we registered for
	PX_UNUSED(deletionEvent);

	if(!(observed->getConcreteType()==PxConcreteType:: eRIGID_DYNAMIC || observed->getConcreteType()==PxConcreteType:: eRIGID_STATIC ||
		observed->getConcreteType()==PxConcreteType::eSHAPE))
		return;

	// check if object was registered
	if(mLockingEnabled)
		mWriteLock.lock();

	const ObservedRefCountMap::Entry* releaseEntry = mObservedRefCountMap.find(observed);
	if(mLockingEnabled)
		mWriteLock.unlock();

	if(releaseEntry)
	{
		for (PxU32 i = 0; i < mControllers.size(); i++)
		{
			Controller* controller = mControllers[i];
			if(mLockingEnabled)
				controller->mWriteLock.lock();

			controller->onRelease(*observed);

			if(mLockingEnabled)
				controller->mWriteLock.unlock();
		}
	}
}

void CharacterControllerManager::registerObservedObject(const PxBase* obj)
{	
	if(mLockingEnabled)
		mWriteLock.lock();

	mObservedRefCountMap[obj].refCount++;	

	if(mLockingEnabled)
		mWriteLock.unlock();
}

void CharacterControllerManager::unregisterObservedObject(const PxBase* obj)
{
	if(mLockingEnabled)
		mWriteLock.lock();

	ObservedRefCounter& refCounter = mObservedRefCountMap[obj];
	PX_ASSERT(refCounter.refCount);
	refCounter.refCount--;
	if(!refCounter.refCount)
		mObservedRefCountMap.erase(obj);

	if(mLockingEnabled)
		mWriteLock.unlock();
}

PxU32 CharacterControllerManager::getNbObstacleContexts() const
{
	return mObstacleContexts.size();
}

PxObstacleContext* CharacterControllerManager::getObstacleContext(PxU32 index)
{
	if(index>=mObstacleContexts.size())
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, "PxControllerManager::getObstacleContext(): out-of-range index");
		return NULL;
	}

	PX_ASSERT(mObstacleContexts[index]);
	return mObstacleContexts[index];
}

PxObstacleContext* CharacterControllerManager::createObstacleContext()
{
	ObstacleContext* oc = PX_NEW(ObstacleContext)(*this);

	mObstacleContexts.pushBack(oc);

	return oc;
}

void CharacterControllerManager::releaseObstacleContext(ObstacleContext& oc)
{
	PX_ASSERT(mObstacleContexts.find(&oc) != mObstacleContexts.end());
	mObstacleContexts.findAndReplaceWithLast(&oc);

	PX_DELETE(&oc);
}

void CharacterControllerManager::onObstacleRemoved(ObstacleHandle index) const
{
	for(PxU32 i = 0; i<mControllers.size(); i++)
	{
		mControllers[i]->mCctModule.onObstacleRemoved(index);
	}
}

void CharacterControllerManager::onObstacleUpdated(ObstacleHandle index, const PxObstacleContext* context) const
{
	for(PxU32 i = 0; i<mControllers.size(); i++)
	{
		mControllers[i]->mCctModule.onObstacleUpdated(index,context, toVec3(mControllers[i]->mPosition), -mControllers[i]->mUserParams.mUpDirection, mControllers[i]->getHalfHeightInternal());
	}
}

void CharacterControllerManager::onObstacleAdded(ObstacleHandle index, const PxObstacleContext* context) const
{
	for(PxU32 i = 0; i<mControllers.size(); i++)
	{
		mControllers[i]->mCctModule.onObstacleAdded(index,context, toVec3(mControllers[i]->mPosition), -mControllers[i]->mUserParams.mUpDirection, mControllers[i]->getHalfHeightInternal());
	}
}


// PT: TODO: move to array class?
template <class T> 
void resetOrClear(T& a)
{
	const PxU32 c = a.capacity();
	if(!c)
		return;
	const PxU32 s = a.size();
	if(s>c/2)
		a.clear();
	else
		a.reset();
}

void CharacterControllerManager::resetObstaclesBuffers()
{
	resetOrClear(mBoxUserData);
	resetOrClear(mBoxes);
	resetOrClear(mCapsuleUserData);
	resetOrClear(mCapsules);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CharacterControllerManager::setTessellation(bool flag, float maxEdgeLength)
{
	mTessellation = flag;
	mMaxEdgeLength = maxEdgeLength;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CharacterControllerManager::setOverlapRecoveryModule(bool flag)
{
	mOverlapRecovery = flag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CharacterControllerManager::setPreciseSweeps(bool flag)
{
	mPreciseSweeps = flag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CharacterControllerManager::setPreventVerticalSlidingAgainstCeiling(bool flag)
{
	mPreventVerticalSlidingAgainstCeiling = flag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CharacterControllerManager::shiftOrigin(const PxVec3& shift)
{
	for(PxU32 i=0; i < mControllers.size(); i++)
	{
		mControllers[i]->onOriginShift(shift);
	}

	for(PxU32 i=0; i < mObstacleContexts.size(); i++)
	{
		mObstacleContexts[i]->onOriginShift(shift);
	}

	if (mRenderBuffer)
		mRenderBuffer->shift(-shift);

	// assumption is that these are just used for temporary stuff
	PX_ASSERT(!mBoxes.size());
	PX_ASSERT(!mCapsules.size());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool computeMTD(PxVec3& mtd, PxF32& depth, const PxVec3& e0, const PxVec3& c0, const PxMat33& r0, const PxVec3& e1, const PxVec3& c1, const PxMat33& r1)
{
	// Translation, in parent frame
	const PxVec3 v = c1 - c0;
	// Translation, in A's frame
	const PxVec3 T(v.dot(r0[0]), v.dot(r0[1]), v.dot(r0[2]));

	// B's basis with respect to A's local frame
	PxReal R[3][3];
	PxReal FR[3][3];
	PxReal ra, rb, t, d;

	PxReal overlap[15];

	// Calculate rotation matrix
	for(PxU32 i=0;i<3;i++)
	{
		for(PxU32 k=0;k<3;k++)
		{
			R[i][k] = r0[i].dot(r1[k]);
			FR[i][k] = 1e-6f + PxAbs(R[i][k]);	// Precompute fabs matrix
		}
	}

	// A's basis vectors
	for(PxU32 i=0;i<3;i++)
	{
		ra = e0[i];
		rb = e1[0]*FR[i][0] + e1[1]*FR[i][1] + e1[2]*FR[i][2];
		t = PxAbs(T[i]);

		d = ra + rb - t;
		if(d<0.0f)
			return false;

		overlap[i] = d;
	}

	// B's basis vectors
	for(PxU32 k=0;k<3;k++)
	{
		ra = e0[0]*FR[0][k] + e0[1]*FR[1][k] + e0[2]*FR[2][k];
		rb = e1[k];
		t = PxAbs(T[0]*R[0][k] + T[1]*R[1][k] + T[2]*R[2][k]);

		d = ra + rb - t;
		if(d<0.0f)
			return false;

		overlap[k+3] = d;
	}

	// PT: edge-edge tests are skipped, by design

	PxU32 minIndex=0;
	PxReal minD = overlap[0];
	for(PxU32 i=1;i<6;i++)
	{
		if(overlap[i]<minD)
		{
			minD = overlap[i];
			minIndex = i;
		}
	}

	depth = minD;

	switch(minIndex)
	{
		case 0:		mtd = r0.column0;	break;
		case 1:		mtd = r0.column1;	break;
		case 2:		mtd = r0.column2;	break;
		case 3:		mtd = r1.column0;	break;
		case 4:		mtd = r1.column1;	break;
		case 5:		mtd = r1.column2;	break;
		default:	PX_ASSERT(0);		break;
	};
	return true;
}

static PxVec3 fixDir(const PxVec3& dir, const PxVec3& up)
{
	PxVec3 normalCompo, tangentCompo;
	Ps::decomposeVector(normalCompo, tangentCompo, dir, up);
	return tangentCompo.getNormalized();
}

static void InteractionCharacterCharacter(Controller* entity0, Controller* entity1, PxF32 elapsedTime)
{
	PX_ASSERT(entity0);
	PX_ASSERT(entity1);

	PxF32 overlap=0.0f;
	PxVec3 dir(0.0f);

	if(entity0->mType>entity1->mType)
		Ps::swap(entity0, entity1);

	if(entity0->mType==PxControllerShapeType::eCAPSULE && entity1->mType==PxControllerShapeType::eCAPSULE)
	{
		CapsuleController* cc0 = static_cast<CapsuleController*>(entity0);
		CapsuleController* cc1 = static_cast<CapsuleController*>(entity1);

		PxExtendedCapsule capsule0;
		cc0->getCapsule(capsule0);

		PxExtendedCapsule capsule1;
		cc1->getCapsule(capsule1);

		const PxF32 r = capsule0.radius + capsule1.radius;

		const PxVec3 p00 = toVec3(capsule0.p0);
		const PxVec3 p01 = toVec3(capsule0.p1);
		const PxVec3 p10 = toVec3(capsule1.p0);
		const PxVec3 p11 = toVec3(capsule1.p1);

		PxF32 s,t;
		const PxF32 d = sqrtf(Gu::distanceSegmentSegmentSquared(p00, p01 - p00, p10, p11 - p10, &s, &t));
		if(d<r)
		{
			const PxVec3 center0 = s * p00 + (1.0f - s) * p01;
			const PxVec3 center1 = t * p10 + (1.0f - t) * p11;
			const PxVec3 up = entity0->mCctModule.mUserParams.mUpDirection;
			dir = fixDir(center0 - center1, up);
			overlap = r - d;
		}
	}
	else if(entity0->mType==PxControllerShapeType::eBOX && entity1->mType==PxControllerShapeType::eCAPSULE)
	{
		BoxController* cc0 = static_cast<BoxController*>(entity0);
		CapsuleController* cc1 = static_cast<CapsuleController*>(entity1);

		PxExtendedBox obb;
		cc0->getOBB(obb);

		PxExtendedCapsule capsule;
		cc1->getCapsule(capsule);
		const PxVec3 p0 = toVec3(capsule.p0);
		const PxVec3 p1 = toVec3(capsule.p1);

		PxF32 t;
		PxVec3 p;
		const PxMat33 M(obb.rot);
		const PxVec3 boxCenter = toVec3(obb.center);
		const PxF32 d = sqrtf(Gu::distanceSegmentBoxSquared(p0, p1, boxCenter, obb.extents, M, &t, &p));
		if(d<capsule.radius)
		{
//			const PxVec3 center0 = M.transform(p) + boxCenter;
//			const PxVec3 center1 = t * p0 + (1.0f - t) * p1;
			const PxVec3 center0 = boxCenter;
			const PxVec3 center1 = (p0 + p1)*0.5f;
			const PxVec3 up = entity0->mCctModule.mUserParams.mUpDirection;
			dir = fixDir(center0 - center1, up);
			overlap = capsule.radius - d;
		}
	}
	else
	{
		PX_ASSERT(entity0->mType==PxControllerShapeType::eBOX);
		PX_ASSERT(entity1->mType==PxControllerShapeType::eBOX);
		BoxController* cc0 = static_cast<BoxController*>(entity0);
		BoxController* cc1 = static_cast<BoxController*>(entity1);

		PxExtendedBox obb0;
		cc0->getOBB(obb0);

		PxExtendedBox obb1;
		cc1->getOBB(obb1);

		PxVec3 mtd;
		PxF32 depth;
		if(computeMTD(	mtd, depth,
						obb0.extents, toVec3(obb0.center), PxMat33(obb0.rot),
						obb1.extents, toVec3(obb1.center), PxMat33(obb1.rot)))
		{
			const PxVec3 center0 = toVec3(obb0.center);
			const PxVec3 center1 = toVec3(obb1.center);
			const PxVec3 witness = center0 - center1;
			if(mtd.dot(witness)<0.0f)
				dir = -mtd;
			else
				dir = mtd;
			const PxVec3 up = entity0->mCctModule.mUserParams.mUpDirection;
			dir = fixDir(dir, up);
			overlap = depth;
		}
	}

	if(overlap!=0.0f)
	{
		// We want to limit this to some reasonable amount, to avoid obvious "popping".
		const PxF32 maxOverlap = gMaxOverlapRecover * elapsedTime;
		if(overlap>maxOverlap)
			overlap=maxOverlap;

		const PxVec3 sep = dir * overlap * 0.5f;
		entity0->mOverlapRecover += sep;
		entity1->mOverlapRecover -= sep;
	}
}

void CharacterControllerManager::computeInteractions(PxF32 elapsedTime, PxControllerFilterCallback* cctFilterCb)
{
	PxU32 nbControllers = mControllers.size();
	Controller** controllers = mControllers.begin();

	PxBounds3* boxes = reinterpret_cast<PxBounds3*>(PX_ALLOC_TEMP(sizeof(PxBounds3)*nbControllers, "CharacterControllerManager::computeInteractions"));	// PT: TODO: get rid of alloc
	PxBounds3* runningBoxes = boxes;

	while(nbControllers--)
	{
		Controller* current = *controllers++;

		PxExtendedBounds3 extBox;
		current->getWorldBox(extBox);

		*runningBoxes++ = PxBounds3(toVec3(extBox.minimum), toVec3(extBox.maximum));	// ### LOSS OF ACCURACY
	}

	//

	const PxU32 nbEntities = PxU32(runningBoxes - boxes);

	Ps::Array<PxU32> pairs;	// PT: TODO: get rid of alloc
	Cm::CompleteBoxPruning(boxes, nbEntities, pairs, Gu::Axes(physx::Gu::AXES_XZY));	// PT: TODO: revisit for variable up axis

	PxU32 nbPairs = pairs.size()>>1;
	const PxU32* indices = pairs.begin();
	while(nbPairs--)
	{
		const PxU32 index0 = *indices++;
		const PxU32 index1 = *indices++;
		Controller* ctrl0 = mControllers[index0];
		Controller* ctrl1 = mControllers[index1];

		bool keep=true;
		if(cctFilterCb)
			keep = cctFilterCb->filter(*ctrl0->getPxController(), *ctrl1->getPxController());

		if(keep)
			InteractionCharacterCharacter(ctrl0, ctrl1, elapsedTime);
	}

	PX_FREE(boxes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Public factory methods

PX_C_EXPORT PX_PHYSX_CHARACTER_API PxControllerManager* PX_CALL_CONV PxCreateControllerManager(PxScene& scene, bool lockingEnabled)
{
	Ps::Foundation::incRefCount();
	return PX_NEW(CharacterControllerManager)(scene, lockingEnabled);
}
