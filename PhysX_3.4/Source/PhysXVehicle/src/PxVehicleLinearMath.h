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

#ifndef PX_VEHICLE_LINEAR_MATH_H
#define PX_VEHICLE_LINEAR_MATH_H
/** \addtogroup vehicle
  @{
*/

#include "PxVehicleSDK.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

#define MAX_VECTORN_SIZE (PX_MAX_NB_WHEELS+3)

class VectorN
{
public:

	VectorN(const PxU32 size)
		: mSize(size)
	{
		PX_ASSERT(mSize <= MAX_VECTORN_SIZE);
	}
	~VectorN()
	{
	}

	VectorN(const VectorN& src)
	{
		for(PxU32 i = 0; i < src.mSize; i++)
		{
			mValues[i] = src.mValues[i];
		}
		mSize = src.mSize;
	}

	PX_FORCE_INLINE VectorN& operator=(const VectorN& src)
	{
		for(PxU32 i = 0; i < src.mSize; i++)
		{
			mValues[i] = src.mValues[i];
		}
		mSize = src.mSize;
		return *this;
	}

	PX_FORCE_INLINE PxF32& operator[] (const PxU32 i)
	{
		PX_ASSERT(i < mSize);
		return (mValues[i]);
	}

	PX_FORCE_INLINE const PxF32& operator[] (const PxU32 i) const
	{
		PX_ASSERT(i < mSize);
		return (mValues[i]);
	}

	PX_FORCE_INLINE PxU32 getSize() const {return mSize;}

private:

	PxF32 mValues[MAX_VECTORN_SIZE];
	PxU32 mSize;
};

class MatrixNN
{
public:

	MatrixNN()
		: mSize(0)
	{
	}
	MatrixNN(const PxU32 size)
		: mSize(size)
	{
		PX_ASSERT(mSize <= MAX_VECTORN_SIZE);
	}
	MatrixNN(const MatrixNN& src)
	{
		for(PxU32 i = 0; i < src.mSize; i++)
		{
			for(PxU32 j = 0; j < src.mSize; j++)
			{
				mValues[i][j] = src.mValues[i][j];
			}
		}
		mSize=src.mSize;
	}
	~MatrixNN()
	{
	}

	PX_FORCE_INLINE MatrixNN& operator=(const MatrixNN& src)
	{
		for(PxU32 i = 0;i < src.mSize; i++)
		{
			for(PxU32 j = 0;j < src.mSize; j++)
			{
				mValues[i][j] = src.mValues[i][j];
			}
		}
		mSize = src.mSize;
		return *this;
	}

	PX_FORCE_INLINE PxF32 get(const PxU32 i, const PxU32 j) const
	{
		PX_ASSERT(i < mSize);
		PX_ASSERT(j < mSize);
		return mValues[i][j];
	}
	PX_FORCE_INLINE void set(const PxU32 i, const PxU32 j, const PxF32 val)
	{
		PX_ASSERT(i < mSize);
		PX_ASSERT(j < mSize);
		mValues[i][j] = val;
	}

	PX_FORCE_INLINE PxU32 getSize() const {return mSize;}

	PX_FORCE_INLINE void setSize(const PxU32 size)
	{
		PX_ASSERT(size <= MAX_VECTORN_SIZE);
		mSize = size;
	}

public:

	PxF32 mValues[MAX_VECTORN_SIZE][MAX_VECTORN_SIZE];
	PxU32 mSize;
};


/*
	LUPQ decomposition

	Based upon "Outer Product LU with Complete Pivoting," from Matrix Computations (4th Edition), Golub and Van Loan

	Solve A*x = b using:

		MatrixNNLUSolver solver;
		solver.decomposeLU(A);
		solver.solve(b, x);
*/
class MatrixNNLUSolver
{
private:

	MatrixNN mLU;
	PxU32	mP[MAX_VECTORN_SIZE-1];	// Row permutation
	PxU32	mQ[MAX_VECTORN_SIZE-1];	// Column permutation
	PxF32	mdetM;

public:

