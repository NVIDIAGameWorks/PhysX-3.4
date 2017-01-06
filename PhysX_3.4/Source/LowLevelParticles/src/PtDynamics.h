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

#ifndef PT_DYNAMICS_H
#define PT_DYNAMICS_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PtConfig.h"
#include "PtParticle.h"
#include "PtDynamicsParameters.h"
#include "PtDynamicsTempBuffers.h"
#include "CmBitMap.h"
#include "CmTask.h"

namespace physx
{

namespace Pt
{

struct ParticleCell;
struct PacketSections;
struct PacketHaloRegions;

class Dynamics
{
  public:
	Dynamics(class ParticleSystemSimCpu& particleSystem);
	~Dynamics();

	void clear();

	void updateSph(physx::PxBaseTask& continuation);

	PX_FORCE_INLINE DynamicsParameters& getParameter()
	{
		return mParams;
	}

  private:
	// Table to get the neighboring halo region indices for a packet section
	struct SectionToHaloTable
	{
		PxU32 numHaloRegions;
		PxU32 haloRegionIndices[19]; // No packet section has more than 19 neighboring halo regions
	};

	struct OrderedIndexTable
	{
		OrderedIndexTable();
		PxU32 indices[PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY];
	};

	struct TaskData
	{
		PxU16 beginPacketIndex;
		PxU16 endPacketIndex;
	};

	void adjustTempBuffers(PxU32 count);

	void schedulePackets(SphUpdateType::Enum updateType, physx::PxBaseTask& continuation);
	void processPacketRange(PxU32 taskDataIndex);

	void updatePacket(SphUpdateType::Enum updateType, PxVec3* forceBuf, Particle* particles, const ParticleCell& packet,
	                  const PacketSections& packetSections, const PacketHaloRegions& haloRegions,
	                  struct DynamicsTempBuffers& tempBuffers);

	void updatePacketLocalHash(SphUpdateType::Enum updateType, PxVec3* forceBuf, Particle* particles,
	                           const ParticleCell& packet, const PacketSections& packetSections,
	                           const PacketHaloRegions& haloRegions, DynamicsTempBuffers& tempBuffers);

	void updateSubpacketPairHalo(PxVec3* __restrict forceBufA, Particle* __restrict particlesSpA, PxU32 numParticlesSpA,
	                             ParticleCell* __restrict particleCellsSpA, PxU32* __restrict particleIndicesSpA,
	                             bool& isLocalHashSpAValid, PxU32 numCellHashBucketsSpA,
	                             Particle* __restrict particlesSpB, PxU32 numParticlesSpB,
	                             ParticleCell* __restrict particleCellsSpB, PxU32* __restrict particleIndicesSpB,
	                             const PxVec3& packetCorner, SphUpdateType::Enum updateType,
	                             PxU16* __restrict hashKeyArray, DynamicsTempBuffers& tempBuffers);

	PX_FORCE_INLINE void updateParticlesBruteForceHalo(SphUpdateType::Enum updateType, PxVec3* forceBuf,
	                                                   Particle* particles, const PacketSections& packetSections,
	                                                   const PacketHaloRegions& haloRegions,
	                                                   DynamicsTempBuffers& tempBuffers);

	void mergeDensity(physx::PxBaseTask* continuation);
	void mergeForce(physx::PxBaseTask* continuation);

  private:
	Dynamics& operator=(const Dynamics&);
	static SectionToHaloTable sSectionToHaloTable[26]; // Halo region table for each packet section
	static OrderedIndexTable sOrderedIndexTable;

	PX_ALIGN(16, DynamicsParameters mParams);
	class ParticleSystemSimCpu& mParticleSystem;
	Particle* mTempReorderedParticles;
	PxVec3* mTempParticleForceBuf;

	typedef Cm::DelegateTask<Dynamics, &Dynamics::mergeDensity> MergeDensityTask;
	typedef Cm::DelegateTask<Dynamics, &Dynamics::mergeForce> MergeForceTask;

	MergeDensityTask mMergeDensityTask;
	MergeForceTask mMergeForceTask;
	PxU32 mNumTasks;
	SphUpdateType::Enum mCurrentUpdateType;
	PxU32 mNumTempBuffers;
	DynamicsTempBuffers mTempBuffers[PT_MAX_PARALLEL_TASKS_SPH];
	TaskData mTaskData[PT_MAX_PARALLEL_TASKS_SPH];
	friend class DynamicsSphTask;
};

} // namespace Pt
} // namespace physx

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_DYNAMICS_H
