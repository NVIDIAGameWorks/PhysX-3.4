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

#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include "PxVehicleMetaDataObjects.h"
#include "PxExtensionMetaDataObjects.h"

namespace physx
{
	inline void SetMFrictionVsSlipGraph( PxVehicleTireData* inTireData, PxU32 idx1, PxU32 idx2, PxReal val ) { inTireData->mFrictionVsSlipGraph[idx1][idx2] = val; }
	inline PxReal GetMFrictionVsSlipGraph( const PxVehicleTireData* inTireData, PxU32 idx1, PxU32 idx2 ) 
	{ 
		return inTireData->mFrictionVsSlipGraph[idx1][idx2]; 
	}
	PX_PHYSX_CORE_API MFrictionVsSlipGraphProperty::MFrictionVsSlipGraphProperty()
									: PxExtendedDualIndexedPropertyInfo<PxVehiclePropertyInfoName::PxVehicleTireData_MFrictionVsSlipGraph
																, PxVehicleTireData
																, PxU32
																, PxU32
																, PxReal> ( "MFrictionVsSlipGraph", SetMFrictionVsSlipGraph, GetMFrictionVsSlipGraph, 3, 2 )
	{

	}
		
	inline PxU32 GetNbWheels( const PxVehicleWheels* inStats ) { return inStats->mWheelsSimData.getNbWheels(); }

	inline PxU32 GetNbTorqueCurvePair( const PxVehicleEngineData* inStats ) { return inStats->mTorqueCurve.getNbDataPairs(); }
		
    
	inline PxReal getXTorqueCurvePair( const PxVehicleEngineData* inStats, PxU32 index)
	{ 
		return inStats->mTorqueCurve.getX(index);
	}
	inline PxReal getYTorqueCurvePair( const PxVehicleEngineData* inStats, PxU32 index)
	{ 
		return inStats->mTorqueCurve.getY(index);
	}
	
	void addTorqueCurvePair(PxVehicleEngineData* inStats, const PxReal x, const PxReal y) 
	{ 
		inStats->mTorqueCurve.addPair(x, y); 
	}
	
	void clearTorqueCurvePair(PxVehicleEngineData* inStats) 
	{ 
		inStats->mTorqueCurve.clear(); 
	}

	PX_PHYSX_CORE_API MTorqueCurveProperty::MTorqueCurveProperty()
		: PxFixedSizeLookupTablePropertyInfo<PxVehiclePropertyInfoName::PxVehicleEngineData_MTorqueCurve
				, PxVehicleEngineData
				, PxU32
				, PxReal>("MTorqueCurve", getXTorqueCurvePair, getYTorqueCurvePair, GetNbTorqueCurvePair, addTorqueCurvePair, clearTorqueCurvePair)
	{
	}


}

