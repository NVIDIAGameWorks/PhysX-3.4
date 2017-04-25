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

#include "PxRepXSimpleType.h"
#include "PxBase.h"
#include "PxCollection.h"
#include "PxVehicleMetaDataObjects.h"
#include "SnRepXSerializerImpl.h"
#include "PxVehicleSerialization.h"
#include "PxVehicleSuspWheelTire4.h"
#include "PxVehicleSuspLimitConstraintShader.h"
#include "PsFPU.h"


namespace physx
{
	using namespace Sn;
	
	template<typename TVehicleType>
	inline void* createVehicle( PxPhysics& physics, PxRigidDynamic* vehActor, 
									const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData, const PxVehicleDriveSimDataNW& driveDataNW,
									const PxU32 numWheels, const PxU32 numNonDrivenWheels)
	{
		PX_UNUSED(physics);
		PX_UNUSED(vehActor);
		PX_UNUSED(wheelsData);
		PX_UNUSED(driveData);
		PX_UNUSED(driveDataNW);
		PX_UNUSED(numWheels);
		PX_UNUSED(numNonDrivenWheels);
		return NULL;
	}

	template<>
	inline void* createVehicle<PxVehicleDrive4W>(PxPhysics& physics, PxRigidDynamic* vehActor,
														   const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData, const PxVehicleDriveSimDataNW& /*driveDataNW*/,
														   const PxU32 numWheels, const PxU32 numNonDrivenWheels)
	{
		PxVehicleDrive4W* vehDrive4W = PxVehicleDrive4W::allocate(numWheels);
		vehDrive4W->setup(&physics, vehActor->is<PxRigidDynamic>(), wheelsData, driveData, numNonDrivenWheels);
		return vehDrive4W;
	}
	
	template<>
	inline void* createVehicle<PxVehicleDriveTank>(PxPhysics& physics, PxRigidDynamic* vehActor,
														   const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& driveData, const PxVehicleDriveSimDataNW& /*driveDataNW*/,
														   const PxU32 numWheels, const PxU32 numNonDrivenWheels)
	{
		PxVehicleDriveTank* tank = PxVehicleDriveTank::allocate(numWheels);
		tank->setup(&physics, vehActor->is<PxRigidDynamic>(), wheelsData, driveData, numWheels - numNonDrivenWheels);
		return tank;
	}
	
	template<>
	inline void* createVehicle<PxVehicleDriveNW>(PxPhysics& physics, PxRigidDynamic* vehActor,
														   const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& /*driveData*/, const PxVehicleDriveSimDataNW& driveDataNW,
														   const PxU32 numWheels, const PxU32 numNonDrivenWheels)
	{
		PxVehicleDriveNW* vehDriveNW = PxVehicleDriveNW::allocate(numWheels);
		vehDriveNW->setup(&physics, vehActor->is<PxRigidDynamic>(), wheelsData, driveDataNW, numWheels - numNonDrivenWheels);
		return vehDriveNW;
	}
	
	template<>
	inline void* createVehicle<PxVehicleNoDrive>(PxPhysics& physics, PxRigidDynamic* vehActor,
		const PxVehicleWheelsSimData& wheelsData, const PxVehicleDriveSimData4W& /*driveData*/, const PxVehicleDriveSimDataNW& /*driveDataNW*/,
		const PxU32 numWheels, const PxU32 /*numNonDrivenWheels*/)
	{
		PxVehicleNoDrive* vehNoDrive = PxVehicleNoDrive::allocate(numWheels);
		vehNoDrive->setup(&physics, vehActor->is<PxRigidDynamic>(), wheelsData);
		return vehNoDrive;
	}

	template<typename TVehicleType>
	PxRepXObject PxVehicleRepXSerializer<TVehicleType>::fileToObject( XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection )
	{
		PxRigidActor* vehActor = NULL;
		readReference<PxRigidActor>( inReader, *inCollection, "PxRigidDynamicRef", vehActor );
		if ( vehActor == NULL )
			return PxRepXObject();

		PxU32 numWheels = 0;
		readProperty( inReader, "NumWheels", numWheels );
		if( numWheels == 0)
		{
			Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, 
				"PxSerialization::createCollectionFromXml: PxVehicleRepXSerializer: Xml field NumWheels is zero!");
			return PxRepXObject();
		}

