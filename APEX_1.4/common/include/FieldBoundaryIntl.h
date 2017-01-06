/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FIELD_BOUNDARY_INTL_H
#define FIELD_BOUNDARY_INTL_H

#include "InplaceTypes.h"

namespace nvidia
{
namespace apex
{


struct FieldShapeTypeIntl
{
	enum Enum
	{
		NONE = 0,
		SPHERE,
		BOX,
		CAPSULE,

		FORCE_DWORD = 0xFFFFFFFFu
	};
};

//struct FieldShapeDescIntl
//dimensions for
//SPHERE: x = radius
//BOX:    (x,y,z) = 1/2 size
//CAPUSE: x = radius, y = height
#define INPLACE_TYPE_STRUCT_NAME FieldShapeDescIntl
#define INPLACE_TYPE_STRUCT_FIELDS \
	INPLACE_TYPE_FIELD(InplaceEnum<FieldShapeTypeIntl::Enum>,	type) \
	INPLACE_TYPE_FIELD(PxTransform,				worldToShape) \
	INPLACE_TYPE_FIELD(PxVec3,						dimensions) \
	INPLACE_TYPE_FIELD(float,						weight)
#include INPLACE_TYPE_BUILD()


#ifndef __CUDACC__

struct FieldBoundaryDescIntl
{
#if PX_PHYSICS_VERSION_MAJOR == 3
	PxFilterData	boundaryFilterData;
#endif
};

class FieldBoundaryIntl
{
public:
	virtual bool updateFieldBoundary(physx::Array<FieldShapeDescIntl>& shapes) = 0;

protected:
	virtual ~FieldBoundaryIntl() {}
};
#endif

}
} // end namespace nvidia::apex

#endif // #ifndef FIELD_BOUNDARY_INTL_H
