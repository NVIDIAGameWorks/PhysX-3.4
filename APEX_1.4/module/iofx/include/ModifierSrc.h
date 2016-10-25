/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#if defined(__CUDACC__) && defined(__CUDA_ARCH__) && __CUDA_ARCH__ < 200
#define _CUDA_OPT_LOC_MEM_ 1
#else
#define _CUDA_OPT_LOC_MEM_ 0
#endif

#define PI 3.141592653589793f

//--- Helpers

APEX_CUDA_CALLABLE PX_INLINE unsigned int binSearch(float val, unsigned int count, const float* data)
{
	unsigned int beg = 0;
	unsigned int end = count;

	while (beg < end)
	{
		unsigned int mid = beg + ((end - beg) >> 1);
		if (val < data[mid])
		{
			end = mid;
		}
		else
		{
			beg = mid + 1;
		}
	}
	return beg;
}

APEX_CUDA_CALLABLE PX_INLINE void approxAxisAngleToMat33(const PxVec3& axisAngle, PxMat33& rot)
{
	const float x = 0.5f * axisAngle.x;
	const float y = 0.5f * axisAngle.y;
	const float z = 0.5f * axisAngle.z;
	const float xx = x * x;
	const float yy = y * y;
	const float zz = z * z;
	const float xy = x * y;
	const float yz = y * z;
	const float zx = z * x;
	const float twoRecipNorm2 = 2.0f / (1.0f + xx + yy + zz);	// w = 1
	rot(0, 0) = 1.0f - twoRecipNorm2 * (yy + zz);
	rot(0, 1) = twoRecipNorm2 * (xy - z);
	rot(0, 2) = twoRecipNorm2 * (zx + y);
	rot(1, 0) = twoRecipNorm2 * (xy + z);
	rot(1, 1) = 1.0f - twoRecipNorm2 * (zz + xx);
	rot(1, 2) = twoRecipNorm2 * (yz - x);
	rot(2, 0) = twoRecipNorm2 * (zx - y);
	rot(2, 1) = twoRecipNorm2 * (yz + x);
	rot(2, 2) = 1.0f - twoRecipNorm2 * (xx + yy);
}


APEX_CUDA_CALLABLE PX_INLINE bool approxEquals(float a, float b, float eps)
{
	const float diff = PxAbs(a - b);
	return (diff < eps);
}

APEX_CUDA_CALLABLE PX_INLINE bool approxEquals(const PxVec3& a, const PxVec3& b, float eps)
{
	return	approxEquals(a.x, b.x, eps) &&
	        approxEquals(a.y, b.y, eps) &&
	        approxEquals(a.z, b.z, eps);
}

APEX_CUDA_CALLABLE PX_INLINE int maxAbsElementIndex(const PxVec3& v)
{
	const PxVec3 a(PxAbs(v.x), PxAbs(v.y), PxAbs(v.z));
	const int m01 = (a.y > a.x);
	const int m2 = (a.z > a[(unsigned)m01]);
	return (m2 << 1) | (m01 >> m2);
}


APEX_CUDA_CALLABLE PX_INLINE void generateRotationMatrix(const PxVec3& srcVec, const PxVec3& dstVec, PxMat33& outRotMat)
{
	if (approxEquals(srcVec, dstVec, 0.0001f))
	{
		outRotMat = PxMat33(PxIdentity);
		return;
	}

	PxVec3 crossResult = srcVec.cross(dstVec);
	crossResult *= PxAcos(srcVec.dot(dstVec));

	approxAxisAngleToMat33(crossResult, outRotMat);
}

APEX_CUDA_CALLABLE PX_INLINE void generateRandomRotation(const PxVec3& srcVec, PxMat33& outRotMat, nvidia::RandState& randState)
{
	PxVec3 tmpRotVec(srcVec);
	tmpRotVec *= randState.nextFloat(0.0f, 2 * PI);
	approxAxisAngleToMat33(tmpRotVec, outRotMat);
}

