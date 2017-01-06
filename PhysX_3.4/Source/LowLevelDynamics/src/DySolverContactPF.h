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



#ifndef DY_SOLVERCONTACTPF_H
#define DY_SOLVERCONTACTPF_H

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxVec3.h"
#include "PxvConfig.h"
#include "PsVecMath.h"

namespace physx
{

using namespace Ps::aos;

namespace Dy
{

struct SolverContactCoulombHeader
{
	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	numNormalConstr;
	PxU16	frictionOffset;					//4
	//PxF32	restitution;
	PxF32	angDom0;						//8
	PxF32	dominance0;						//12
	PxF32	dominance1;						//16
	PX_ALIGN(16, PxVec3	normalXYZ);			//28
	PxF32	angDom1;						//32
	
	Sc::ShapeInteraction* shapeInteraction;		//36	40
	PxU8	flags;								//37	41
	PxU8	pad0[3];							//40	44
#if !PX_P64_FAMILY
	PxU32	pad1[2];							//48	
#else
	PxU32 pad1;									//		48
#endif
	
	
	
	PX_FORCE_INLINE void setDominance0(const FloatV f)		{FStore(f, &dominance0);}
	PX_FORCE_INLINE void setDominance1(const FloatV f)		{FStore(f, &dominance1);}
	PX_FORCE_INLINE void setNormal(const Vec3V n)			{V3StoreA(n, normalXYZ);}
	
	PX_FORCE_INLINE FloatV getDominance0() const			{return FLoad(dominance0);}
	PX_FORCE_INLINE FloatV getDominance1() const			{return FLoad(dominance1);}
	//PX_FORCE_INLINE FloatV getRestitution() const			{return FLoad(restitution);}
	PX_FORCE_INLINE	Vec3V getNormal()const					{return V3LoadA(normalXYZ);}

  
	PX_FORCE_INLINE void setDominance0(PxF32 f)				{ dominance0 = f; }
	PX_FORCE_INLINE void setDominance1(PxF32 f)				{ dominance1 = f;}
	//PX_FORCE_INLINE void setRestitution(PxF32 f)			{ restitution = f;}

	PX_FORCE_INLINE PxF32 getDominance0PxF32() const		{return dominance0;}
	PX_FORCE_INLINE PxF32 getDominance1PxF32() const		{return dominance1;}
	//PX_FORCE_INLINE PxF32 getRestitutionPxF32() const		{return restitution;}

}; 
PX_COMPILE_TIME_ASSERT(sizeof(SolverContactCoulombHeader) == 48);

struct SolverFrictionHeader
{
	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	numNormalConstr;
	PxU8	numFrictionConstr;
	PxU8	flags;
	PxF32   staticFriction;
	PxF32   invMass0D0;
	PxF32	invMass1D1;
	PxF32	angDom0;
	PxF32	angDom1;
	PxU32	pad2[2];

	PX_FORCE_INLINE void setStaticFriction(const FloatV f)	{FStore(f, &staticFriction);}
	
	PX_FORCE_INLINE FloatV getStaticFriction() const		{return FLoad(staticFriction);}
	
	PX_FORCE_INLINE void setStaticFriction(PxF32 f)			{staticFriction = f;}

	PX_FORCE_INLINE PxF32 getStaticFrictionPxF32() const	{return staticFriction;}	

	PX_FORCE_INLINE PxU32 getAppliedForcePaddingSize() const {return sizeof(PxU32)*((4 * ((numNormalConstr + 3)/4)));}
	static PX_FORCE_INLINE PxU32 getAppliedForcePaddingSize(const PxU32 numConstr) {return sizeof(PxU32)*((4 * ((numConstr + 3)/4)));}
}; 

PX_COMPILE_TIME_ASSERT(sizeof(SolverFrictionHeader) == 32);

}

}

#endif //DY_SOLVERCONTACTPF_H
