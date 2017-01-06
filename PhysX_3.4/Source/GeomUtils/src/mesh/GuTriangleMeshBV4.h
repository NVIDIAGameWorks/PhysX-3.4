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

#ifndef GU_TRIANGLEMESH_BV4_H
#define GU_TRIANGLEMESH_BV4_H

#include "GuTriangleMesh.h"

namespace physx
{
class GuMeshFactory;

namespace Gu
{

#if PX_VC
#pragma warning(push)
#pragma warning(disable: 4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#endif

class BV4TriangleMesh : public TriangleMesh
{
	public:
						virtual const char*				getConcreteTypeName()	const	{ return "PxBVH34TriangleMesh"; }
// PX_SERIALIZATION
														BV4TriangleMesh(PxBaseFlags baseFlags) : TriangleMesh(baseFlags), mMeshInterface(PxEmpty), mBV4Tree(PxEmpty)	{}
	PX_PHYSX_COMMON_API	virtual void					exportExtraData(PxSerializationContext& ctx);
								void					importExtraData(PxDeserializationContext&);
	PX_PHYSX_COMMON_API	static	TriangleMesh*			createObject(PxU8*& address, PxDeserializationContext& context);
	PX_PHYSX_COMMON_API	static	void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
														BV4TriangleMesh(GuMeshFactory& factory, TriangleMeshData& data);
						virtual							~BV4TriangleMesh(){}

						virtual	PxMeshMidPhase::Enum	getMidphaseID()			const	{ return PxMeshMidPhase::eBVH34;	}
	PX_FORCE_INLINE				const Gu::BV4Tree&		getBV4Tree()			const	{ return mBV4Tree;				}
	private:
								Gu::SourceMesh			mMeshInterface;
								Gu::BV4Tree				mBV4Tree;
};

#if PX_VC
#pragma warning(pop)
#endif

} // namespace Gu

}

#endif
