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

#ifndef CCT_CONTROLLER
#define CCT_CONTROLLER

/* Exclude from documentation */
/** \cond */

#include "CctCharacterController.h"
#include "PsUserAllocated.h"
#include "PsMutex.h"

namespace physx
{

class PxPhysics;
class PxScene;
class PxRigidDynamic;
class PxGeometry;
class PxMaterial;

namespace Cct
{
	class CharacterControllerManager;

	class Controller : public Ps::UserAllocated
	{
		PX_NOCOPY(Controller)
	public:
														Controller(const PxControllerDesc& desc, PxScene* scene);
		virtual											~Controller();

					void								releaseInternal();
					void								getInternalState(PxControllerState& state)	const;
					void								getInternalStats(PxControllerStats& stats)	const;					

		virtual		PxF32								getHalfHeightInternal()				const	= 0;
		virtual		bool								getWorldBox(PxExtendedBounds3& box)	const	= 0;
		virtual		PxController*						getPxController()							= 0;

					void								onOriginShift(const PxVec3& shift);

					void								onRelease(const PxBase& observed);

					void								setCctManager(CharacterControllerManager* cm)
					{
						mManager = cm;
						mCctModule.setCctManager(cm);
					}

	PX_FORCE_INLINE	CharacterControllerManager*			getCctManager()				{ return mManager;									}
	PX_FORCE_INLINE	PxU64								getContextId()		const	{ return PxU64(reinterpret_cast<size_t>(mScene));	}

					PxControllerShapeType::Enum			mType;
		// User params
					CCTParams							mUserParams;
					PxUserControllerHitReport*			mReportCallback;
					PxControllerBehaviorCallback*		mBehaviorCallback;
					void*								mUserData;
		// Internal data
					SweepTest							mCctModule;			// Internal CCT object. Optim test for Ubi.
					PxRigidDynamic*						mKineActor;			// Associated kinematic actor
					PxExtendedVec3						mPosition;			// Current position
					PxVec3								mDeltaXP;
					PxVec3								mOverlapRecover;
					PxScene*							mScene;				// Handy scene owner
					PxU32								mPreviousSceneTimestamp;					
					PxF64								mGlobalTime;
					PxF64								mPreviousGlobalTime;
					PxF32								mProxyDensity;		// Density for proxy actor
					PxF32								mProxyScaleCoeff;	// Scale coeff for proxy actor
					PxControllerCollisionFlags			mCollisionFlags;	// Last known collision flags (PxControllerCollisionFlag)
					bool								mCachedStandingOnMoving;
					bool								mRegisterDeletionListener;
		mutable		Ps::Mutex							mWriteLock;			// Lock used for guarding touched pointers and cache data from overwriting 
																			// during onRelease call.
	protected:
		// Internal methods
					void								setUpDirectionInternal(const PxVec3& up);
					PxShape*							getKineShape()	const;
					bool								createProxyActor(PxPhysics& sdk, const PxGeometry& geometry, const PxMaterial& material);
					bool								setPos(const PxExtendedVec3& pos);
					void								findTouchedObject(const PxControllerFilters& filters, const PxObstacleContext* obstacleContext, const PxVec3& upDirection);
					bool								rideOnTouchedObject(SweptVolume& volume, const PxVec3& upDirection, PxVec3& disp, const PxObstacleContext* obstacleContext);
					PxControllerCollisionFlags			move(SweptVolume& volume, const PxVec3& disp, PxF32 minDist, PxF32 elapsedTime, const PxControllerFilters& filters, const PxObstacleContext* obstacles, bool constrainedClimbingMode);
					bool								filterTouchedShape(const PxControllerFilters& filters);

	PX_FORCE_INLINE	float								computeTimeCoeff()
														{
															const float elapsedTime = float(mGlobalTime - mPreviousGlobalTime);
															mPreviousGlobalTime = mGlobalTime;
															return 1.0f / elapsedTime;
														}

					CharacterControllerManager*			mManager;			// Owner manager
	};

} // namespace Cct

}

/** \endcond */
#endif
