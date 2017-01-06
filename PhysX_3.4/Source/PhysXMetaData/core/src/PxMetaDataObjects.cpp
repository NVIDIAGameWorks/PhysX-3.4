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

#include "PsFoundation.h"
#include "PsUtilities.h"
#include "CmPhysXCommon.h"

#include "PxMetaDataObjects.h"
#include "PxPhysicsAPI.h"
#include "NpClothFabric.h"

using namespace physx;

PX_PHYSX_CORE_API PxGeometryType::Enum PxShapeGeometryPropertyHelper::getGeometryType(const PxShape* inShape) const { return inShape->getGeometryType(); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxBoxGeometry& geometry) const { return inShape->getBoxGeometry( geometry ); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxSphereGeometry& geometry) const { return inShape->getSphereGeometry( geometry ); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxCapsuleGeometry& geometry) const { return inShape->getCapsuleGeometry( geometry ); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxPlaneGeometry& geometry) const { return inShape->getPlaneGeometry( geometry ); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxConvexMeshGeometry& geometry) const { return inShape->getConvexMeshGeometry( geometry ); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxTriangleMeshGeometry& geometry) const { return inShape->getTriangleMeshGeometry( geometry ); }
PX_PHYSX_CORE_API bool PxShapeGeometryPropertyHelper::getGeometry(const PxShape* inShape, PxHeightFieldGeometry& geometry) const { return inShape->getHeightFieldGeometry( geometry ); }

PX_PHYSX_CORE_API void PxShapeMaterialsPropertyHelper::setMaterials(PxShape* inShape, PxMaterial*const* materials, PxU16 materialCount) const
{
	inShape->setMaterials( materials, materialCount );
}

PX_PHYSX_CORE_API PxShape* PxRigidActorShapeCollectionHelper::createShape(PxRigidActor* inActor, const PxGeometry& geometry, PxMaterial& material,
														PxShapeFlags shapeFlags ) const
{
	return PxRigidActorExt::createExclusiveShape(*inActor, geometry, material, shapeFlags );
}
PX_PHYSX_CORE_API PxShape* PxRigidActorShapeCollectionHelper::createShape(PxRigidActor* inActor, const PxGeometry& geometry, PxMaterial *const* materials,
														PxU16 materialCount, PxShapeFlags shapeFlags ) const
{
	return PxRigidActorExt::createExclusiveShape(*inActor, geometry, materials, materialCount, shapeFlags );
}

PX_PHYSX_CORE_API PxArticulationLink*	PxArticulationLinkCollectionPropHelper::createLink(PxArticulation* inArticulation, PxArticulationLink* parent,
																	   const PxTransform& pose) const
{
	PX_CHECK_AND_RETURN_NULL(pose.isValid(), "PxArticulationLinkCollectionPropHelper::createLink pose is not valid.");
	return inArticulation->createLink(parent, pose );
}



/*
	typedef void (*TSetterType)( TObjType*, TIndexType, TPropertyType );
	typedef TPropertyType (*TGetterType)( const TObjType* inObj, TIndexType );
	*/

inline void SetNbBroadPhaseAdd( PxSimulationStatistics* inStats, PxSimulationStatistics::VolumeType data, PxU32 val ) { inStats->nbBroadPhaseAdds[data] = val; }
inline PxU32 GetNbBroadPhaseAdd( const PxSimulationStatistics* inStats, PxSimulationStatistics::VolumeType data) { return inStats->nbBroadPhaseAdds[data]; }


PX_PHYSX_CORE_API NbBroadPhaseAddsProperty::NbBroadPhaseAddsProperty()
	: PxIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbBroadPhaseAdds
			, PxSimulationStatistics
			, PxSimulationStatistics::VolumeType
			, PxU32> ( "NbBroadPhaseAdds", SetNbBroadPhaseAdd, GetNbBroadPhaseAdd )
{
}

inline void SetNbBroadPhaseRemove( PxSimulationStatistics* inStats, PxSimulationStatistics::VolumeType data, PxU32 val ) { inStats->nbBroadPhaseRemoves[data] = val; }
inline PxU32 GetNbBroadPhaseRemove( const PxSimulationStatistics* inStats, PxSimulationStatistics::VolumeType data) { return inStats->nbBroadPhaseRemoves[data]; }


PX_PHYSX_CORE_API NbBroadPhaseRemovesProperty::NbBroadPhaseRemovesProperty()
	: PxIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbBroadPhaseRemoves
			, PxSimulationStatistics
			, PxSimulationStatistics::VolumeType
			, PxU32> ( "NbBroadPhaseRemoves", SetNbBroadPhaseRemove, GetNbBroadPhaseRemove )
{
}

inline void SetNbShape( PxSimulationStatistics* inStats, PxGeometryType::Enum data, PxU32 val ) { inStats->nbShapes[data] = val; }
inline PxU32 GetNbShape( const PxSimulationStatistics* inStats, PxGeometryType::Enum data) { return inStats->nbShapes[data]; }


PX_PHYSX_CORE_API NbShapesProperty::NbShapesProperty()
	: PxIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbShapes
			, PxSimulationStatistics
			, PxGeometryType::Enum
			, PxU32> ( "NbShapes", SetNbShape, GetNbShape )
{
}


