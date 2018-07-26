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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "PxDefaultSimulationFilterShader.h"
#include "PsIntrinsics.h"
#include "particles/PxParticleBase.h"
#include "cloth/PxCloth.h"
#include "PxRigidActor.h"
#include "PxShape.h"
#include "PsAllocator.h"
#include "CmPhysXCommon.h"
#include "PsInlineArray.h"
#include "PsFoundation.h"

using namespace physx;

namespace
{
	#define GROUP_SIZE	32

	struct PxCollisionBitMap
	{
		PX_INLINE PxCollisionBitMap() : enable(true) {}

		bool operator()() const { return enable; }
		bool& operator= (const bool &v) { enable = v; return enable; } 

		private:
		bool enable;
	};

	PxCollisionBitMap gCollisionTable[GROUP_SIZE][GROUP_SIZE];

	PxFilterOp::Enum gFilterOps[3] = { PxFilterOp::PX_FILTEROP_AND, PxFilterOp::PX_FILTEROP_AND, PxFilterOp::PX_FILTEROP_AND };
	
	PxGroupsMask gFilterConstants[2];
	
	bool gFilterBool = false;

	static void gAND(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(mask0.bits0 & mask1.bits0);
		results.bits1 = PxU16(mask0.bits1 & mask1.bits1);
		results.bits2 = PxU16(mask0.bits2 & mask1.bits2);
		results.bits3 = PxU16(mask0.bits3 & mask1.bits3);
	}
	static void gOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(mask0.bits0 | mask1.bits0);
		results.bits1 = PxU16(mask0.bits1 | mask1.bits1);
		results.bits2 = PxU16(mask0.bits2 | mask1.bits2);
		results.bits3 = PxU16(mask0.bits3 | mask1.bits3);
	}
	static void gXOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(mask0.bits0 ^ mask1.bits0);
		results.bits1 = PxU16(mask0.bits1 ^ mask1.bits1);
		results.bits2 = PxU16(mask0.bits2 ^ mask1.bits2);
		results.bits3 = PxU16(mask0.bits3 ^ mask1.bits3);
	}
	static void gNAND(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(~(mask0.bits0 & mask1.bits0));
		results.bits1 = PxU16(~(mask0.bits1 & mask1.bits1));
		results.bits2 = PxU16(~(mask0.bits2 & mask1.bits2));
		results.bits3 = PxU16(~(mask0.bits3 & mask1.bits3));
	}
	static void gNOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(~(mask0.bits0 | mask1.bits0));
		results.bits1 = PxU16(~(mask0.bits1 | mask1.bits1));
		results.bits2 = PxU16(~(mask0.bits2 | mask1.bits2));
		results.bits3 = PxU16(~(mask0.bits3 | mask1.bits3));
	}
	static void gNXOR(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(~(mask0.bits0 ^ mask1.bits0));
		results.bits1 = PxU16(~(mask0.bits1 ^ mask1.bits1));
		results.bits2 = PxU16(~(mask0.bits2 ^ mask1.bits2));
		results.bits3 = PxU16(~(mask0.bits3 ^ mask1.bits3));
	}

	static void gSWAP_AND(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1)
	{
		results.bits0 = PxU16(mask0.bits0 & mask1.bits2);
		results.bits1 = PxU16(mask0.bits1 & mask1.bits3);
		results.bits2 = PxU16(mask0.bits2 & mask1.bits0);
		results.bits3 = PxU16(mask0.bits3 & mask1.bits1);
	}
	
	typedef void	(*FilterFunction)	(PxGroupsMask& results, const PxGroupsMask& mask0, const PxGroupsMask& mask1);

	FilterFunction const gTable[] = { gAND, gOR, gXOR, gNAND, gNOR, gNXOR, gSWAP_AND };

	static PxFilterData convert(const PxGroupsMask& mask)
	{
		PxFilterData fd;

		fd.word2 = PxU32(mask.bits0 | (mask.bits1 << 16));
		fd.word3 = PxU32(mask.bits2 | (mask.bits3 << 16));

		return fd;
	}

	static PxGroupsMask convert(const PxFilterData& fd)
	{
		PxGroupsMask mask;

		mask.bits0 = PxU16((fd.word2 & 0xffff));
		mask.bits1 = PxU16((fd.word2 >> 16));
		mask.bits2 = PxU16((fd.word3 & 0xffff));
		mask.bits3 = PxU16((fd.word3 >> 16));

		return mask;
	}

	static bool getFilterData(const PxActor& actor, PxFilterData& fd)
	{
		PxActorType::Enum aType = actor.getType();
		switch (aType)
		{
			case PxActorType::eRIGID_DYNAMIC:
			case PxActorType::eRIGID_STATIC:
			case PxActorType::eARTICULATION_LINK:
			{
				const PxRigidActor& rActor = static_cast<const PxRigidActor&>(actor);
				PX_CHECK_AND_RETURN_VAL(rActor.getNbShapes() >= 1,"There must be a shape in actor", false);

				PxShape* shape = NULL;
				rActor.getShapes(&shape, 1);

				fd = shape->getSimulationFilterData();
			}
			break;

#if PX_USE_PARTICLE_SYSTEM_API
			case PxActorType::ePARTICLE_FLUID:
			case PxActorType::ePARTICLE_SYSTEM:
			{
				const PxParticleBase& pActor = static_cast<const PxParticleBase&>(actor);
				fd = pActor.getSimulationFilterData();
			}
			break;
#endif

#if PX_USE_CLOTH_API
			case PxActorType::eCLOTH:
			{
				const PxCloth& cActor = static_cast<const PxCloth&>(actor);
				fd = cActor.getSimulationFilterData();
			}
			break;
#endif
			case PxActorType::eACTOR_COUNT:
			case PxActorType::eACTOR_FORCE_DWORD:
			break;
		}

		return true;
	}

	PX_FORCE_INLINE static void adjustFilterData(bool groupsMask, const PxFilterData& src, PxFilterData& dst)
	{
		if (groupsMask)
		{
			dst.word2 = src.word2;
			dst.word3 = src.word3;
		}
		else
			dst.word0 = src.word0;
	}

	template<bool TGroupsMask>
	static void setFilterData(PxActor& actor, const PxFilterData& fd)
	{
		PxActorType::Enum aType = actor.getType();
		switch (aType)
		{
			case PxActorType::eRIGID_DYNAMIC:
			case PxActorType::eRIGID_STATIC:
			case PxActorType::eARTICULATION_LINK:
			{
				const PxRigidActor& rActor = static_cast<const PxRigidActor&>(actor);

				PxShape* shape;
				for(PxU32 i=0; i < rActor.getNbShapes(); i++)
				{
					rActor.getShapes(&shape, 1, i);

					// retrieve current group mask
					PxFilterData resultFd = shape->getSimulationFilterData();
					
					adjustFilterData(TGroupsMask, fd, resultFd);
					
					// set new filter data
					shape->setSimulationFilterData(resultFd);
				}
			}
			break;

#if PX_USE_PARTICLE_SYSTEM_API
			case PxActorType::ePARTICLE_FLUID:
			case PxActorType::ePARTICLE_SYSTEM:
			{
				PxParticleBase& pActor = static_cast<PxParticleBase&>(actor);

				PxFilterData resultFd = pActor.getSimulationFilterData();

				adjustFilterData(TGroupsMask, fd, resultFd);

				pActor.setSimulationFilterData(resultFd);
			}
			break;
#endif

#if PX_USE_CLOTH_API
			case PxActorType::eCLOTH:
			{
				PxCloth& cActor = static_cast<PxCloth&>(actor);
				
				PxFilterData resultFd = cActor.getSimulationFilterData();

				adjustFilterData(TGroupsMask, fd, resultFd);

				cActor.setSimulationFilterData(resultFd);
			}
			break;
#endif
			
			case PxActorType::eACTOR_COUNT:
			case PxActorType::eACTOR_FORCE_DWORD:
			break;
		}
	}
}

