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


#ifndef PX_PHYSICS_NP_CLOTH
#define PX_PHYSICS_NP_CLOTH

#include "PxPhysXConfig.h"

#if PX_USE_CLOTH_API

#include "PxCloth.h"
#include "NpActorTemplate.h"
#include "ScbCloth.h"


namespace physx
{
class NpClothFabric;

typedef NpActorTemplate<PxCloth> NpClothT;

class NpCloth : public NpClothT
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
									NpCloth(PxBaseFlags baseFlags);
	virtual		void				requires(PxProcessPxBaseCallback& c);
	virtual		void				exportExtraData(PxSerializationContext& stream) { mCloth.exportExtraData(stream); }
				void				importExtraData(PxDeserializationContext& context) { mCloth.importExtraData(context); }
				void				resolveReferences(PxDeserializationContext& context);
	static		NpCloth*			createObject(PxU8*& address, PxDeserializationContext& context);
	static      void				getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
									NpCloth(const PxTransform& globalPose, NpClothFabric& fabric, const PxClothParticle* particles, PxClothFlags flags);
	virtual							~NpCloth();
	NpCloth &operator=(const NpCloth &);

	//---------------------------------------------------------------------------------
	// PxCloth implementation
	//---------------------------------------------------------------------------------
	virtual		void				release();

	virtual		PxActorType::Enum	getType() const;

	virtual		PxClothFabric*		getFabric() const;

	virtual		void				setParticles(const PxClothParticle* currentParticles, const PxClothParticle* previousParticles);
	virtual		PxU32				getNbParticles() const;

	virtual		void				setMotionConstraints(const PxClothParticleMotionConstraint* motionConstraints);
	virtual		bool				getMotionConstraints(PxClothParticleMotionConstraint* motionConstraintsBuffer) const;
	virtual		PxU32				getNbMotionConstraints() const;

	virtual		void				setMotionConstraintConfig(const PxClothMotionConstraintConfig& config);
	virtual		PxClothMotionConstraintConfig getMotionConstraintConfig() const;

	virtual     void                setSeparationConstraints(const PxClothParticleSeparationConstraint* separationConstraints);
	virtual     bool                getSeparationConstraints(PxClothParticleSeparationConstraint* separationConstraintsBuffer) const;
	virtual     PxU32               getNbSeparationConstraints() const;

	virtual		void				clearInterpolation();

	virtual     void                setParticleAccelerations(const PxVec4* particleAccelerations);
	virtual     bool                getParticleAccelerations(PxVec4* particleAccelerationsBuffer) const;
	virtual     PxU32               getNbParticleAccelerations() const;

	virtual		void				addCollisionSphere(const PxClothCollisionSphere& sphere);
	virtual		void				removeCollisionSphere(PxU32 index);
	virtual		void				setCollisionSpheres(const PxClothCollisionSphere* spheresBuffer, PxU32 count);
	virtual		PxU32				getNbCollisionSpheres() const;

	virtual		void				getCollisionData(PxClothCollisionSphere* spheresBuffer, PxU32* capsulesBuffer,
										PxClothCollisionPlane* planesBuffer, PxU32* convexesBuffer, PxClothCollisionTriangle* trianglesBuffer) const;

	virtual		void				addCollisionCapsule(PxU32 first, PxU32 second);
	virtual		void				removeCollisionCapsule(PxU32 index);
	virtual		PxU32				getNbCollisionCapsules() const;

	virtual		void				addCollisionTriangle(const PxClothCollisionTriangle& triangle);
	virtual		void				removeCollisionTriangle(PxU32 index);
	virtual		void				setCollisionTriangles(const PxClothCollisionTriangle* trianglesBuffer, PxU32 count);
	virtual		PxU32				getNbCollisionTriangles() const;

	virtual		void				addCollisionPlane(const PxClothCollisionPlane& plane);
	virtual		void				removeCollisionPlane(PxU32 index);
	virtual		void				setCollisionPlanes(const PxClothCollisionPlane* planesBuffer, PxU32 count);
	virtual		PxU32				getNbCollisionPlanes() const;

	virtual		void				addCollisionConvex(PxU32 mask);
	virtual		void				removeCollisionConvex(PxU32 index);
	virtual		PxU32				getNbCollisionConvexes() const;

	virtual		void				setVirtualParticles(PxU32 numParticles, const PxU32* indices, PxU32 numWeights, const PxVec3* weights);

	virtual		PxU32				getNbVirtualParticles() const;
	virtual		void				getVirtualParticles(PxU32* indicesBuffer) const;

	virtual		PxU32				getNbVirtualParticleWeights() const;
	virtual		void				getVirtualParticleWeights(PxVec3* weightsBuffer) const;

	virtual		void				setGlobalPose(const PxTransform& pose);
	virtual		PxTransform			getGlobalPose() const;

	virtual		void				setTargetPose(const PxTransform& pose);

	virtual		void				setExternalAcceleration(PxVec3 acceleration);
	virtual		PxVec3				getExternalAcceleration() const;

	virtual		void				setLinearInertiaScale(PxVec3 scale);
	virtual		PxVec3				getLinearInertiaScale() const;
	virtual		void				setAngularInertiaScale(PxVec3 scale);
	virtual		PxVec3				getAngularInertiaScale() const;
	virtual		void				setCentrifugalInertiaScale(PxVec3 scale);
	virtual		PxVec3				getCentrifugalInertiaScale() const;
	virtual		void				setInertiaScale(float);

