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



#ifndef DY_SOLVERCONTACT_H
#define DY_SOLVERCONTACT_H

#include "foundation/PxSimpleTypes.h"
#include "foundation/PxVec3.h"
#include "PxvConfig.h"
#include "PsVecMath.h"

namespace physx
{

using namespace Ps::aos;

namespace Sc
{
	class ShapeInteraction;
}
/**
\brief A header to represent a friction patch for the solver.
*/

namespace Dy
{

struct SolverContactHeader
{
	enum DySolverContactFlags
	{
		eHAS_FORCE_THRESHOLDS = 0x1
	};

	PxU8	type;					//Note: mType should be first as the solver expects a type in the first byte.
	PxU8	flags;	
	PxU8	numNormalConstr;
	PxU8	numFrictionConstr;					//4

	PxReal	angDom0;							//8
	PxReal	angDom1;							//12
	PxReal	invMass0;							//16

	Vec4V   staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W;		//32
	Vec3V	normal;															//48

	PxReal	invMass1;														//52
	PxU32 broken;															//56
	PxU8* frictionBrokenWritebackByte;										//60	64
	Sc::ShapeInteraction* shapeInteraction;									//64	72
#if PX_P64_FAMILY
	PxU32	pad[2];															//64	80
#endif // PX_X64


	PX_FORCE_INLINE void setStaticFriction(const FloatV f)	{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}
	PX_FORCE_INLINE void setDynamicFriction(const FloatV f)	{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}
	PX_FORCE_INLINE void setDominance0(const FloatV f)		{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}
	PX_FORCE_INLINE void setDominance1(const FloatV f)		{staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W=V4SetW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W,f);}

