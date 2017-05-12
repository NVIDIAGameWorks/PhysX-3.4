/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef APEX_SIMD_MATH_H
#define APEX_SIMD_MATH_H

#include "PxMat44.h"
#include "PxVec3.h"
#include "PsMathUtils.h"
#include "simd/NvSimd4f.h"

namespace nvidia
{
	/** Normalization of the (a[0], a[1], a[2]) vector
	*  @param a  input vector
	*  @return normalized vector
	*/
	inline Simd4f normalizeSimd3f(const Simd4f& a) 
	{
		return a * rsqrt(dot3(a, a));
	}

	/** Create simd 4-float tuple from vec3 and wComponent
	* @param vec3 vector with 3 components
	* @param wComponent with this value final element will be initialized
	* @return filled simd 4-float tuple
	*/
	inline Simd4f createSimd3f(const physx::PxVec3& vec3, float wComponent = 0.0f)
	{
		return Simd4fLoad3SetWFactory(&vec3.x, wComponent);
	}

	/** Apply affine transform to position. Algorithm is not sensitive to pos.w.
	* @param transformAlignMemLayout transform
	* @param pos input position.
	* @return transformed position. With pos.w setuped to one.
	*/
	inline Simd4f applyAffineTransform(const physx::PxMat44& transformAlignMemLayout, const Simd4f& pos)
	{
		const physx::PxMat44& tr = transformAlignMemLayout;
		
		const Simd4f& col0 = Simd4fAlignedLoadFactory(&tr.column0.x);
		const Simd4f xMultiplier = splat<0>(pos);

		const Simd4f& col1 = Simd4fAlignedLoadFactory(&tr.column1.x);
		const Simd4f yMultiplier = splat<1>(pos);

		const Simd4f& col2 = Simd4fAlignedLoadFactory(&tr.column2.x);		
		const Simd4f zMultiplier = splat<2>(pos);

		Simd4f result = xMultiplier * col0 + yMultiplier * col1 + zMultiplier * col2 + Simd4fAlignedLoadFactory(&tr.column3.x);

		array(result)[3] = 1.0f;

		return result;
	}

	/** Apply linear transform to position or more probability to the vector(direction)
	* @param transformAlignMemLayout transform
	* @param pos input position. Algo does not sensitive to pos.w.
	* @return transformed position. With pos.w setuped to zero.
	*/
	inline Simd4f applyLinearTransform(const physx::PxMat44& transformAlignMemLayout, const Simd4f& direction)
	{
		const physx::PxMat44& tr = transformAlignMemLayout;	
		const Simd4f& col0 = Simd4fAlignedLoadFactory(&tr.column0.x);
		const Simd4f xMultiplier = splat<0>(direction);
		const Simd4f& col1 = Simd4fAlignedLoadFactory(&tr.column1.x);
		const Simd4f yMultiplier = splat<1>(direction);
		const Simd4f& col2 = Simd4fAlignedLoadFactory(&tr.column2.x);		
		const Simd4f zMultiplier = splat<2>(direction);
		Simd4f result = xMultiplier * col0 + yMultiplier * col1 + zMultiplier * col2;
		result = result & gSimd4fMaskXYZ;

		return result;
	}

	/** Apply transpose of matrix consisted of col0, col1, col2, col3. 
	* Ported version of V4Transpose() from PxShared\*\foundation\include\PsVecMathAoSScalarInline.h
	* @param col0 input column of the matrix, and output column of the result matrix
	* @param col1 input column of the matrix, and output column of the result matrix
	* @param col2 input column of the matrix, and output column of the result matrix
	* @param col3 input column of the matrix, and output column of the result matrix
	* @return None
	*/
	inline void applyTranspose(Simd4f& col0, Simd4f& col1, Simd4f& col2, Simd4f& col3)
	{
		/*
		     col0    col1    col2    col3
	     0  col0[0] col0[1] col0[1] col0[1]
		 1  col0[1] col0[1] col0[1] col0[1]
		 2  col0[2] col0[1] col0[1] col0[1]
		 3  col0[3] col0[1] col0[1] col0[1]
		*/

		float* arrayCol0 = array(col0);
		float* arrayCol1 = array(col1);
		float* arrayCol2 = array(col2);
		float* arrayCol3 = array(col3);
				
		using physx::PxF32;
		const PxF32 t01 = arrayCol0[1];
		const PxF32 t02 = arrayCol0[2];
		const PxF32 t03 = arrayCol0[3];
		const PxF32 t12 = arrayCol1[2];
		const PxF32 t13 = arrayCol1[3];
		const PxF32 t23 = arrayCol2[3];

		// x -- 0, y -- 1, z -- 2, w -- 3
		arrayCol0[1] = arrayCol1[0];	arrayCol0[2] = arrayCol2[0];	arrayCol0[3] = arrayCol3[0];
		arrayCol1[2] = arrayCol2[1];	arrayCol1[3] = arrayCol3[1];
		arrayCol2[3] = arrayCol3[2];

		arrayCol1[0] = t01;		arrayCol2[0] = t02;		arrayCol3[0] = t03;
		arrayCol2[1] = t12;		arrayCol3[1] = t13;
		arrayCol3[2] = t23;
	}
}

#endif // APEX_SIND_MATH_H
