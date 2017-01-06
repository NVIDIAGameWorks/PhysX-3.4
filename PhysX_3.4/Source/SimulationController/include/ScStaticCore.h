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


#ifndef PX_PHYSICS_SCP_STATIC_CORE
#define PX_PHYSICS_SCP_STATIC_CORE

#include "ScRigidCore.h"
#include "PxvDynamics.h"

namespace physx
{
namespace Sc
{
	
	class StaticSim;

	class StaticCore: public RigidCore
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
											StaticCore(const PxTransform& actor2World): RigidCore(PxActorType::eRIGID_STATIC)	
											{
												mCore.body2World = actor2World;
												mCore.mFlags = PxRigidBodyFlags();
											}
							
		PX_FORCE_INLINE	const PxTransform&	getActor2World() const	{ return mCore.body2World;	}
						void				setActor2World(const PxTransform& actor2World);

		PX_FORCE_INLINE	PxsRigidCore&		getCore()				{ return mCore;				}

											StaticCore(const PxEMPTY) :	RigidCore(PxEmpty), mCore(PxEmpty) {}
		static			void				getBinaryMetaData(PxOutputStream& stream);

						StaticSim*			getSim() const;

		PX_FORCE_INLINE	void				onOriginShift(const PxVec3& shift)	{ mCore.body2World.p -= shift; }
	private:
						PxsRigidCore		mCore;
	};

} // namespace Sc

}

#endif
