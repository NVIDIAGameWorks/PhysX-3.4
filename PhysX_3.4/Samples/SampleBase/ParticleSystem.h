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

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "PxPhysicsAPI.h"

#if PX_USE_PARTICLE_SYSTEM_API

#include "task/PxTask.h"
#include "pvd/PxPvd.h"
#include "extensions/PxParticleExt.h"
#include "PhysXSampleApplication.h"

// fwd to avoid including cuda.h
#if defined(__x86_64) || defined(AMD64) || defined(_M_AMD64)
typedef unsigned long long CUdeviceptr;
#else
typedef unsigned int CUdeviceptr;
#endif

/* 
	This is class for particle systems/fluids with constant rate emitter.
The class constructed using PxParticleBase object, the ownership of 
which is transferred to ParticleSystem object. Other parameters are mandatory: 
application pointer, billboard_size and texture object. Each particle rendered
as a quad, which normals always points to viewer. Texture for this quad must be supplied.
Depending on actual type of object created (PxParticleBase/PxParticleFluid) either particle system
could be created, or fluid. 
	This class has copy semantics, so it is possible to store objects of this 
type in container and periodically call it's member functions - update/draw.
	Particles' lifetime could be limited by lifetime. See setUseLifetime and 
setLifetime functions.
	In order to get particles created one must set emitter. Emitter is 2D shape (ellipse/rectangle)
located in some place in world coordinates, which emits particles with some rate. 
This shape also has size. Also, in order to get particles created one must call periodically update()
function.
	Worth mentioning, that particles count is limited by two or three factors - maximum number of particles, 
which is set, during PxParticleBase/PxParticleFluid object construction AND emitter rate 
AND IF SET particles lifetime.
*/
class ParticleSystem : public SampleAllocateable
{
private:
	PxParticleBase*			mParticleSystem;
	PxParticleExt::IndexPool* mIndexPool;
	std::vector<PxVec3>		mParticlesPositions;
	std::vector<PxVec3>		mParticlesVelocities;
	std::vector<PxMat33>	mParticlesOrientations;
	PxU32					mNumParticles;
	PxReal					mParticleLifetime;
	std::vector<PxReal>		mParticleLifes;
	PxU32*					mParticleValidity;
	PxU32                   mValidParticleRange;
	bool					mUseLifetime;
	bool					mUseInstancedMeshes;

	CUdeviceptr				mParticlesOrientationsDevice;
	CUdeviceptr				mParticleLifetimeDevice;
	CUdeviceptr				mParticleValidityDevice;	
	PxCudaContextManager*	mCtxMgr;

	void					modifyRotationMatrix(PxMat33& rotMatrix, PxReal deltaTime, const PxVec3& velocity);
	void					initializeParticlesOrientations();

public:
	class Visitor
        {
        public:
            virtual ~Visitor() {}
            virtual void visit(ParticleSystem& particleSystem) = 0;
        };
        void accept(Visitor& visitor) { visitor.visit(*this); }

	ParticleSystem(PxParticleBase* _particle_system, bool _useInstancedMeshes = false);
	~ParticleSystem();
	/* enables limiting particles lifetime */
	void setUseLifetime(bool use);
	/* returns true if limiting particles lifetime is enabled */
	bool useLifetime();
	/* NOTE! setUseLifetime(true) before setting this. Sets lifetime of particle to "lt" on emit */
	void setLifetime(PxReal lt);
	/* fetches particles positions from the PhysX SDK, 
	removes invalid particles 
	(intersected with drain, non-positive lifetime),
	creates new particles */
	void update(float deltaTime);
	/* creates particles in the PhysX SDK */
	void createParticles(struct ParticleData& particles);
	
	/* Returns pointer to the internal PxParticleBase */
	PxParticleBase* getPxParticleBase();
	/* Returns pointer to the particles positions */
	const std::vector<PxVec3>& getPositions();
	/* Returns pointer to the particles lifetimes */
	const std::vector<PxReal>& getLifetimes();
	/* Returns pointer to the particles velocities */
	const std::vector<PxVec3>& getVelocities();
	/* Returns pointer to the particles orientations */
	const std::vector<PxMat33>& getOrientations();
	/* Returns pointer to the particles validity */
	const PxU32* getValidity();
	/* Returns range of vaild particles index */
	PxU32 getValidParticleRange();
	/* Returns number of particles */
	PxU32 getNumParticles();

	/* Return the device copy of particles validity */
	CUdeviceptr getValiditiesDevice() const { return mParticleValidityDevice; }
	/* Return the device copy of particles orientation */
	CUdeviceptr getOrientationsDevice() const { return mParticlesOrientationsDevice; }
	/* Return the device copy of particle lifetimes*/
	CUdeviceptr getLifetimesDevice() const { return mParticleLifetimeDevice; }
	/* Return cuda context manager if set, which is only the case if interop is enabled*/
	PxCudaContextManager* getCudaContextManager() const { return mCtxMgr; }

public:
	PxParticleSystem& getPxParticleSystem() { PX_ASSERT(mParticleSystem); return static_cast<PxParticleSystem&>(*mParticleSystem); }
	PxParticleFluid& getPxParticleFluid() { PX_ASSERT(mParticleSystem); return static_cast<PxParticleFluid&>(*mParticleSystem); }

	PxU32 createParticles(const PxParticleCreationData& particles, PxStrideIterator<PxU32>* particleIndices = NULL, PxReal lifetime = 0.0f);
	PxU32 createParticleSphere(PxU32 maxParticles, float particleDistance, const PxVec3& center, const PxVec3& vel, PxReal lifetime = 0.0f, PxReal restOffsetVariance = 0.0f);
	PxU32 createParticleCube(PxU32 numX, PxU32 numY, PxU32 numZ, float particleDistance, const PxVec3& center, const PxVec3& vel, PxReal lifetime = 0.0f, PxReal restOffsetVariance = 0.0f);
	PxU32 createParticleCube(const PxBounds3& aabb, float particleDistance, const PxVec3& vel, PxReal lifetime = 0.0f, PxReal restOffsetVariance = 0.0f);
	PxU32 createParticleRand(PxU32 numParticles,const PxVec3& particleRange, const PxVec3& center, const PxVec3& vel, PxReal lifetime = 0.0f, PxReal restOffsetVariance = 0.0f);
	PxU32 createParticlesFromFile(const char* particleFileName);
	bool dumpParticlesToFile(const char* particleFileName);
	void releaseParticles(const SampleArray<PxU32>& indices);
	void releaseParticles();

private:
	PxU32 createParticles(const ParticleData& particles, PxReal lifetime);
};

#endif // PX_USE_PARTICLE_SYSTEM_API
#endif
