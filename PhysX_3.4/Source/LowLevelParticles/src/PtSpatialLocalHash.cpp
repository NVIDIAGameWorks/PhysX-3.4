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

#include "PtSpatialHashHelper.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PtSpatialHash.h"
#include "PtParticle.h"
#include "PsUtilities.h"

/*!
Builds local hash and reorders particle index table.
*/
void physx::Pt::SpatialHash::buildLocalHash(const Particle* particles, PxU32 numParticles, ParticleCell* cells,
                                            PxU32* particleIndices, PxU16* hashKeyArray, PxU32 numHashBuckets,
                                            PxF32 cellSizeInv, const PxVec3& packetCorner)
{
	PX_ASSERT(particles);
	PX_ASSERT(cells);
	PX_ASSERT(particleIndices);
	PX_ASSERT(numHashBuckets > numParticles); // Needs to be larger to have at least one empty hash bucket (required to
	// detect invalid cells).

	// Mark packet cell entries as empty.
	for(PxU32 c = 0; c < numHashBuckets; c++)
		cells[c].numParticles = PX_INVALID_U32;

	PX_ALIGN(16, Particle fakeParticle);
	fakeParticle.position = PxVec3(FLT_MAX, FLT_MAX, FLT_MAX);

	PxU32 numParticles4 = ((numParticles + 3) & ~0x3) + 4; // ceil up to multiple of four + 4 for save unrolling

	// Add particles to cell hash

	const Particle* prt0 = particles;
	const Particle* prt1 = (1 < numParticles) ? particles + 1 : &fakeParticle;
	const Particle* prt2 = (2 < numParticles) ? particles + 2 : &fakeParticle;
	const Particle* prt3 = (3 < numParticles) ? particles + 3 : &fakeParticle;

	struct Int32Vec3
	{
		PX_FORCE_INLINE void set(const PxVec3& realVec, const PxF32 scale)
		{
			x = static_cast<PxI32>(Ps::floor(realVec.x * scale));
			y = static_cast<PxI32>(Ps::floor(realVec.y * scale));
			z = static_cast<PxI32>(Ps::floor(realVec.z * scale));
		}
		PxI32 x;
		PxI32 y;
		PxI32 z;
	};

	PX_ALIGN(16, Int32Vec3 cellCoords[8]);
	cellCoords[0].set(prt0->position - packetCorner, cellSizeInv);
	cellCoords[1].set(prt1->position - packetCorner, cellSizeInv);
	cellCoords[2].set(prt2->position - packetCorner, cellSizeInv);
	cellCoords[3].set(prt3->position - packetCorner, cellSizeInv);

	for(PxU32 p = 0; p < numParticles4; p += 4)
	{
		const Particle* prt0_N = (p + 4 < numParticles) ? particles + p + 4 : &fakeParticle;
		const Particle* prt1_N = (p + 5 < numParticles) ? particles + p + 5 : &fakeParticle;
		const Particle* prt2_N = (p + 6 < numParticles) ? particles + p + 6 : &fakeParticle;
		const Particle* prt3_N = (p + 7 < numParticles) ? particles + p + 7 : &fakeParticle;

		PxU32 wIndex = (p + 4) & 7;
		cellCoords[wIndex].set(prt0_N->position - packetCorner, cellSizeInv);
		cellCoords[wIndex + 1].set(prt1_N->position - packetCorner, cellSizeInv);
		cellCoords[wIndex + 2].set(prt2_N->position - packetCorner, cellSizeInv);
		cellCoords[wIndex + 3].set(prt3_N->position - packetCorner, cellSizeInv);

		PxU32 rIndex = p & 7;
		for(PxU32 i = 0; i < 4; ++i)
		{
			if(p + i < numParticles)
			{
				const Int32Vec3& int32Vec3 = cellCoords[rIndex + i];
				const GridCellVector cellCoord(PxI16(int32Vec3.x), PxI16(int32Vec3.y), PxI16(int32Vec3.z));
				PxU32 hashKey = getCellIndex(cellCoord, cells, numHashBuckets);
				PX_ASSERT(hashKey < PT_PARTICLE_SYSTEM_HASH_KEY_LIMIT);
				ParticleCell* cell = &cells[hashKey];
				hashKeyArray[p + i] = Ps::to16(hashKey);
				PX_ASSERT(cell);

				if(cell->numParticles == PX_INVALID_U32)
				{
					// Entry is empty -> Initialize new entry
					cell->coords = cellCoord;
					cell->numParticles = 1; // this avoids some LHS
				}
				else
				{
					cell->numParticles++; // this avoids some LHS
				}
				PX_ASSERT(cell->numParticles != PX_INVALID_U32);
			}
		}
	}

	// Set for each cell the starting index of the associated particle index interval.
	PxU32 cellFirstParticle = 0;
	for(PxU32 c = 0; c < numHashBuckets; c++)
	{
		ParticleCell& cell = cells[c];

		if(cell.numParticles == PX_INVALID_U32)
			continue;

		cell.firstParticle = cellFirstParticle;
		cellFirstParticle += cell.numParticles;
	}

	reorderParticleIndicesToCells(particles, numParticles, cells, particleIndices, numHashBuckets, hashKeyArray);
}

/*!
Reorders particle indices to cells.
*/
void physx::Pt::SpatialHash::reorderParticleIndicesToCells(const Particle* /*particles*/, PxU32 numParticles,
                                                           ParticleCell* cells, PxU32* particleIndices,
                                                           PxU32 numHashBuckets, PxU16* hashKeyArray)
{
	for(PxU32 c = 0; c < numHashBuckets; c++)
	{
		ParticleCell& cell = cells[c];
		if(cell.numParticles == PX_INVALID_U32)
			continue;

		cell.numParticles = 0;
	}

	// Reorder particle indices according to cells
	for(PxU32 p = 0; p < numParticles; p++)
	{
		// Get cell for fluid
		ParticleCell* cell;
		cell = &cells[hashKeyArray[p]];

		PX_ASSERT(cell);
		PX_ASSERT(cell->numParticles != PX_INVALID_U32);

		particleIndices[cell->firstParticle + cell->numParticles] = p;
		cell->numParticles++;
	}
}

#endif // PX_USE_PARTICLE_SYSTEM_API