// ------------------------------------------------------------------------------------------------


//--- Rotation modifier ---

//struct PARAMS_NAME(Rotation)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(Rotation)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	rollType) \
	INPLACE_TYPE_FIELD(int32_t,	rollAxis) \
	INPLACE_TYPE_FIELD(float,	rollSign) \
	INPLACE_TYPE_FIELD(float,	maxSettleRatePerSec) \
	INPLACE_TYPE_FIELD(float,	maxRotationRatePerSec) \
	INPLACE_TYPE_FIELD(float,	inAirRotationMultiplier) \
	INPLACE_TYPE_FIELD(float,	collisionRotationMultiplier) \
	INPLACE_TYPE_FIELD(uint32_t,	includeVerticalDirection)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 3;
};

APEX_CUDA_CALLABLE PX_INLINE void chooseUp(PxVec3& outUp, unsigned int rollType, nvidia::RandState& randState)
{
	const float angle = randState.nextFloat(0.0f, 2 * PI);
	const float up = angle < PI ? -1.0f : 1.0f;
	const float cosAng = PxCos(angle);
	const float sinAng = PxSin(angle);

	switch (rollType)
	{
	default:
		outUp = PxVec3(0.0f, 0.0f, 1.0f);
		break;

	case nvidia::apex::ApexMeshParticleRollType::FLAT_X:
		outUp = PxVec3(up, 0.0f, 0.0f);
		break;
	case nvidia::apex::ApexMeshParticleRollType::FLAT_Y:
		outUp = PxVec3(0.0f, up, 0.0f);
		break;
	case nvidia::apex::ApexMeshParticleRollType::FLAT_Z:
		outUp = PxVec3(0.0f, 0.0f, up);
		break;

	case nvidia::apex::ApexMeshParticleRollType::LONG_X:
		outUp = PxVec3(0.0f, cosAng, sinAng);
		break;
	case nvidia::apex::ApexMeshParticleRollType::LONG_Y:
		outUp = PxVec3(cosAng, 0.0f, sinAng);
		break;
	case nvidia::apex::ApexMeshParticleRollType::LONG_Z:
		outUp = PxVec3(cosAng, sinAng, 0.0f);
		break;
	}
}

APEX_CUDA_CALLABLE PX_INLINE void updateParticleRollBoxFromCollision(const PARAMS_NAME(Rotation)& params, PxVec3& particleAngularDelta, const PxMat33& rot, const PxVec3& collisionNormal, float timeSlice)
{
	const float maxSettle = params.maxSettleRatePerSec * timeSlice;
	const float maxSettle2 = maxSettle * maxSettle; // where to compute this?

	if (params.rollType != nvidia::apex::ApexMeshParticleRollType::SPHERICAL)
	{
		// Settling
		PxVec3 a;
		if (params.rollAxis < 0)
		{
			// Cubic rolling, must choose most normal-pointing mRollAxis
			PxVec3 overlap;
			overlap = rot.transform(collisionNormal);
			const int32_t bestAxis = maxAbsElementIndex(overlap);
			a = bestAxis == 0 ? rot.column0 : bestAxis == 1 ? rot.column1 : rot.column2;
		}
		else
		{
			// Flat or long box, mRollAxis is chosen
			a = params.rollAxis == 0 ? rot.column0 : params.rollAxis == 1 ? rot.column1 : rot.column2;
		}

		PxVec3 settle = (params.rollSign * a.dot(collisionNormal)) * a.cross(collisionNormal);
		if (maxSettle != 0.0f)
		{
			const float settle2 = settle.magnitudeSquared();
			// Cap how much settling is allowed per simulation tick
			if (settle2 > maxSettle2)
			{
				settle *= maxSettle * PxRecipSqrt(settle2);
			}
		}
		particleAngularDelta += settle;
	}
}

