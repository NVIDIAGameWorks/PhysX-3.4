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


#ifndef PX_PHYSICS_SCP_MATERIAL_CORE
#define PX_PHYSICS_SCP_MATERIAL_CORE

#include "foundation/PxVec3.h"
#include "PsUserAllocated.h"
#include "CmPhysXCommon.h"
#include "PxMaterial.h"
#include "PxsMaterialCore.h"

namespace physx
{

class PxMaterial;

namespace Sc
{

typedef	PxsMaterialData	MaterialData;

class MaterialCore : public PxsMaterialCore
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
											MaterialCore(const MaterialData& desc);
											MaterialCore(const PxEMPTY) : PxsMaterialCore(PxEmpty) {}
											MaterialCore(){}
											~MaterialCore();
	static			void					getBinaryMetaData(PxOutputStream& stream);

	PX_FORCE_INLINE	void					save(MaterialData& data)		const			{ data = *this;			}
	PX_FORCE_INLINE	void					load(const MaterialData& data)					{ static_cast<MaterialData&>(*this) = data; }	// To make synchronization between master material and scene material table less painful

};

} // namespace Sc

}

#endif
