/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_FIELD_SAMPLER_H
#define MODULE_FIELD_SAMPLER_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief This is an optional callback interface for collision filtering between field samplers and other objects
	   If this interface is not provided, then collision filtering happens normally through the default scene simulationfiltershader callback
	   However, if this is provided, then the user can no only return true/false to indicate whether or not the two objects should interact but
	   can also assign a weighted multiplier value to control how strongly the two objects should interact.
*/
class FieldSamplerWeightedCollisionFilterCallback
{
public:
	/**
	\brief This is an optional callback interface for collision filtering between field samplers and other objects
		   If this interface is not provided, then collision filtering happens normally through the default scene simulationfiltershader callback
		   However, if this is provided, then the user can no only return true/false to indicate whether or not the two objects should interact but
		   can also assign a weighted multiplier value to control how strongly the two objects should interact.
	*/
	virtual bool fieldSamplerWeightedCollisionFilter(const physx::PxFilterData &objectA,const physx::PxFilterData &objectB,float &multiplierValue) = 0;
};


/**
 \brief FieldSampler module class
 */
class ModuleFieldSampler : public Module
{
protected:
	virtual					~ModuleFieldSampler() {}

public:

	/**
	\brief Sets the optional weighted collision filter callback for this scene. If not provided, it will use the default SimulationFilterShader on the current scene
	*/
	virtual bool			setFieldSamplerWeightedCollisionFilterCallback(const Scene& apexScene,FieldSamplerWeightedCollisionFilterCallback *callback) = 0;

	
#if PX_PHYSICS_VERSION_MAJOR == 3
	/**
		Set flag to toggle PhysXMonitor for ForceFields.
	*/
	virtual void			enablePhysXMonitor(const Scene& apexScene, bool enable) = 0;
	/**
	\brief Add filter data (collision group) to PhysXMonitor. 
		
	\param apexScene [in] - Apex scene for which to submit the force sample batch.
	\param filterData [in] - PhysX 3.0 collision mask for PhysXMonitor
	*/
	virtual void			setPhysXMonitorFilterData(const Scene& apexScene, physx::PxFilterData filterData) = 0;
#endif

	/**
	\brief Initialize a query for a batch of sampling points

	\param apexScene [in] - Apex scene for which to create the force sample batch query.
	\param maxCount [in] - Maximum number of indices (active samples)
	\param filterData [in] - PhysX 3.0 collision mask for data
	\return the ID of created query
	*/
	virtual uint32_t		createForceSampleBatch(const Scene& apexScene, uint32_t maxCount, const physx::PxFilterData filterData) = 0;

	/**
	\brief Release a query for a batch of sampling points

	\param apexScene [in] - Apex scene for which to create the force sample batch.
	\param batchId [in] - ID of query that should be released
	*/
	virtual void			releaseForceSampleBatch(const Scene& apexScene, uint32_t batchId) = 0;

	/**
	\brief Submits a batch of sampling points to be evaluated during the simulation step.
	
	\param apexScene [in] - Apex scene for which to submit the force sample batch.
	\param batchId [in] - ID of query for force sample batch.
	\param forces [out] - Buffer to which computed forces are written to. The buffer needs to be persistent between calling this function and the next PxApexScene::fetchResults.
	\param forcesStride [in] - Stride between consecutive force vectors within the forces output.
	\param positions [in] - Buffer containing the positions of the input samples.
	\param positionsStride [in] - Stride between consecutive position vectors within the positions input.
	\param velocities [in] - Buffer containing the velocities of the input samples.
	\param velocitiesStride [in] - Stride between consecutive velocity vectors within the velocities input.
	\param mass [in] - Buffer containing the mass of the input samples.
	\param massStride [in] - Stride between consecutive mass values within the mass input.
	\param indices [in] - Buffer containing the indices of the active samples that are considered for the input and output buffers.
	\param numIndices [in] - Number of indices (active samples). 
	*/
	virtual void			submitForceSampleBatch(	const Scene& apexScene, uint32_t batchId,
													PxVec4* forces, const uint32_t forcesStride, 
													const PxVec3* positions, const uint32_t positionsStride, 
													const PxVec3* velocities, const uint32_t velocitiesStride,
													const float* mass, const uint32_t massStride,
													const uint32_t* indices, const uint32_t numIndices) = 0;

};



PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // MODULE_FIELD_SAMPLER_H