APEX_CUDA_CALLABLE PX_INLINE void updateParticleRollBoxNoCollision(const PARAMS_NAME(Rotation)& params, PxVec3& particleAngularDelta, float timeSlice)
{
	const float maxRotation = params.maxRotationRatePerSec * timeSlice;
	const float maxRotation2 = maxRotation * maxRotation;

	if (maxRotation != 0.0f)
	{
		// Cap the total amount of roll.
		const float roll2 = particleAngularDelta.magnitudeSquared();
		if (roll2 > maxRotation2)
		{
			particleAngularDelta *= maxRotation * PxRecipSqrt(roll2);
		}
	}
}


INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierRotation(const PARAMS_NAME(Rotation)& params, const Input& input, PubState& pubState, PrivState& privState, const nvidia::iofx::ModifierCommonParams& common, nvidia::RandState& randState)
{
	if (usage == nvidia::apex::ModifierUsage_Mesh)
	{
		// TODO: This should really go into a 'settle' modifier, that says that objects should try to settle with one side up
		// or another, but for now it's here.
		if (spawn)
		{
			// Need to come up with a default pose.
			PxMat33 rotationOnSpawn;
			if (params.rollType == nvidia::apex::ApexMeshParticleRollType::SPHERICAL)
			{
				PxVec3 randomRotation;
				randomRotation.x = randState.nextFloat(-PI, PI);
				randomRotation.y = randState.nextFloat(-PI, PI);
				randomRotation.z = randState.nextFloat(-PI, PI);
				approxAxisAngleToMat33(randomRotation, rotationOnSpawn);
			}
			else
			{
				PxVec3 upVector;
				chooseUp(upVector, params.rollType, randState);

				PxVec3 unitSystemUp = common.upVector;
				unitSystemUp.normalize();

				PxMat33 rotateToUp, randomRotation;
				generateRotationMatrix(upVector, unitSystemUp, rotateToUp);
				generateRandomRotation(unitSystemUp, randomRotation, randState);

				rotationOnSpawn = randomRotation * rotateToUp;
			}
			privState.rotation = rotationOnSpawn;
			pubState.rotation = rotationOnSpawn;
		}
		else
		{
			PxVec3 rollDelta;
			PxVec3 delta = input.velocity * common.deltaTime;

			if (common.inputHasCollision)
			{
				if (input.collisionFlags != 0)
				{
					rollDelta = params.collisionRotationMultiplier * input.collisionNormal.cross(delta);
				}
				else
				{
					if (params.includeVerticalDirection)
					{
						// I'd like to use the absolute value of the "up" speed... just check it in first.
						rollDelta = params.inAirRotationMultiplier * delta;
					}
					else
					{
						rollDelta = params.inAirRotationMultiplier * common.upVector.cross( delta );
					}
				}

				updateParticleRollBoxFromCollision(params, rollDelta, privState.rotation, input.collisionNormal, common.deltaTime);
				updateParticleRollBoxNoCollision(params, rollDelta, common.deltaTime);
			}
			else
			{
				if (params.includeVerticalDirection)
				{
					// I'd like to use the absolute value of the "up" speed... just check it in first.
					rollDelta = params.inAirRotationMultiplier * delta;
				}
				else
				{
					rollDelta = params.inAirRotationMultiplier * common.upVector.cross( delta );
				}
				updateParticleRollBoxNoCollision(params, rollDelta, common.deltaTime);
			}

			if (rollDelta.magnitudeSquared() > 0.0f)
			{
				// update particle transform
				// A) This is because maxAngle isn't really working properly
				// B) This should be a configurable parameter of the system
				PxMat33 rot;
				approxAxisAngleToMat33(rollDelta, rot);
				privState.rotation = rot * privState.rotation;
			}
			pubState.rotation = privState.rotation;
		}
	}
}

//--- SimpleScale modifier ---