inline void SetNbDiscreteContactPairs( PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2, PxU32 val ) { inStats->nbDiscreteContactPairs[idx1][idx2] = val; }
inline PxU32 GetNbDiscreteContactPairs( const PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2 ) { return inStats->nbDiscreteContactPairs[idx1][idx2]; }
PX_PHYSX_CORE_API NbDiscreteContactPairsProperty::NbDiscreteContactPairsProperty()
								: PxDualIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbDiscreteContactPairs
															, PxSimulationStatistics
															, PxGeometryType::Enum
															, PxGeometryType::Enum
															, PxU32> ( "NbDiscreteContactPairs", SetNbDiscreteContactPairs, GetNbDiscreteContactPairs )
{
}

inline void SetNbModifiedContactPairs( PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2, PxU32 val ) { inStats->nbModifiedContactPairs[idx1][idx2] = val; }
inline PxU32 GetNbModifiedContactPairs( const PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2 ) { return inStats->nbModifiedContactPairs[idx1][idx2]; }
PX_PHYSX_CORE_API NbModifiedContactPairsProperty::NbModifiedContactPairsProperty()
								: PxDualIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbModifiedContactPairs
															, PxSimulationStatistics
															, PxGeometryType::Enum
															, PxGeometryType::Enum
															, PxU32> ( "NbModifiedContactPairs", SetNbModifiedContactPairs, GetNbModifiedContactPairs )
{
}

inline void SetNbCCDPairs( PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2, PxU32 val ) { inStats->nbCCDPairs[idx1][idx2] = val; }
inline PxU32 GetNbCCDPairs( const PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2 ) { return inStats->nbCCDPairs[idx1][idx2]; }
PX_PHYSX_CORE_API NbCCDPairsProperty::NbCCDPairsProperty()
								: PxDualIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbCCDPairs
															, PxSimulationStatistics
															, PxGeometryType::Enum
															, PxGeometryType::Enum
															, PxU32> ( "NbCCDPairs", SetNbCCDPairs, GetNbCCDPairs )
{
}

inline void SetNbTriggerPairs( PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2, PxU32 val ) { inStats->nbTriggerPairs[idx1][idx2] = val; }
inline PxU32 GetNbTriggerPairs( const PxSimulationStatistics* inStats, PxGeometryType::Enum idx1, PxGeometryType::Enum idx2 ) { return inStats->nbTriggerPairs[idx1][idx2]; }
PX_PHYSX_CORE_API NbTriggerPairsProperty::NbTriggerPairsProperty()
								: PxDualIndexedPropertyInfo<PxPropertyInfoName::PxSimulationStatistics_NbTriggerPairs
															, PxSimulationStatistics
															, PxGeometryType::Enum
															, PxGeometryType::Enum
															, PxU32> ( "NbTriggerPairs", SetNbTriggerPairs, GetNbTriggerPairs )
{
}

inline PxSimulationStatistics GetStats( const PxScene* inScene ) { PxSimulationStatistics stats; inScene->getSimulationStatistics( stats ); return stats; }
PX_PHYSX_CORE_API SimulationStatisticsProperty::SimulationStatisticsProperty() 
	: PxReadOnlyPropertyInfo<PxPropertyInfoName::PxScene_SimulationStatistics, PxScene, PxSimulationStatistics >( "SimulationStatistics", GetStats )
{
}

#if PX_USE_PARTICLE_SYSTEM_API
inline void SetProjectionPlane( PxParticleBase* inBase, PxMetaDataPlane inPlane ) { inBase->setProjectionPlane( inPlane.normal, inPlane.distance ); } 
inline PxMetaDataPlane GetProjectionPlane( const PxParticleBase* inBase ) 
{ 
	PxMetaDataPlane retval;
	inBase->getProjectionPlane( retval.normal, retval.distance ); 
	return retval;
}

PX_PHYSX_CORE_API ProjectionPlaneProperty::ProjectionPlaneProperty() 
	: PxPropertyInfo< PxPropertyInfoName::PxParticleBase_ProjectionPlane, PxParticleBase, PxMetaDataPlane, PxMetaDataPlane >( "ProjectionPlane", SetProjectionPlane, GetProjectionPlane )
{
}
#endif // PX_USE_PARTICLE_SYSTEM_API

#if PX_USE_CLOTH_API
inline PxU32 GetNbPxClothFabric_Restvalues( const PxClothFabric* fabric ) { return fabric->getNbParticleIndices()/2; }
inline PxU32 GetPxClothFabric_Restvalues( const PxClothFabric* fabric, PxReal* outBuffer, PxU32 outBufLen ){ return fabric->getRestvalues( outBuffer, outBufLen ); }

PX_PHYSX_CORE_API RestvaluesProperty::RestvaluesProperty()
	: PxReadOnlyCollectionPropertyInfo<PxPropertyInfoName::PxClothFabric_Restvalues, PxClothFabric, PxReal>( "Restvalues", GetPxClothFabric_Restvalues, GetNbPxClothFabric_Restvalues )
{
}

inline PxU32 GetNbPxClothFabric_PhaseTypes( const PxClothFabric* fabric ) { return fabric->getNbPhases(); }
inline PxU32 GetPxClothFabric_PhaseTypes( const PxClothFabric* fabric, PxClothFabricPhaseType::Enum* outBuffer, PxU32 outBufLen )
{
	PxU32 numItems = PxMin( outBufLen, fabric->getNbPhases() );
	for ( PxU32 idx = 0; idx < numItems; ++idx )
		outBuffer[idx] =(static_cast<const NpClothFabric*> (fabric))->getPhaseType( idx );
	return numItems;
}
#endif // PX_USE_CLOTH_API