	MatrixNNLUSolver(){}
	~MatrixNNLUSolver(){}

	PxF32 getDet() const {return mdetM;}

	void decomposeLU(const MatrixNN& A)
	{
		const PxU32 D = A.mSize;

		mLU = A;

		mdetM = 1.0f;

		for (PxU32 k = 0; k < D-1; ++k)
		{
			PxU32 pivot_row = k;
			PxU32 pivot_col = k;
			float abs_pivot_elem = 0.0f;
			for (PxU32 c = k; c < D; ++c)
			{
				for (PxU32 r = k; r < D; ++r)
				{
					const PxF32 abs_elem = PxAbs(mLU.get(r,c));
					if (abs_elem > abs_pivot_elem)
					{
						abs_pivot_elem = abs_elem;
						pivot_row = r;
						pivot_col = c;
					}
				}
			}

			mP[k] = pivot_row;
			if (pivot_row != k)
			{
				mdetM = -mdetM;
				for (PxU32 c = 0; c < D; ++c) 
				{
					//swap(m_LU(k,c), m_LU(pivot_row,c));
					const PxF32 pivotrowc = mLU.get(pivot_row, c);
					mLU.set(pivot_row, c, mLU.get(k, c));
					mLU.set(k, c, pivotrowc);
				}
			}

			mQ[k] = pivot_col;
			if (pivot_col != k)
			{
				mdetM = -mdetM;
				for (PxU32 r = 0; r < D; ++r) 
				{
					//swap(m_LU(r,k), m_LU(r,pivot_col));
					const PxF32 rpivotcol = mLU.get(r, pivot_col);
					mLU.set(r,pivot_col, mLU.get(r,k));
					mLU.set(r, k, rpivotcol);
				}
			}

			mdetM *= mLU.get(k,k);

			if (mLU.get(k,k) != 0.0f)
			{
				for (PxU32 r = k+1; r < D; ++r)
				{
					mLU.set(r, k, mLU.get(r,k) / mLU.get(k,k));
					for (PxU32 c = k+1; c < D; ++c) 
					{
						//m_LU(r,c) -= m_LU(r,k)*m_LU(k,c);
						const PxF32 rc = mLU.get(r, c);
						const PxF32 rk = mLU.get(r, k);
						const PxF32 kc = mLU.get(k, c);
						mLU.set(r, c, rc - rk*kc);
					}
				}
			}
		}

		mdetM *= mLU.get(D-1,D-1);
	}

	//Given a matrix A and a vector b find x that satisfies Ax = b, where the matrix A is the matrix that was passed to decomposeLU.
	//Returns true if the lu decomposition indicates that the matrix has an inverse and x was successfully computed.
	//Returns false if the lu decomposition resulted in zero determinant ie the matrix has no inverse and no solution exists for x.
	//Returns false if the size of either b or x doesn't match the size of the matrix passed to decomposeLU.
	//If false is returned then each relevant element of x is set to zero. 
	bool solve(const VectorN& b, VectorN& x) const
	{
		const PxU32 D = x.getSize();

		if((b.getSize() != x.getSize()) || (b.getSize() != mLU.getSize()) || (0.0f == mdetM))
		{
			for(PxU32 i = 0; i < D; i++)
			{
				x[i] = 0.0f;
			}
			return false;
		}

		x = b;

		// Perform row permutation to get Pb
		for(PxU32 i = 0; i < D-1; ++i) 
		{
			//swap(x(i), x(m_P[i]));														
			const PxF32 xp = x[mP[i]];
			x[mP[i]] = x[i];
			x[i] = xp;
		}

		// Forward substitute to get (L^-1)Pb
		for (PxU32 r = 1; r < D; ++r)
		{
			for (PxU32 i = 0; i < r; ++i) 
			{
				x[r] -= mLU.get(r,i)*x[i];								
			}
		}

		// Back substitute to get (U^-1)(L^-1)Pb
		for (PxU32 r = D; r-- > 0;) 
		{ 
			for (PxU32 i = r+1; i < D; ++i) 
			{
				x[r] -= mLU.get(r,i)*x[i]; 
			} 
			x[r] /= mLU.get(r,r);			
		}

		// Perform column permutation to get the solution (Q^T)(U^-1)(L^-1)Pb	
		for (PxU32 i = D-1; i-- > 0;) 
		{
			//swap(x(i), x(m_Q[i]));													
			const PxF32 xq = x[mQ[i]];
			x[mQ[i]] = x[i];
			x[i] = xq;
		}

		return true;
	}

};


