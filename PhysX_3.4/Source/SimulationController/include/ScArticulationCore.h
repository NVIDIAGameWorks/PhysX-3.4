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


#ifndef PX_PHYSICS_SCP_ARTICULATION_CORE
#define PX_PHYSICS_SCP_ARTICULATION_CORE

#include "ScActorCore.h"
#include "DyArticulation.h"

namespace physx
{

class PxvArticulation;

namespace IG
{
	class NodeIndex;
}


namespace Sc
{
	typedef Dy::FsData ArticulationDriveCache;

	class ArticulationSim;
	class BodyCore;

	class ArticulationCore
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		//---------------------------------------------------------------------------------
		// Construction, destruction & initialization
		//---------------------------------------------------------------------------------

// PX_SERIALIZATION
		public:
													ArticulationCore(const PxEMPTY) : mSim(NULL)	{}
		static		void							getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
													ArticulationCore();
													~ArticulationCore();

		//---------------------------------------------------------------------------------
		// External API
		//---------------------------------------------------------------------------------
						PxU32						getInternalDriveIterations() const; 
						void						setInternalDriveIterations(const PxU32 v);

						PxU32						getExternalDriveIterations() const; 
						void						setExternalDriveIterations(const PxU32 v);

						PxU32						getMaxProjectionIterations() const; 
						void						setMaxProjectionIterations(const PxU32 v);

						PxReal						getSeparationTolerance() const; 
						void						setSeparationTolerance(const PxReal v);

						PxReal						getSleepThreshold() const; 
						void						setSleepThreshold(const PxReal v);

						PxReal						getFreezeThreshold() const; 
						void						setFreezeThreshold(const PxReal v);

						PxReal						getWakeCounter() const; 
						void						setWakeCounter(const PxReal v);
						void						setWakeCounterInternal(const PxReal v);

						bool						isSleeping() const;
						void						wakeUp(PxReal wakeCounter);
						void						putToSleep();

						PxU16						getSolverIterationCounts()	const;
						void						setSolverIterationCounts(PxU16 c);

						PxArticulation*				getPxArticulation();
						const PxArticulation*		getPxArticulation() const;


		//---------------------------------------------------------------------------------
		// Drive Cache API
		//---------------------------------------------------------------------------------
						ArticulationDriveCache*		createDriveCache(PxReal compliance,
																	 PxU32 driveIterations) const;

						void						updateDriveCache(ArticulationDriveCache& cache,
																	 PxReal compliance,
																	 PxU32 driveIterations) const;

						void						releaseDriveCache(ArticulationDriveCache& cache) const;

						PxU32						getCacheLinkCount(const ArticulationDriveCache& cache) const;

						void						applyImpulse(BodyCore& link,
																 const ArticulationDriveCache& driveCache,
																 const PxVec3& force,
																 const PxVec3& torque);

						void						computeImpulseResponse(BodyCore& link,
																		   PxVec3& linearResponse, 
																		   PxVec3& angularResponse,
																		   const ArticulationDriveCache& driveCache,
																		   const PxVec3& force,
																		   const PxVec3& torque) const;

		//---------------------------------------------------------------------------------
		// Internal API
		//---------------------------------------------------------------------------------
	public:
		PX_FORCE_INLINE	void						setSim(ArticulationSim* sim)
													{
														PX_ASSERT((sim==0) ^ (mSim == 0));
														mSim = sim;
													}
		PX_FORCE_INLINE	ArticulationSim*			getSim()			const	{ return mSim;			}

		PX_FORCE_INLINE	const Dy::ArticulationCore&	getCore()					{ return mCore;			}

		static PX_FORCE_INLINE ArticulationCore&	getArticulationCore(ArticulationCore& core)			
		{ 
			size_t offset = PX_OFFSET_OF(ArticulationCore, mCore);
			return *reinterpret_cast<ArticulationCore*>(reinterpret_cast<PxU8*>(&core) - offset); 
		}	

		IG::NodeIndex getIslandNodeIndex() const;

	private:
						ArticulationSim*			mSim;
						Dy::ArticulationCore			mCore;
	};



} // namespace Sc

}

#endif