	PX_FORCE_INLINE FloatV getStaticFriction() const		{return V4GetX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE FloatV getDynamicFriction() const		{return V4GetY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE FloatV getDominance0() const			{return V4GetZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE FloatV getDominance1() const			{return V4GetW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}

	PX_FORCE_INLINE void setStaticFriction(PxF32 f)			{V4WriteX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}
	PX_FORCE_INLINE void setDynamicFriction(PxF32 f)		{V4WriteY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}
	PX_FORCE_INLINE void setDominance0(PxF32 f)				{V4WriteZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}
	PX_FORCE_INLINE void setDominance1(PxF32 f)				{V4WriteW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W, f);}

	PX_FORCE_INLINE PxF32 getStaticFrictionPxF32() const	{return V4ReadX(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE PxF32 getDynamicFrictionPxF32() const	{return V4ReadY(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE PxF32 getDominance0PxF32() const		{return V4ReadZ(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
	PX_FORCE_INLINE PxF32 getDominance1PxF32() const		{return V4ReadW(staticFrictionX_dynamicFrictionY_dominance0Z_dominance1W);}
}; 

#if !PX_P64_FAMILY
PX_COMPILE_TIME_ASSERT(sizeof(SolverContactHeader) == 64);
#else
PX_COMPILE_TIME_ASSERT(sizeof(SolverContactHeader) == 80);
#endif

/**
\brief A single rigid body contact point for the solver.
*/
struct SolverContactPoint
{
	Vec3V raXn;
	Vec3V rbXn;

	PxF32 velMultiplier;
	PxF32 biasedErr;
	PxF32 unbiasedErr;
	PxF32 maxImpulse;

	PX_FORCE_INLINE FloatV getVelMultiplier() const			{return FLoad(velMultiplier);}

	PX_FORCE_INLINE FloatV getBiasedErr() const				{return FLoad(biasedErr);}
	PX_FORCE_INLINE FloatV getMaxImpulse() const			{return FLoad(maxImpulse);}


	PX_FORCE_INLINE Vec3V getRaXn() const					{return raXn;}
	PX_FORCE_INLINE Vec3V getRbXn() const					{return rbXn;}

	PX_FORCE_INLINE void setRaXn(const PxVec3& v)			{V3WriteXYZ(raXn, v);}
	PX_FORCE_INLINE void setRbXn(const PxVec3& v)			{V3WriteXYZ(rbXn, v);}
	PX_FORCE_INLINE void setVelMultiplier(PxF32 f)			{velMultiplier = f;}

	PX_FORCE_INLINE void setBiasedErr(PxF32 f)				{biasedErr = f;}
	PX_FORCE_INLINE void setUnbiasedErr(PxF32 f)			{unbiasedErr = f;}

	PX_FORCE_INLINE PxF32 getVelMultiplierPxF32() const		{return velMultiplier;}
	PX_FORCE_INLINE const PxVec3& getRaXnPxVec3() const		{return V3ReadXYZ(raXn);}
	PX_FORCE_INLINE const PxVec3& getRbXnPxVec3() const		{return V3ReadXYZ(rbXn);}
	PX_FORCE_INLINE PxF32 getBiasedErrPxF32() const			{return biasedErr;}
}; 


PX_COMPILE_TIME_ASSERT(sizeof(SolverContactPoint) == 48);

/**
\brief A single extended articulation contact point for the solver.
*/
struct SolverContactPointExt : public SolverContactPoint
{
	Vec3V linDeltaVA;
	Vec3V angDeltaVA;
	Vec3V linDeltaVB;
	Vec3V angDeltaVB;
};

PX_COMPILE_TIME_ASSERT(sizeof(SolverContactPointExt) == 112);


/**
\brief A single friction constraint for the solver.
*/
struct SolverContactFriction
{
	Vec4V normalXYZ_appliedForceW;		//16
	Vec4V raXnXYZ_velMultiplierW;		//32
	Vec4V rbXnXYZ_biasW;				//48
	PxReal targetVel;					//52
	PxU32 mPad[3];						//64

	PX_FORCE_INLINE void setAppliedForce(const FloatV f)	{normalXYZ_appliedForceW=V4SetW(normalXYZ_appliedForceW,f);}
	PX_FORCE_INLINE void setVelMultiplier(const FloatV f)	{raXnXYZ_velMultiplierW=V4SetW(raXnXYZ_velMultiplierW,f);}
	PX_FORCE_INLINE void setBias(const FloatV f)			{rbXnXYZ_biasW=V4SetW(rbXnXYZ_biasW,f);}

	PX_FORCE_INLINE FloatV getAppliedForce() const			{return V4GetW(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE FloatV getVelMultiplier() const			{return V4GetW(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE FloatV getBias() const					{return V4GetW(rbXnXYZ_biasW);}

	PX_FORCE_INLINE Vec3V getNormal() const					{return Vec3V_From_Vec4V(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE Vec3V getRaXn() const					{return Vec3V_From_Vec4V(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE Vec3V getRbXn() const					{return Vec3V_From_Vec4V(rbXnXYZ_biasW);}

	PX_FORCE_INLINE void setNormal(const PxVec3& v)			{V4WriteXYZ(normalXYZ_appliedForceW, v);}
	PX_FORCE_INLINE void setRaXn(const PxVec3& v)			{V4WriteXYZ(raXnXYZ_velMultiplierW, v);}
	PX_FORCE_INLINE void setRbXn(const PxVec3& v)			{V4WriteXYZ(rbXnXYZ_biasW, v);}

	PX_FORCE_INLINE const PxVec3& getNormalPxVec3() const	{return V4ReadXYZ(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE const PxVec3& getRaXnPxVec3() const		{return V4ReadXYZ(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE const PxVec3& getRbXnPxVec3() const		{return V4ReadXYZ(rbXnXYZ_biasW);}

	PX_FORCE_INLINE void setAppliedForce(PxF32 f)			{V4WriteW(normalXYZ_appliedForceW, f);}
	PX_FORCE_INLINE void setVelMultiplier(PxF32 f)			{V4WriteW(raXnXYZ_velMultiplierW, f);}
	PX_FORCE_INLINE void setBias(PxF32 f)					{V4WriteW(rbXnXYZ_biasW, f);}
	
	PX_FORCE_INLINE PxF32 getAppliedForcePxF32() const		{return V4ReadW(normalXYZ_appliedForceW);}
	PX_FORCE_INLINE PxF32 getVelMultiplierPxF32() const		{return V4ReadW(raXnXYZ_velMultiplierW);}
	PX_FORCE_INLINE PxF32 getBiasPxF32() const				{return V4ReadW(rbXnXYZ_biasW);}

}; 

PX_COMPILE_TIME_ASSERT(sizeof(SolverContactFriction) == 64);

/**
\brief A single extended articulation friction constraint for the solver.
*/
struct SolverContactFrictionExt : public SolverContactFriction
{
	Vec3V linDeltaVA;
	Vec3V angDeltaVA;
	Vec3V linDeltaVB;
	Vec3V angDeltaVB;
};
PX_COMPILE_TIME_ASSERT(sizeof(SolverContactFrictionExt) == 128);

}

}



#endif //DY_SOLVERCONTACT_H