class MatrixNGaussSeidelSolver
{
public:

	void solve(const PxU32 maxIterations, const PxF32 tolerance, const MatrixNN& A, const VectorN& b, VectorN& result) const
	{
		const PxU32 N = A.getSize();

		VectorN DInv(N);
		PxF32 bLength2 = 0.0f;
		for(PxU32 i = 0; i < N; i++)
		{
			DInv[i] = 1.0f/A.get(i,i);
			bLength2 += (b[i] * b[i]);
		}

		PxU32 iteration = 0;
		PxF32 error = PX_MAX_F32;
		while(iteration < maxIterations && tolerance < error)
		{
			for(PxU32 i = 0; i < N; i++)
			{
				PxF32 l = 0.0f;
				for(PxU32 j = 0; j < i; j++)
				{
					l += A.get(i,j) * result[j];
				}

				PxF32 u = 0.0f;
				for(PxU32 j = i + 1; j < N; j++)
				{
					u += A.get(i,j) * result[j];
				}

				result[i] = DInv[i] * (b[i] - l - u);
			}

			//Compute the error.
			PxF32 rLength2 = 0;
			for(PxU32 i = 0; i < N; i++)
			{
				PxF32 e = -b[i];
				for(PxU32 j = 0; j < N; j++)
				{
					e += A.get(i,j) * result[j];
				}
				rLength2 += e * e;
			}
			error = (rLength2 / (bLength2 + 1e-10f));

			iteration++;
		}
	}
};

class Matrix33Solver
{
public:

	bool solve(const MatrixNN& _A_, const VectorN& _b_, VectorN& result) const
	{
		const PxF32 a = _A_.get(0,0);
		const PxF32 b = _A_.get(0,1);
		const PxF32 c = _A_.get(0,2);

		const PxF32 d = _A_.get(1,0);
		const PxF32 e = _A_.get(1,1);
		const PxF32 f = _A_.get(1,2);

		const PxF32 g = _A_.get(2,0);
		const PxF32 h = _A_.get(2,1);
		const PxF32 k = _A_.get(2,2);

		const PxF32 detA = a*(e*k - f*h) - b*(k*d - f*g) + c*(d*h - e*g);
		if(0.0f == detA)
		{
			return false;
		}
		const PxF32 detAInv = 1.0f/detA;

		const PxF32 A = (e*k - f*h);
		const PxF32 D = -(b*k - c*h);
		const PxF32 G = (b*f - c*e);
		const PxF32 B = -(d*k - f*g);
		const PxF32 E = (a*k - c*g);
		const PxF32 H = -(a*f - c*d);
		const PxF32 C = (d*h - e*g);
		const PxF32 F = -(a*h - b*g);
		const PxF32 K = (a*e - b*d); 

		result[0] = detAInv*(A*_b_[0] + D*_b_[1] + G*_b_[2]);
		result[1] = detAInv*(B*_b_[0] + E*_b_[1] + H*_b_[2]);
		result[2] = detAInv*(C*_b_[0] + F*_b_[1] + K*_b_[2]);

		return true;
	}
};

#if !PX_DOXYGEN
} // namespace physx
#endif

#endif //PX_VEHICLE_LINEAR_MATH_H
