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

#ifndef PT_CONFIG_H
#define PT_CONFIG_H

#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "PtParticleSystemFlags.h"

// Marker for fluid particles with no collision constraints
#define PT_NO_CONSTRAINT PT_PARTICLE_SYSTEM_PARTICLE_LIMIT

// Needs to be addressable with PxU16 Particle::hashKey
// - Ps::nextPowerOf2((PXD_PARTICLE_SYSTEM_HASH_KEY_LIMIT + 1)) must be addressable
//   through PxU16 Particle::hashKey, see and PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY
//   Dynamics::updatePacket()
#define PT_PARTICLE_SYSTEM_HASH_KEY_LIMIT 0x7ffe

// Size of particle packet hash table.
// - Must be a power of 2
// - Must be at least as large as PT_PARTICLE_SYSTEM_PACKET_LIMIT (see further below), but should be larger for the hash
// to be efficient.
// - Must to be addressable through PxU16 Pt::Particle::hashKey.
#define PT_PARTICLE_SYSTEM_PACKET_HASH_SIZE 1024

// One larger than PT_PARTICLE_SYSTEM_PACKET_HASH_SIZE to fit a special cell for overflow particles.
#define PT_PARTICLE_SYSTEM_PACKET_HASH_BUFFER_SIZE 1025

// Index of special overflow packet
#define PT_PARTICLE_SYSTEM_OVERFLOW_INDEX 1024

// Maximum number of particle packets (should be smaller than hash size since a full hash table is not efficient)
#define PT_PARTICLE_SYSTEM_PACKET_LIMIT 924

// Slack for building the triangle packet hash. Has to be bigger than any epsilons used in collision detection.
#define PT_PARTICLE_SYSTEM_COLLISION_SLACK 1.0e-3f

// Maximum number of fluid particles in a packet that can be handled at a time
#define PT_SUBPACKET_PARTICLE_LIMIT 512
// If the number of particles in a packet and the number of particles for each neighboring halo region
// are below this threshold, then no local hash will be constructed and each particle of one packet will be
// tested against each particle of the other packet (for particle-particle interaction only).
//
// Note: Has to be smaller or equal to PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY
#define PT_BRUTE_FORCE_PARTICLE_THRESHOLD 100
// If the number of particles in a packet section and the number of particles in a neighboring halo
// region are below this threshold, then no local hash will be constructed and each particle of the
// packet section will be tested against each particle of the halo region (for particle-particle interaction only).
//
// Note: Has to be smaller or equal to PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY
#define PT_BRUTE_FORCE_PARTICLE_THRESHOLD_HALO_VS_SECTION 200

// Maximum number of fluid particles in a packet that can be handled at a time for dividing
// a packet into sections and reordering the particles accordingly
#define PT_SUBPACKET_PARTICLE_LIMIT_PACKET_SECTIONS PT_SUBPACKET_PARTICLE_LIMIT

// Maximum number of fluid particles in a packet that can be handled at a time for SPH dynamics
// calculations, i.e., computation of density & force
// - Ps::nextPowerOf2((PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY + 1)) must be addressable
//   through PxU16 Particle::hashKey, see Dynamics::updatePacket().
#define PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY PT_SUBPACKET_PARTICLE_LIMIT

//  loacl hash bucket size,  should equal nextPowerOfTwo(PT_SUBPACKET_PARTICLE_LIMIT_FORCE_DENSITY + 1)
#define PT_SUBPACKET_PARTICLE_HASH_BUCKET_SIZE 512

// Maximum number of parallel tasks created for sph computation
#define PT_MAX_PARALLEL_TASKS_SPH 8

// Maximum number of fluid particles in a packet that can be handled at a time for velocity
// integration
#define PT_SUBPACKET_PARTICLE_LIMIT_VEL_INTEGRATION PT_SUBPACKET_PARTICLE_LIMIT

// Maximum number of fluid particles in a packet that can be handled at a time for
// detecting and resolving collisions.
// - Must be smaller than PT_LOCAL_HASH_SIZE_MESH_COLLISION.
#define PT_SUBPACKET_PARTICLE_LIMIT_COLLISION 128

// Hash size for the local particle cell hash.
// - Must to be larger than PT_SUBPACKET_PARTICLE_LIMIT_COLLISION
// - Must be a power of 2
// - Must be addressable with PxU16 Particle::hashKey
#define PT_LOCAL_HASH_SIZE_MESH_COLLISION 256

// Number of fluid packet shapes to run in parallel during collision update.
#define PT_NUM_PACKETS_PARALLEL_COLLISION 8

// Initial size of triangle mesh collision buffer (for storing indices of colliding triangles)
#define PT_INITIAL_MESH_COLLISION_BUFFER_SIZE 1024

#define PT_USE_SIMD_CONVEX_COLLISION 1

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif // PT_CONFIG_H
