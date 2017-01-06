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


#ifndef PX_PHYSICS_SC_PHYSICS
#define PX_PHYSICS_SC_PHYSICS

#include "PxPhysics.h"
#include "PxScene.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PsBasicTemplates.h"
#include "PxActor.h"

namespace physx
{

namespace cloth
{
class Factory;
}


class PxMaterial;
class PxTolerancesScale;
struct PxvOffsetTable;

#if PX_SUPPORT_GPU_PHYSX
class PxPhysXGpu;
#endif

namespace Sc
{
	class Scene;
	class StaticCore;
	class RigidCore;
	class BodyCore;
	class ArticulationCore;
	class ConstraintCore;
	class ParticleSystemCore;
	class ClothCore;
	class ShapeCore;

	struct OffsetTable
	{
		PX_FORCE_INLINE OffsetTable() {}

		PX_FORCE_INLINE PxActor*				convertScRigidStatic2PxActor(StaticCore* sc)					const	{ return Ps::pointerOffset<PxActor*>(sc, scRigidStatic2PxActor);					}
		PX_FORCE_INLINE PxActor*				convertScRigidDynamic2PxActor(BodyCore* sc)						const	{ return Ps::pointerOffset<PxActor*>(sc, scRigidDynamic2PxActor);					}
		PX_FORCE_INLINE PxActor*				convertScArticulationLink2PxActor(BodyCore* sc)					const	{ return Ps::pointerOffset<PxActor*>(sc, scArticulationLink2PxActor);				}

		PX_FORCE_INLINE PxShape*				convertScShape2Px(ShapeCore* sc)								const	{ return Ps::pointerOffset<PxShape*>(sc, scShape2Px);								}
		PX_FORCE_INLINE const PxShape*			convertScShape2Px(const ShapeCore* sc)							const	{ return Ps::pointerOffset<const PxShape*>(sc, scShape2Px);							}

		PX_FORCE_INLINE PxArticulation*			convertScArticulation2Px(ArticulationCore* sc)					const	{ return Ps::pointerOffset<PxArticulation*>(sc, scArticulation2Px);					}
		PX_FORCE_INLINE const PxArticulation*	convertScArticulation2Px(const ArticulationCore* sc)			const	{ return Ps::pointerOffset<const PxArticulation*>(sc, scArticulation2Px);			}

		PX_FORCE_INLINE PxConstraint*			convertScConstraint2Px(ConstraintCore* sc)						const	{ return Ps::pointerOffset<PxConstraint*>(sc, scConstraint2Px);						}
		PX_FORCE_INLINE const PxConstraint*		convertScConstraint2Px(const ConstraintCore* sc)				const	{ return Ps::pointerOffset<const PxConstraint*>(sc, scConstraint2Px);				}

		PX_FORCE_INLINE PxParticleFluid*		convertScParticleSystem2PxParticleFluid(ParticleSystemCore* sc)	const	{ return Ps::pointerOffset<PxParticleFluid*>(sc, scParticleSystem2PxParticleFluid);	}
		PX_FORCE_INLINE PxParticleSystem*		convertScParticleSystem2Px(ParticleSystemCore* sc)				const	{ return Ps::pointerOffset<PxParticleSystem*>(sc, scParticleSystem2Px);				}

		PX_FORCE_INLINE PxCloth*				convertScCloth2Px(ClothCore* sc)								const	{ return Ps::pointerOffset<PxCloth*>(sc, scCloth2Px);								}
	
		ptrdiff_t	scRigidStatic2PxActor;
		ptrdiff_t 	scRigidDynamic2PxActor;
		ptrdiff_t 	scArticulationLink2PxActor;
		ptrdiff_t 	scShape2Px;
		ptrdiff_t 	scArticulation2Px;
		ptrdiff_t 	scConstraint2Px;
		ptrdiff_t 	scParticleSystem2PxParticleFluid;
		ptrdiff_t 	scParticleSystem2Px;
		ptrdiff_t 	scCloth2Px;

		ptrdiff_t	scCore2PxActor[PxActorType::eACTOR_COUNT];
	};
	extern OffsetTable gOffsetTable;

	class Physics : public Ps::UserAllocated
	{
	public:
		PX_FORCE_INLINE static Physics&				getInstance()						{ return *mInstance; }

													Physics(const PxTolerancesScale&, const PxvOffsetTable& pxvOffsetTable);
													~Physics(); // use release() instead
	public:
						void						release();

		PX_FORCE_INLINE	const PxTolerancesScale&	getTolerancesScale()		const	{ return mScale;	}

#if PX_USE_CLOTH_API
						void						registerCloth();
		PX_FORCE_INLINE	bool						hasLowLevelClothFactory()	const	{ return mLowLevelClothFactory != 0; }
		PX_FORCE_INLINE	cloth::Factory&				getLowLevelClothFactory()			{ PX_ASSERT(mLowLevelClothFactory); return *mLowLevelClothFactory; }
#endif

	private:
						PxTolerancesScale			mScale;
		static			Physics*					mInstance;
						cloth::Factory*				mLowLevelClothFactory;

	public:
		static			const PxReal				sWakeCounterOnCreation;
	};

} // namespace Sc

}

#endif
