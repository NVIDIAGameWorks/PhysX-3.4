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

#ifndef CCT_CHARACTER_CONTROLLER_MANAGER
#define CCT_CHARACTER_CONTROLLER_MANAGER

//Exclude file from docs
/** \cond */

#include "PxControllerManager.h"
#include "PxControllerObstacles.h"
#include "PxMeshQuery.h"
#include "PxDeletionListener.h"
#include "CmRenderOutput.h"
#include "CctUtils.h"
#include "PsHashSet.h"
#include "PsHashMap.h"
#include "PsMutex.h"

namespace physx
{
namespace Cct
{
	class Controller;
	class ObstacleContext;

	struct ObservedRefCounter
	{
		ObservedRefCounter(): refCount(0)
		{
		}

		PxU32 refCount;
	};

	typedef Ps::HashMap<const PxBase*, ObservedRefCounter>			ObservedRefCountMap;

	//Implements the PxControllerManager interface, this class used to be called ControllerManager
	class CharacterControllerManager : public PxControllerManager   , public Ps::UserAllocated, public PxDeletionListener
	{		
	public:
														CharacterControllerManager(PxScene& scene, bool lockingEnabled = false);
		virtual											~CharacterControllerManager();

		// PxControllerManager
		virtual			void							release();
		virtual			PxScene&						getScene() const;
		virtual			PxU32							getNbControllers()	const;
		virtual			PxController*					getController(PxU32 index);
        virtual			PxController*					createController(const PxControllerDesc& desc);
       
		virtual			void							purgeControllers();
		virtual			PxRenderBuffer&					getRenderBuffer();
		virtual			void							setDebugRenderingFlags(PxControllerDebugRenderFlags flags);
		virtual			PxU32							getNbObstacleContexts() const;
		virtual			PxObstacleContext*				getObstacleContext(PxU32 index);
		virtual			PxObstacleContext*				createObstacleContext();
		virtual			void							computeInteractions(PxF32 elapsedTime, PxControllerFilterCallback* cctFilterCb);
		virtual			void							setTessellation(bool flag, float maxEdgeLength);
		virtual			void							setOverlapRecoveryModule(bool flag);
		virtual			void							setPreciseSweeps(bool flag);
		virtual			void							setPreventVerticalSlidingAgainstCeiling(bool flag);
		virtual			void							shiftOrigin(const PxVec3& shift);		
		//~PxControllerManager

		// PxDeletionListener
		virtual		void								onRelease(const PxBase* observed, void* userData, PxDeletionEventFlag::Enum deletionEvent);
		//~PxDeletionListener
						void							registerObservedObject(const PxBase* obj);
						void							unregisterObservedObject(const PxBase* obj);

		// ObstacleContextNotifications
						void							onObstacleRemoved(ObstacleHandle index) const;
						void							onObstacleUpdated(ObstacleHandle index, const PxObstacleContext* ) const;
						void							onObstacleAdded(ObstacleHandle index, const PxObstacleContext*) const;

						void							releaseController(PxController& controller);
						Controller**					getControllers();
						void							releaseObstacleContext(ObstacleContext& oc);
						void							resetObstaclesBuffers();

						PxScene&						mScene;

						Cm::RenderBuffer*				mRenderBuffer;
						PxControllerDebugRenderFlags	mDebugRenderingFlags;
		// Shared buffers for obstacles
						Ps::Array<const void*>			mBoxUserData;
						Ps::Array<PxExtendedBox>		mBoxes;

						Ps::Array<const void*>			mCapsuleUserData;
						Ps::Array<PxExtendedCapsule>	mCapsules;

						Ps::Array<Controller*>			mControllers;
						Ps::HashSet<PxShape*>			mCCTShapes;

						Ps::Array<ObstacleContext*>		mObstacleContexts;

						float							mMaxEdgeLength;
						bool							mTessellation;

						bool							mOverlapRecovery;
						bool							mPreciseSweeps;
						bool							mPreventVerticalSlidingAgainstCeiling;

						bool							mLockingEnabled;						

	protected:
		CharacterControllerManager &operator=(const CharacterControllerManager &);
		CharacterControllerManager(const CharacterControllerManager& );

	private:
						ObservedRefCountMap				mObservedRefCountMap;
						mutable		Ps::Mutex			mWriteLock;			// Lock used for guarding pointers in observedrefcountmap
	};

} // namespace Cct

}

/** \endcond */
#endif //CCT_CHARACTER_CONTROLLER_MANAGER
