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


#ifndef PX_PHYSICS_NX_CLOTH_FABRIC
#define PX_PHYSICS_NX_CLOTH_FABRIC
/** \addtogroup cloth
  @{
*/


#if PX_DOXYGEN == 0
namespace nvidia
{
#endif

/**
\brief Describe type of phase in cloth fabric.
\see PxClothFabric for an explanation of concepts on phase and set.
*/
struct PxClothFabricPhaseType
{
	enum Enum
	{
		eINVALID,     //!< invalid type 
		eVERTICAL,    //!< resists stretching or compression, usually along the gravity
		eHORIZONTAL,  //!< resists stretching or compression, perpendicular to the gravity
		eBENDING,     //!< resists out-of-plane bending in angle-based formulation
		eSHEARING,    //!< resists in-plane shearing along (typically) diagonal edges,
        eCOUNT        // internal use only
	};
};

/**
\brief References a set of constraints that can be solved in parallel.
\see PxClothFabric for an explanation of the concepts on phase and set.
*/
struct PxClothFabricPhase
{
	PxClothFabricPhase(PxClothFabricPhaseType::Enum type = 
		PxClothFabricPhaseType::eINVALID, uint32_t index = 0);

	/**
	\brief Type of constraints to solve.
	*/
	PxClothFabricPhaseType::Enum phaseType;

	/**
	\brief Index of the set that contains the particle indices.
	*/
	uint32_t setIndex;
};

PX_INLINE PxClothFabricPhase::PxClothFabricPhase(
	PxClothFabricPhaseType::Enum type, uint32_t index)
	: phaseType(type)
	, setIndex(index)
{}

/**
\brief References all the data required to create a fabric.
\see PxPhysics.createClothFabric(), PxClothFabricCooker.getDescriptor()
*/
class PxClothFabricDesc
{
public:
	/** \brief The number of particles needed when creating a PxCloth instance from the fabric. */
	uint32_t nbParticles;

	/** \brief The number of solver phases. */
	uint32_t nbPhases;
	/** \brief Array defining which constraints to solve each phase. See #PxClothFabric.getPhases(). */
	const PxClothFabricPhase* phases;

	/** \brief The number of sets in the fabric. */
	uint32_t nbSets;
	/** \brief Array with an index per set which points one entry beyond the last constraint of the set. See #PxClothFabric.getSets(). */
	const uint32_t* sets;

	/** \brief Array of particle indices which specifies the pair of constrained vertices. See #PxClothFabric.getParticleIndices(). */
	const uint32_t* indices;
	/** \brief Array of rest values for each constraint. See #PxClothFabric.getRestvalues(). */
	const float* restvalues;

	/** \brief Size of tetherAnchors and tetherLengths arrays, needs to be multiple of nbParticles. */
	uint32_t nbTethers;
	/** \brief Array of particle indices specifying the tether anchors. See #PxClothFabric.getTetherAnchors(). */
	const uint32_t* tetherAnchors;
	/** \brief Array of rest distance between tethered particle pairs. See #PxClothFabric.getTetherLengths(). */
	const float* tetherLengths;

	/**
	\brief constructor sets to default.
	*/
	PX_INLINE PxClothFabricDesc();

	/**
	\brief (re)sets the structure to the default.	
	*/
	PX_INLINE void setToDefault();

	/**
	\brief Returns true if the descriptor is valid.
	\return True if the current settings are valid
	*/
	PX_INLINE bool isValid() const;
};

PX_INLINE PxClothFabricDesc::PxClothFabricDesc()
{
	setToDefault();
}

PX_INLINE void PxClothFabricDesc::setToDefault()
{
	memset(this, 0, sizeof(PxClothFabricDesc));
}

PX_INLINE bool PxClothFabricDesc::isValid() const
{
	return (nbParticles && nbPhases && phases && restvalues && nbSets 
		&& sets && indices && (!nbTethers || (tetherAnchors && tetherLengths)));
}


#if PX_DOXYGEN == 0
} // namespace nvidia
#endif

/** @} */
#endif
