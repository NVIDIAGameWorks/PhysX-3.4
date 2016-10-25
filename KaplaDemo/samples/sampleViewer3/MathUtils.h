#ifndef MATH_UTILS
#define MATH_UTILS

#include "foundation/PxVec3.h"
#include "foundation/PxQuat.h"
#include "foundation/PxMath.h"

using namespace physx;

// ------------------------------------------------------------------------
PX_FORCE_INLINE PxF32 randRange(const PxF32 a, const PxF32 b)	
{
	return a + (b-a)*::rand()/RAND_MAX;
}

// ------------------------------------------------------------------------
inline static float fastInvSqrt(float input)
{
	const float halfInput = 0.5f * input;
	int         i     = *(int*)&input;

	i = 0x5f375a86 - ( i >> 1 );
	input = *(float*) & i;
	input = input * ( 1.5f - halfInput * input * input);
	return input;
}

// ------------------------------------------------------------------------
inline static void fastNormalize(PxVec3 &v) {
	v *= fastInvSqrt(v.magnitudeSquared());
}

// ------------------------------------------------------------------------
inline static float fastLen(PxVec3 &v) {
	return 1.0f / fastInvSqrt(v.magnitudeSquared());
}

// ------------------------------------------------------------------------
inline static float fastNormalizeLen(PxVec3 &v) {
	float inv = fastInvSqrt(v.magnitudeSquared());
	v *= inv;
	return 1.0f / inv;
}

// ------------------------------------------------------------------------
inline static float fastAcos(float x) {
	// MacLaurin series
	if (x < -1.0f) x = -1.0f;
	if (x > 1.0f) x = 1.0f;
	float x2 = x*x;
	float angle = (35.0f/1152.0f)*x2;
	angle = (5.0f/112.0f) + angle*x2;
	angle = (3.0f/40.0f) + angle*x2;
	angle = (1.0f/6.0f) + angle*x2;
	angle =  1        + angle*x2;
	angle =	 1.5707963267948966f - angle * x;		
	return angle;	
}

// ------------------------------------------------------------------------
inline static void quatFromVectors(const PxVec3 &n0, const PxVec3 &n1, PxQuat &q)
{
	// fast approximation
	PxVec3 axis = n0.cross(n1) * 0.5f;
	q = PxQuat(axis.x, axis.y, axis.z, PxSqrt(1.0f - axis.magnitudeSquared()));

	// correct
	//PxVec3 axis = n0 ^ n1;
	//axis.normalize();
	//float s = PxMath::sqrt(0.5f * (1.0f - n0.dot(n1)));
	//q.x = axis.x * s;
	//q.y = axis.y * s;
	//q.z = axis.z * s;
	//q.w = PxMath::sqrt(1.0f - s*s);
}

// ------------------------------------------------------------------------
void polarDecomposition(const PxMat33 &A, PxMat33 &R);
void polarDecompositionStabilized(const PxMat33 &A, PxMat33 &R);

// ------------------------------------------------------------------------
void eigenDecomposition(PxMat33 &A, PxMat33 &R);

// ------------------------------------------------------------------------
void eigenDecomposition22(const PxMat33 &A, PxMat33 &R, PxMat33 &D);

// ------------------------------------------------------------------------
PxMat33 outerProduct(const PxVec3 &v0, const PxVec3 &v1);

// ------------------------------------------------------------------------
PxQuat align(const PxVec3& v1, const PxVec3& v2);

// ------------------------------------------------------------------------
void decomposeTwistTimesSwing(const PxQuat& q,  const PxVec3& v1,
		PxQuat& twist, PxQuat& swing);

// ------------------------------------------------------------------------
void decomposeSwingTimesTwist(const PxQuat& q, const PxVec3& v1,
		PxQuat& swing, PxQuat& twist);


#endif