//struct PARAMS_NAME(SimpleScale)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(SimpleScale)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxVec3,	scaleFactor)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierSimpleScale(const PARAMS_NAME(SimpleScale)& params, const Input& /*input*/, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
	pubState.scale.x *= params.scaleFactor.x;
	pubState.scale.y *= params.scaleFactor.y;
	pubState.scale.z *= params.scaleFactor.z;
}

//--- RandomScale modifier ---

//struct PARAMS_NAME(RandomScale)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(RandomScale)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	scaleFactorMin) \
	INPLACE_TYPE_FIELD(float,	scaleFactorMax)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 1;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierRandomScale(const PARAMS_NAME(RandomScale)& params, const Input& /*input*/, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& randState)
{
	const float scaleFactor = randState.nextFloat(params.scaleFactorMin, params.scaleFactorMax);

	pubState.scale *= scaleFactor;
}

//--- ScaleByMass modifier ---

//struct PARAMS_NAME(ScaleByMass)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ScaleByMass)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	scaleFactor)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierScaleByMass(const PARAMS_NAME(ScaleByMass)& /*params*/, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
	pubState.scale *= input.mass;
}

//--- ColorVsLife modifier ---

//struct PARAMS_NAME(ColorVsLife)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ColorVsLife)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	channel) \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierColorVsLife(const PARAMS_NAME(ColorVsLife)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
#define _MODIFIER_CODE_(channel) \
	{ \
		pubState.color[channel] *= EVAL_CURVE(params.curve, input.liferemain); \
	}
#if _CUDA_OPT_LOC_MEM_
	if (params.channel == 0) _MODIFIER_CODE_(0)
		else if (params.channel == 1) _MODIFIER_CODE_(1)
			else if (params.channel == 2) _MODIFIER_CODE_(2)
				else if (params.channel == 3) _MODIFIER_CODE_(3)
#else
	_MODIFIER_CODE_(params.channel)
#endif
#undef _MODIFIER_CODE_

				}

//--- ColorVsDensity modifier ---

//struct PARAMS_NAME(ColorVsDensity)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ColorVsDensity)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	channel) \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierColorVsDensity(const PARAMS_NAME(ColorVsDensity)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
#define _MODIFIER_CODE_(channel) \
	{ \
		pubState.color[channel] *= EVAL_CURVE(params.curve, input.density); \
	}
#if _CUDA_OPT_LOC_MEM_
	if (params.channel == 0) _MODIFIER_CODE_(0)
		else if (params.channel == 1) _MODIFIER_CODE_(1)
			else if (params.channel == 2) _MODIFIER_CODE_(2)
				else if (params.channel == 3) _MODIFIER_CODE_(3)
#else
	_MODIFIER_CODE_(params.channel)
#endif
#undef _MODIFIER_CODE_
				}

//--- ColorVsVelocity modifier ---

//struct PARAMS_NAME(ColorVsVelocity)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ColorVsVelocity)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	velocity0) \
	INPLACE_TYPE_FIELD(float,	velocity1) \
	INPLACE_TYPE_FIELD(uint32_t,	channel) \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierColorVsVelocity(const PARAMS_NAME(ColorVsVelocity)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
	float velocity = (input.velocity.magnitude() - params.velocity0) / (params.velocity1 - params.velocity0);
	velocity = PxClamp(velocity, 0.0f, 1.0f);

#define _MODIFIER_CODE_(channel) \
	{ \
		pubState.color[channel] *= EVAL_CURVE(params.curve, velocity); \
	}
#if _CUDA_OPT_LOC_MEM_
	if (params.channel == 0) _MODIFIER_CODE_(0)
		else if (params.channel == 1) _MODIFIER_CODE_(1)
			else if (params.channel == 2) _MODIFIER_CODE_(2)
				else if (params.channel == 3) _MODIFIER_CODE_(3)
#else
	_MODIFIER_CODE_(params.channel)
#endif
#undef _MODIFIER_CODE_
}

//--- SubtextureVsLife modifier ---