PxFilterFlags physx::PxDefaultSimulationFilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0, 
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize)
{
	PX_UNUSED(constantBlock);
	PX_UNUSED(constantBlockSize);

	// let triggers through
	if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlags();
	}

	// Collision Group
	if (!gCollisionTable[filterData0.word0][filterData1.word0]())
	{
		return PxFilterFlag::eSUPPRESS;
	}

	// Filter function
	PxGroupsMask g0 = convert(filterData0);
	PxGroupsMask g1 = convert(filterData1);

	PxGroupsMask g0k0;	gTable[gFilterOps[0]](g0k0, g0, gFilterConstants[0]);
	PxGroupsMask g1k1;	gTable[gFilterOps[1]](g1k1, g1, gFilterConstants[1]);
	PxGroupsMask final;	gTable[gFilterOps[2]](final, g0k0, g1k1);
	
	bool r = final.bits0 || final.bits1 || final.bits2 || final.bits3;
	if (r != gFilterBool)
	{
		return PxFilterFlag::eSUPPRESS;
	}

	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	return PxFilterFlags();
}

bool physx::PxGetGroupCollisionFlag(const PxU16 group1, const PxU16 group2)
{
	PX_CHECK_AND_RETURN_NULL(group1 < 32 && group2 < 32, "Group must be less than 32");	

	return gCollisionTable[group1][group2]();
}