	virtual		void				setDampingCoefficient(PxVec3 dampingCoefficient);
	virtual		PxVec3				getDampingCoefficient() const;

	virtual		void				setFrictionCoefficient(PxReal frictionCoefficient);
	virtual		PxReal				getFrictionCoefficient() const;

	virtual		void				setLinearDragCoefficient(PxVec3 dampingCoefficient);
	virtual		PxVec3				getLinearDragCoefficient() const;
	virtual		void				setAngularDragCoefficient(PxVec3 dampingCoefficient);
	virtual		PxVec3				getAngularDragCoefficient() const;
	virtual     void				setDragCoefficient(float);

	virtual		void				setCollisionMassScale(PxReal scalingCoefficient);
	virtual		PxReal				getCollisionMassScale() const;

	virtual		void				setSelfCollisionDistance(PxReal distance);
	virtual		PxReal				getSelfCollisionDistance() const;
	virtual		void				setSelfCollisionStiffness(PxReal stiffness);
	virtual		PxReal				getSelfCollisionStiffness() const;

	virtual		void				setSelfCollisionIndices(const PxU32* indices, PxU32 nbIndices);
	virtual		bool				getSelfCollisionIndices(PxU32* indices) const;
	virtual		PxU32				getNbSelfCollisionIndices() const;

	virtual		void				setRestPositions(const PxVec4* restPositions);
	virtual		bool				getRestPositions(PxVec4* restPositions) const;
	virtual		PxU32				getNbRestPositions() const; 

	virtual		void				setSolverFrequency(PxReal);
	virtual		PxReal				getSolverFrequency() const;

	virtual		void				setStiffnessFrequency(PxReal);
	virtual		PxReal				getStiffnessFrequency() const;

	virtual		void				setStretchConfig(PxClothFabricPhaseType::Enum type, const PxClothStretchConfig& config);
	virtual		PxClothStretchConfig getStretchConfig(PxClothFabricPhaseType::Enum type) const;

	virtual		void				setTetherConfig(const PxClothTetherConfig& config);
	virtual		PxClothTetherConfig	getTetherConfig() const;

	virtual		void				setClothFlags(PxClothFlags flags);
	virtual		void				setClothFlag(PxClothFlag::Enum flag, bool val);
	virtual		PxClothFlags		getClothFlags() const;

	virtual     PxVec3				getWindVelocity() const;
	virtual     void				setWindVelocity(PxVec3);
	virtual     PxReal				getWindDrag() const;
	virtual     void				setWindDrag(PxReal);
	virtual     PxReal				getWindLift() const;
	virtual     void				setWindLift(PxReal);

	virtual		bool				isSleeping() const;
	virtual		PxReal				getSleepLinearVelocity() const;
	virtual		void				setSleepLinearVelocity(PxReal threshold);
	virtual		void				setWakeCounter(PxReal wakeCounterValue);
	virtual		PxReal				getWakeCounter() const;
	virtual		void				wakeUp();
	virtual		void				putToSleep();

	virtual		PxClothParticleData* lockParticleData(PxDataAccessFlags);
	virtual		PxClothParticleData* lockParticleData() const;


	virtual		PxReal				getPreviousTimeStep() const;

	virtual		PxBounds3			getWorldBounds(float inflation=1.01f) const;

	virtual		void				setSimulationFilterData(const PxFilterData& data);
	virtual		PxFilterData		getSimulationFilterData() const;

	virtual		void				setContactOffset(PxReal);
	virtual		PxReal				getContactOffset() const;
	virtual		void				setRestOffset(PxReal);
	virtual		PxReal				getRestOffset() const;

	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
public:

	PX_FORCE_INLINE	Scb::Cloth&			getScbCloth() { return mCloth; }
	PX_FORCE_INLINE	const Scb::Cloth&	getScbCloth() const { return mCloth; }

#if PX_CHECKED 
	static bool checkParticles(PxU32 numParticles, const PxClothParticle* particles);
	static bool checkMotionConstraints(PxU32 numConstraints, const PxClothParticleMotionConstraint* constraints);
	static bool checkSeparationConstraints(PxU32 numConstraints, const PxClothParticleSeparationConstraint* constraints);
	static bool checkParticleAccelerations(PxU32 numParticles, const PxVec4* accelerations);
	static bool checkCollisionSpheres(PxU32 numSpheres, const PxClothCollisionSphere* spheres);
	static bool checkCollisionSpherePairs(PxU32 numSpheres, PxU32 numPairs, const PxU32* pairIndices);
	static bool checkVirtualParticles(PxU32 numParticles, PxU32 numVParticles, const PxU32* indices, PxU32 numWeights, const PxVec3* weights);
#endif

#if PX_ENABLE_DEBUG_VISUALIZATION
	virtual		void				visualize(Cm::RenderOutput& out, NpScene* scene);
#endif

	void							unlockParticleData();

	
private:
	
	void sendPvdSimpleProperties();
	void sendPvdMotionConstraints();
	void sendPvdSeparationConstraints();
	void sendPvdCollisionSpheres();
	void sendPvdCollisionCapsules();
	void sendPvdCollisionTriangles();
	void sendPvdVirtualParticles();
	void sendPvdSelfCollisionIndices();
	void sendPvdRestPositions();
	Scb::Cloth			mCloth;
	NpClothFabric*		mClothFabric;
	NpClothParticleData mParticleData;
};

}

#endif // PX_USE_CLOTH_API
#endif