//struct PARAMS_NAME(SubtextureVsLife)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(SubtextureVsLife)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierSubtextureVsLife(const PARAMS_NAME(SubtextureVsLife)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
	if (usage == nvidia::apex::ModifierUsage_Sprite)
	{
		pubState.subTextureId += EVAL_CURVE(params.curve, input.liferemain);
	}
}

//--- OrientAlongVelocity modifier ---

APEX_CUDA_CALLABLE PX_INLINE bool buildRotationMatrix(const PxVec3& srcVec, const PxVec3& dstVec, PxMat33& outRotMat)
{
	PxVec3 axis = srcVec.cross(dstVec);

	float cosAngle = srcVec.dot(dstVec);
	float angle = PxAcos(cosAngle);

	float axisLen = axis.normalize();
	if (axisLen < 0.0001f)
	{
		return false;
	}

	PxQuat quat(angle, axis);
	outRotMat = PxMat33(quat);
	return true;
}

//struct PARAMS_NAME(OrientAlongVelocity)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(OrientAlongVelocity)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(PxVec3,	modelForward)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierOrientAlongVelocity(const PARAMS_NAME(OrientAlongVelocity)& params, const Input& input, PubState& pubState, PrivState& privState, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
	if (usage == nvidia::apex::ModifierUsage_Mesh)
	{
		PxVec3 vel = input.velocity;
		float velMagnitude = vel.normalize(); // normalize it
		// If travelling too slowly, this will be unstable, so don't do anything.
		if (PxAbs(velMagnitude) >= 0.0001f)
		{
			PxMat33 rotMat;
			if (buildRotationMatrix(params.modelForward, vel, rotMat))
			{
				privState.rotation = rotMat;
			}
		}
		pubState.rotation = privState.rotation;
	}
}

//--- ScaleAlongVelocity modifier ---

///p,q -> p^q = n (n - must be normalized!)
APEX_CUDA_CALLABLE PX_INLINE void buildPlaneBasis(const PxVec3& n, PxVec3& p, PxVec3& q)
{
	const float SQRT1_2 = PxSqrt(float(0.5));
	if (PxAbs(n.z) > SQRT1_2)
	{
		// choose p in y-z plane
		float k = sqrtf(n.y * n.y + n.z * n.z);
		p.x = 0;
		p.y = -n.z / k;
		p.z = n.y / k;
		// set q = n x p
		q.x = k;
		q.y = -n.x * p.z;
		q.z = n.x * p.y;
	}
	else
	{
		// choose p in x-y plane
		float k = PxSqrt(n.x * n.x + n.y * n.y);
		p.x = -n.y / k;
		p.y = n.x / k;
		p.z = 0;
		// set q = n x p
		q.x = -n.z * p.y;
		q.y = n.z * p.x;
		q.z = k;
	}
}