void physx::PxSetGroupCollisionFlag(const PxU16 group1, const PxU16 group2, const bool enable)
{
	PX_CHECK_AND_RETURN(group1 < 32 && group2 < 32, "Group must be less than 32");	

	gCollisionTable[group1][group2] = enable;
	gCollisionTable[group2][group1] = enable;
}

PxU16 physx::PxGetGroup(const PxActor& actor)
{
	PxFilterData fd;
	getFilterData(actor, fd);
	return PxU16(fd.word0);
}

void physx::PxSetGroup(PxActor& actor, const PxU16 collisionGroup)
{
	PX_CHECK_AND_RETURN(collisionGroup < 32,"Collision group must be less than 32");
	PxFilterData fd(collisionGroup, 0, 0, 0);
	setFilterData<false>(actor, fd);
}

void physx::PxGetFilterOps(PxFilterOp::Enum& op0, PxFilterOp::Enum& op1, PxFilterOp::Enum& op2)
{
	op0 = gFilterOps[0];
	op1 = gFilterOps[1];
	op2 = gFilterOps[2];
}

void physx::PxSetFilterOps(const PxFilterOp::Enum& op0, const PxFilterOp::Enum& op1, const PxFilterOp::Enum& op2)
{
	gFilterOps[0] = op0;
	gFilterOps[1] = op1;
	gFilterOps[2] = op2;
}

bool physx::PxGetFilterBool()
{
	return gFilterBool;
}

void physx::PxSetFilterBool(const bool enable)
{
	gFilterBool = enable;
}

void physx::PxGetFilterConstants(PxGroupsMask& c0, PxGroupsMask& c1)
{
	c0 = gFilterConstants[0];
	c1 = gFilterConstants[1];
}

void physx::PxSetFilterConstants(const PxGroupsMask& c0, const PxGroupsMask& c1)
{
	gFilterConstants[0] = c0;
	gFilterConstants[1] = c1;
}

PxGroupsMask physx::PxGetGroupsMask(const PxActor& actor)
{
	PxFilterData fd;
	if (getFilterData(actor, fd))
		return convert(fd);
	else
		return PxGroupsMask();
}

void physx::PxSetGroupsMask(PxActor& actor, const PxGroupsMask& mask)
{
	PxFilterData fd = convert(mask);
	setFilterData<true>(actor, fd);
}
