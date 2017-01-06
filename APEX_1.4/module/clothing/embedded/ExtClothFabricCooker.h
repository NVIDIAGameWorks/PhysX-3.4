/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_PHYSICS_EXTENSIONS_CLOTH_FABRIC_COOKER_H
#define PX_PHYSICS_EXTENSIONS_CLOTH_FABRIC_COOKER_H

/** \addtogroup extensions
  @{
*/

#include "PxClothMeshDesc.h"
#include "PxClothFabric.h"

#if PX_DOXYGEN == 0
namespace nvidia
{
#endif

struct PxFabricCookerImpl;

class PxClothFabricCooker
{
public:
	/**
	\brief Cooks a triangle mesh to a PxClothFabricDesc.
	\param desc The cloth mesh descriptor on which the generation of the cooked mesh depends.
	\param gravity A normalized vector which specifies the direction of gravity. 
	This information allows the cooker to generate a fabric with higher quality simulation behavior.
	\param useGeodesicTether A flag to indicate whether to compute geodesic distance for tether constraints.
	\note The geodesic option for tether only works for manifold input.  For non-manifold input, a simple Euclidean distance will be used.
	For more detailed cooker status for such cases, try running PxClothGeodesicTetherCooker directly.
	*/
	PxClothFabricCooker(const PxClothMeshDesc& desc, const physx::PxVec3& gravity, bool useGeodesicTether = true);
	~PxClothFabricCooker();

	/** \brief Returns the fabric descriptor to create the fabric. */
	PxClothFabricDesc getDescriptor() const;
	/** \brief Saves the fabric data to a platform and version dependent stream. */
	void save(physx::PxOutputStream& stream, bool platformMismatch) const;

private:
	PxFabricCookerImpl* mImpl;
};

#if PX_DOXYGEN == 0
} // namespace nvidia
#endif

/** @} */
#endif // PX_PHYSICS_EXTENSIONS_CLOTH_FABRIC_COOKER_H