APEX_CUDA_CALLABLE PX_INLINE void buildScaleAlongAxis(const PxVec3& scaleAxis, float scale, PxMat33& scaleMat)
{
	PxVec3 axis0, axis1;
	buildPlaneBasis(scaleAxis, axis0, axis1);

	PxMat33 rotToAxisMat(axis0, axis1, scaleAxis);

	scaleMat = rotToAxisMat;
	scaleMat.column2 = scaleAxis * scale;

	//scaleMat.setMultiplyTransposeRight(scaleMat, rotToAxisMat);

	float a = scaleMat.column0.dot(rotToAxisMat.column0);
	float b = scaleMat(0,0) * rotToAxisMat(0,1) + scaleMat(1,0) * rotToAxisMat(1,1) + scaleMat(2,0) * rotToAxisMat(2,1);
	float c = scaleMat(0,0) * rotToAxisMat(0,2) + scaleMat(1,0) * rotToAxisMat(1,2) + scaleMat(2,0) * rotToAxisMat(2,2);
	float d = scaleMat(0,1) * rotToAxisMat(0,0) + scaleMat(1,1) * rotToAxisMat(1,0) + scaleMat(2,1) * rotToAxisMat(2,0);
	float e = scaleMat(0,1) * rotToAxisMat(0,1) + scaleMat(1,1) * rotToAxisMat(1,1) + scaleMat(2,1) * rotToAxisMat(2,1);
	float f = scaleMat(0,1) * rotToAxisMat(0,2) + scaleMat(1,1) * rotToAxisMat(1,2) + scaleMat(2,1) * rotToAxisMat(2,2);
	float g = scaleMat(0,2) * rotToAxisMat(0,0) + scaleMat(1,2) * rotToAxisMat(1,0) + scaleMat(2,2) * rotToAxisMat(2,0);
	float h = scaleMat(0,2) * rotToAxisMat(0,1) + scaleMat(1,2) * rotToAxisMat(1,1) + scaleMat(2,2) * rotToAxisMat(2,1);
	float i = scaleMat(0,2) * rotToAxisMat(0,2) + scaleMat(1,2) * rotToAxisMat(1,2) + scaleMat(2,2) * rotToAxisMat(2,2);

	scaleMat(0,0) = a;
	scaleMat(1,0) = b;
	scaleMat(2,0) = c;

	scaleMat(0,1) = d;
	scaleMat(1,1) = e;
	scaleMat(2,1) = f;

	scaleMat(0,2) = g;
	scaleMat(1,2) = h;
	scaleMat(2,2) = i;
}

//struct PARAMS_NAME(ScaleAlongVelocity)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ScaleAlongVelocity)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	scaleFactor)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierScaleAlongVelocity(const PARAMS_NAME(ScaleAlongVelocity)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
	if (usage == nvidia::apex::ModifierUsage_Mesh)
	{
		PxVec3 vel = input.velocity;
		float velMagnitude = vel.normalize(); // normalize it

		PxMat33 scaleMat(PxIdentity);
		if (velMagnitude >= 0.0001f)
		{
			float scale = 1.0f + velMagnitude * params.scaleFactor;

			buildScaleAlongAxis(vel, scale, scaleMat);
		}
		pubState.rotation = scaleMat * pubState.rotation;
	}
}

//--- RandomSubtexture modifier ---

//struct PARAMS_NAME(RandomSubtexture)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(RandomSubtexture)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	subtextureRangeMin) \
	INPLACE_TYPE_FIELD(float,	subtextureRangeMax)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 1;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierRandomSubtexture(const PARAMS_NAME(RandomSubtexture)& params, const Input& /*input*/, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& randState)
{
	if (usage == nvidia::apex::ModifierUsage_Sprite)
	{
		pubState.subTextureId += randState.nextFloat(params.subtextureRangeMin, params.subtextureRangeMax);
	}
}

//--- RandomRotation modifier ---

//struct PARAMS_NAME(RandomRotation)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(RandomRotation)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	rotationRangeMin) \
	INPLACE_TYPE_FIELD(float,	rotationRangeMax)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 1;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierRandomRotation(const PARAMS_NAME(RandomRotation)& params, const Input& /*input*/, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& randState)
{
	if (usage == nvidia::apex::ModifierUsage_Sprite)
	{
		const float DegToRad = PI / 180;
		pubState.rotation += DegToRad * randState.nextFloat(params.rotationRangeMin, params.rotationRangeMax);
	}
}

//--- ScaleVsLife modifier ---

//struct PARAMS_NAME(ScaleVsLife)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ScaleVsLife)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	axis) \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierScaleVsLife(const PARAMS_NAME(ScaleVsLife)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
#define _MODIFIER_CODE_(axis) \
	{ \
		pubState.scale[axis] *= EVAL_CURVE(params.curve, input.liferemain); \
	}
#if _CUDA_OPT_LOC_MEM_
	if (params.axis == 0) _MODIFIER_CODE_(0)
		else if (params.axis == 1) _MODIFIER_CODE_(1)
			else if (params.axis == 2) _MODIFIER_CODE_(2)
