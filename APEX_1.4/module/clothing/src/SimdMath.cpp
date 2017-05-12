#include "SimdMath.h"

#include "PxMat44.h"
#include "PxMat33.h"
#include "PxVec3.h"
#include "PxVec4.h"
#include "PxQuat.h"

using physx::PxMat44;
using physx::PxMat33;
using physx::PxVec3;
using physx::PxVec4;
using physx::PxQuat;

#include "PxPreprocessor.h"
#include "PxAssert.h"
#include "ApexMath.h"

namespace nvidia
{

	bool operator != (const PxMat44& a, const PxMat44& b)
	{
		PX_ASSERT((((size_t)&a) & 0xf) == 0); // verify 16 byte alignment
		PX_ASSERT((((size_t)&b) & 0xf) == 0); // verify 16 byte alignment

		int allEq = true;
		
		typedef Simd4fLoadFactory loadFactory;

		const Simd4f ca1 = loadFactory(&a.column0.x);
		const Simd4f cb1 = loadFactory(&b.column0.x);
		allEq &= allEqual(ca1, cb1);

		const Simd4f ca2 = loadFactory(&a.column1.x);
		const Simd4f cb2 = loadFactory(&b.column1.x);
		allEq &= allEqual(ca2, cb2);

		const Simd4f ca3 = loadFactory(&a.column2.x);
		const Simd4f cb3 = loadFactory(&b.column2.x);
		allEq &= allEqual(ca3, cb3);

		const Simd4f ca4 = loadFactory(&a.column3.x);
		const Simd4f cb4 = loadFactory(&b.column3.x);
		allEq &= allEqual(ca4, cb4);
		
		return allEq == 0;
	}
}
