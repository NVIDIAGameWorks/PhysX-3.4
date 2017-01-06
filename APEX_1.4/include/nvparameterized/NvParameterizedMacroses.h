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
 
#ifndef NV_PARAMETERIZED_MACROSES_H
#define NV_PARAMETERIZED_MACROSES_H

/** Case labels for different NvParameterized::DataType's
*/
#define NV_PARAMETRIZED_AGGREGATE_DATATYPE_LABELS \
case NvParameterized::TYPE_ARRAY:\
case NvParameterized::TYPE_STRUCT:\

#define NV_PARAMETRIZED_LINAL_DATATYPE_LABELS \
case NvParameterized::TYPE_VEC2:\
case NvParameterized::TYPE_VEC3:\
case NvParameterized::TYPE_VEC4:\
case NvParameterized::TYPE_QUAT:\
case NvParameterized::TYPE_MAT33:\
case NvParameterized::TYPE_BOUNDS3:\
case NvParameterized::TYPE_MAT44:\
case NvParameterized::TYPE_TRANSFORM:\
case NvParameterized::TYPE_MAT34:
 
#define NV_PARAMETRIZED_ARITHMETIC_DATATYPE_LABELS \
case NvParameterized::TYPE_I8:\
case NvParameterized::TYPE_I16:\
case NvParameterized::TYPE_I32:\
case NvParameterized::TYPE_I64:\
case NvParameterized::TYPE_U8:\
case NvParameterized::TYPE_U16:\
case NvParameterized::TYPE_U32:\
case NvParameterized::TYPE_U64:\
case NvParameterized::TYPE_F32:\
case NvParameterized::TYPE_F64:\
case NvParameterized::TYPE_BOOL: 

#define NV_PARAMETRIZED_STRING_DATATYPE_LABELS \
case NvParameterized::TYPE_STRING:

#define NV_PARAMETRIZED_REF_DATATYPE_LABELS \
case NvParameterized::TYPE_REF:

#define NV_PARAMETRIZED_ENUM_DATATYPE_LABELS \
case NvParameterized::TYPE_ENUM:

#define NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
case NvParameterized::TYPE_POINTER:

#define NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS \
case NvParameterized::TYPE_UNDEFINED:\
case NvParameterized::TYPE_LAST:

/** Case labels for composite conditions of switch conditions*/
/***********************************************************************/
#define NV_PARAMETRIZED_NO_AGGREGATE_DATATYPE_LABELS \
/* NV_PARAMETRIZED_AGGREGATE_DATATYPE_LABELS*/ \
NV_PARAMETRIZED_LINAL_DATATYPE_LABELS \
NV_PARAMETRIZED_ARITHMETIC_DATATYPE_LABELS \
NV_PARAMETRIZED_STRING_DATATYPE_LABELS \
NV_PARAMETRIZED_REF_DATATYPE_LABELS \
NV_PARAMETRIZED_ENUM_DATATYPE_LABELS \
NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS

#define NV_PARAMETRIZED_NO_AGGREGATE_AND_ARITHMETIC_DATATYPE_LABELS \
/* NV_PARAMETRIZED_AGGREGATE_DATATYPE_LABELS*/ \
NV_PARAMETRIZED_LINAL_DATATYPE_LABELS \
/*NV_PARAMETRIZED_ARITHMETIC_DATATYPE_LABELS*/ \
NV_PARAMETRIZED_STRING_DATATYPE_LABELS \
NV_PARAMETRIZED_REF_DATATYPE_LABELS \
NV_PARAMETRIZED_ENUM_DATATYPE_LABELS \
NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS

#define NV_PARAMETRIZED_NO_AGGREGATE_AND_REF_DATATYPE_LABELS \
/* NV_PARAMETRIZED_AGGREGATE_DATATYPE_LABELS*/ \
NV_PARAMETRIZED_LINAL_DATATYPE_LABELS \
NV_PARAMETRIZED_ARITHMETIC_DATATYPE_LABELS \
NV_PARAMETRIZED_STRING_DATATYPE_LABELS \
/*NV_PARAMETRIZED_REF_DATATYPE_LABELS*/ \
NV_PARAMETRIZED_ENUM_DATATYPE_LABELS \
NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS

#define NV_PARAMETRIZED_NO_ARITHMETIC_AND_LINAL_DATATYPE_LABELS \
NV_PARAMETRIZED_AGGREGATE_DATATYPE_LABELS \
/*NV_PARAMETRIZED_LINAL_DATATYPE_LABELS*/ \
/*NV_PARAMETRIZED_ARITHMETIC_DATATYPE_LABELS*/ \
NV_PARAMETRIZED_STRING_DATATYPE_LABELS \
NV_PARAMETRIZED_REF_DATATYPE_LABELS \
NV_PARAMETRIZED_ENUM_DATATYPE_LABELS \
NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS

#define NV_PARAMETRIZED_SERVICE_AND_LAST_DATATYPE_LABELS \
NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
case NvParameterized::TYPE_LAST:

#define NV_PARAMETRIZED_LEGACY_DATATYPE_LABELS \
case NvParameterized::TYPE_MAT34:

#define NV_PARAMETRIZED_NO_MATH_DATATYPE_LABELS \
NV_PARAMETRIZED_AGGREGATE_DATATYPE_LABELS \
NV_PARAMETRIZED_STRING_DATATYPE_LABELS \
NV_PARAMETRIZED_REF_DATATYPE_LABELS \
NV_PARAMETRIZED_ENUM_DATATYPE_LABELS \
NV_PARAMETRIZED_SERVICE_DATATYPE_LABELS \
NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS

/***********************************************************************/

#endif // NV_PARAMETERIZED_MACROSES_H