		PxU32 numNonDrivenWheels = 0;
		readProperty( inReader, "NumNonDrivenWheels", numNonDrivenWheels );

		//change to numwheel
		PxVehicleWheelsSimData* wheelsSimData=PxVehicleWheelsSimData::allocate(numWheels);
		{
			inReader.pushCurrentContext();
			if ( inReader.gotoChild( "MWheelsSimData" ) )
			{
				readAllProperties( inArgs, inReader, wheelsSimData, inAllocator, *inCollection );
			}

			inReader.popCurrentContext();
		}

		PxVehicleDriveSimData4W driveSimData;
		{
			inReader.pushCurrentContext();
			if ( inReader.gotoChild( "MDriveSimData" ) )
			{
				readAllProperties( inArgs, inReader, &driveSimData, inAllocator, *inCollection );
			}

			inReader.popCurrentContext();
		}

		PxVehicleDriveSimDataNW nmSimData;
		{
			inReader.pushCurrentContext();
			if ( inReader.gotoChild( "MDriveSimDataNW" ) )
			{
				readAllProperties( inArgs, inReader, &driveSimData, inAllocator, *inCollection );
			}
			inReader.popCurrentContext();
		}
		TVehicleType* drive = static_cast<TVehicleType*>(createVehicle<TVehicleType>(inArgs.physics, vehActor->is<PxRigidDynamic>(), *wheelsSimData, driveSimData, nmSimData, numWheels, numNonDrivenWheels));
		readAllProperties( inArgs, inReader, drive, inAllocator, *inCollection );

		PxVehicleWheels4DynData* wheel4DynData = drive->mWheelsDynData.getWheel4DynData();
		PX_ASSERT( wheel4DynData );    
		for(PxU32 i=0;i<wheelsSimData->getNbWheels4();i++)
		{
			PxConstraint* constraint = wheel4DynData[i].getVehicletConstraintShader().getPxConstraint();
			if( constraint )
				inCollection->add(*constraint);
		}

		if( wheelsSimData )
			wheelsSimData->free();

		return PxCreateRepXObject(drive);
	}

	template<typename TVehicleType>
	void PxVehicleRepXSerializer<TVehicleType>::objectToFileImpl( const TVehicleType* drive, PxCollection* inCollection, XmlWriter& inWriter, MemoryBuffer& inTempBuffer, PxRepXInstantiationArgs& /*inArgs*/ )
	{
		PX_SIMD_GUARD; // denorm exception triggered in PxVehicleGearsDataGeneratedInfo::visitInstanceProperties on osx
		writeReference( inWriter, *inCollection, "PxRigidDynamicRef", drive->getRigidDynamicActor() );
		writeProperty( inWriter, *inCollection, inTempBuffer, "NumWheels", drive->mWheelsSimData.getNbWheels() );
		writeProperty( inWriter, *inCollection, inTempBuffer, "NumNonDrivenWheels", drive->getNbNonDrivenWheels());
		writeAllProperties( drive, inWriter, inTempBuffer, *inCollection );
	}

	PxVehicleNoDrive::PxVehicleNoDrive()
	: PxVehicleWheels(PxVehicleConcreteType::eVehicleNoDrive, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
	{}	

	PxVehicleDrive4W::PxVehicleDrive4W()
	: PxVehicleDrive(PxVehicleConcreteType::eVehicleDrive4W, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
	{}	

	PxVehicleDriveNW::PxVehicleDriveNW()
	: PxVehicleDrive(PxVehicleConcreteType::eVehicleDriveNW, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
	{}

	PxVehicleDriveTank::PxVehicleDriveTank() 
	: PxVehicleDrive(PxVehicleConcreteType::eVehicleDriveTank, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE)
	, mDriveModel(PxVehicleDriveTankControlModel::eSTANDARD) 
	{}

	// explicit template instantiations
	template struct PxVehicleRepXSerializer<PxVehicleDrive4W>;
	template struct PxVehicleRepXSerializer<PxVehicleDriveTank>;
	template struct PxVehicleRepXSerializer<PxVehicleDriveNW>;
	template struct PxVehicleRepXSerializer<PxVehicleNoDrive>;

} 