#else
	_MODIFIER_CODE_(params.axis)
#endif
#undef _MODIFIER_CODE_
			}

//--- ScaleVsDensity modifier ---

//struct PARAMS_NAME(ScaleVsDensity)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ScaleVsDensity)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	axis) \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierScaleVsDensity(const PARAMS_NAME(ScaleVsDensity)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
#define _MODIFIER_CODE_(axis) \
	{ \
		pubState.scale[axis] *= EVAL_CURVE(params.curve, input.density); \
	}
#if _CUDA_OPT_LOC_MEM_
	if (params.axis == 0) _MODIFIER_CODE_(0)
		else if (params.axis == 1) _MODIFIER_CODE_(1)
			else if (params.axis == 2) _MODIFIER_CODE_(2)
#else
	_MODIFIER_CODE_(params.axis)
#endif
#undef _MODIFIER_CODE_
			}

//--- ScaleVsCameraDistance modifier ---

//struct PARAMS_NAME(ScaleVsCameraDistance)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ScaleVsCameraDistance)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(uint32_t,	axis) \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierScaleVsCameraDistance(const PARAMS_NAME(ScaleVsCameraDistance)& params, const Input& input, PubState& pubState, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& common, nvidia::RandState& /*nvidia::RandState*/)
{
	float cameraDistance = (input.position - common.eyePosition).magnitude();

#define _MODIFIER_CODE_(axis) \
	{ \
		pubState.scale[axis] *= EVAL_CURVE(params.curve, cameraDistance); \
	}
#if _CUDA_OPT_LOC_MEM_
	if (params.axis == 0) _MODIFIER_CODE_(0)
		else if (params.axis == 1) _MODIFIER_CODE_(1)
			else if (params.axis == 2) _MODIFIER_CODE_(2)
#else
	_MODIFIER_CODE_(params.axis)
#endif
#undef _MODIFIER_CODE_
			}

//--- ViewDirectionSorting modifier ---

//struct PARAMS_NAME(ViewDirectionSorting)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(ViewDirectionSorting)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierViewDirectionSorting(const PARAMS_NAME(ViewDirectionSorting)& /*params*/, const Input& /*input*/, PubState& /*pubState*/, PrivState& /*privState*/, const nvidia::iofx::ModifierCommonParams& /*common*/, nvidia::RandState& /*nvidia::RandState*/)
{
}

//--- RotationRate modifier ---

//struct PARAMS_NAME(RotationRate)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(RotationRate)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	rotationRate)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierRotationRate(const PARAMS_NAME(RotationRate)& params, const Input& /*input*/, PubState& pubState, PrivState& privState, const nvidia::iofx::ModifierCommonParams& common, nvidia::RandState& /*nvidia::RandState*/)
{
	if (usage == nvidia::apex::ModifierUsage_Sprite)
	{
		const float TwoPi = 2 * PI;
		const float rotationDelta = params.rotationRate * TwoPi * common.deltaTime;
		privState.rotation += rotationDelta;
		pubState.rotation += privState.rotation;
	}
}

//--- RotationRateVsLife modifier ---

//struct PARAMS_NAME(RotationRateVsLife)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(RotationRateVsLife)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(CURVE_TYPE,		curve)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierRotationRateVsLife(const PARAMS_NAME(RotationRateVsLife)& params, const Input& input, PubState& pubState, PrivState& privState, const nvidia::iofx::ModifierCommonParams& common, nvidia::RandState& /*nvidia::RandState*/)
{
	if (usage == nvidia::apex::ModifierUsage_Sprite)
	{
		const float TwoPi = 2 * PI;
		const float rotationDelta = EVAL_CURVE(params.curve, input.liferemain) * TwoPi * common.deltaTime;
		privState.rotation += rotationDelta;
		pubState.rotation += privState.rotation;
	}
}

//--- OrientScaleAlongScreenVelocity modifier ---

//struct PARAMS_NAME(OrientScaleAlongScreenVelocity)
#define INPLACE_TYPE_STRUCT_NAME PARAMS_NAME(OrientScaleAlongScreenVelocity)
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(float,	scalePerVelocity) \
	INPLACE_TYPE_FIELD(float,	scaleChangeLimit) \
	INPLACE_TYPE_FIELD(float,	scaleChangeDelay)
#define INPLACE_TYPE_STRUCT_LEAVE_OPEN 1
#include INPLACE_TYPE_BUILD()

	static const uint32_t RANDOM_COUNT = 0;
};

INPLACE_TEMPL_VA_ARGS_DEF(bool spawn, int usage, typename Input, typename PubState, typename PrivState)
MODIFIER_DECL void modifierOrientScaleAlongScreenVelocity(const PARAMS_NAME(OrientScaleAlongScreenVelocity)& params, const Input& input, PubState& pubState, PrivState& privState, const nvidia::iofx::ModifierCommonParams& common, nvidia::RandState& /*nvidia::RandState*/)
{
	if (usage == nvidia::apex::ModifierUsage_Sprite)
	{
		PxVec3 viewPos, viewVel; // position & velocity in view space

		viewPos.x = (input.position - common.eyePosition).dot(common.eyeAxisX);
		viewPos.y = (input.position - common.eyePosition).dot(common.eyeAxisY);
		viewPos.z = (input.position - common.eyePosition).dot(common.eyeDirection);   // eyeDir = eyeAxisZ

		// 4 is simply a hack that looks decent in UE3 ATM, we should use the FOV to determine when to
		// cull particles, otherwise you end up with particles around the zNear plane that have huge
		// 'd' values, resulting in bad scaling
		if (viewPos.z < common.zNear * 4)
		{
			pubState.rotation = privState.rotation;
			pubState.scale.x = 0.0f;
			pubState.scale.y = 0.0f;
			return;
		}

		viewVel.x = (input.velocity).dot(common.eyeAxisX);
		viewVel.y = (input.velocity).dot(common.eyeAxisY);
		viewVel.z = (input.velocity).dot(common.eyeDirection);   // eyeDir = eyeAxisZ

		// tan(angle) = (Vy*Pz - Py*Vz) / (Vx*Pz - Px*Vz)
		const float velX = viewVel.x * viewPos.z - viewPos.x * viewVel.z;
		const float velY = viewVel.y * viewPos.z - viewPos.y * viewVel.z;

		const float velLengthMultiplier = 1.0f / viewPos.z;

		const float dx = velX * velLengthMultiplier;
		const float dy = velY * velLengthMultiplier;

		const float d = PxSqrt(dx * dx + dy * dy);

		if (d >= 1e-5f)
		{
			// "Note that the order of arguments is reversed; the function atan2(y,x) computes
			//  the angle corresponding to the point (x,y)."
			// see http://en.wikipedia.org/wiki/Atan2
			privState.rotation = PxAtan2(velY, velX);

			float scale = 1.0f + d * params.scalePerVelocity;
			if (params.scaleChangeDelay > 0.0f)
			{
				const float blendFactor = PxExp(-common.deltaTime / params.scaleChangeDelay);
				scale *= (1.0f - blendFactor);
				scale += blendFactor * privState.scale;
			}
			if (params.scaleChangeLimit > 0.0f)
			{
				const float scaleChangeLimit = params.scaleChangeLimit * common.deltaTime;
				const float scaleChange = scale - privState.scale;
				if (scaleChange > +scaleChangeLimit)
				{
					scale = privState.scale + scaleChangeLimit;
				}
				else if (scaleChange < -scaleChangeLimit)
				{
					scale = privState.scale - scaleChangeLimit;
				}
			}
			privState.scale = scale;
			pubState.scale.x *= scale;
		}
		pubState.rotation = privState.rotation;
	}
}

#undef _CUDA_OPT_LOC_MEM_
