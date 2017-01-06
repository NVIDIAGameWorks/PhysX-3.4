/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/filter.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/array/size.hpp>
#include <boost/preprocessor/array/elem.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>

#pragma warning(push)
#pragma warning(disable: 4355)

#define _PP_GET_FIELD_TYPE(n) BOOST_PP_ARRAY_ELEM(0, BOOST_PP_SEQ_ELEM(n, INPLACE_TYPE_STRUCT_FIELDS))
#define _PP_GET_FIELD_NAME(n) BOOST_PP_ARRAY_ELEM(1, BOOST_PP_SEQ_ELEM(n, INPLACE_TYPE_STRUCT_FIELDS))

#define _PP_HAS_FIELD_SIZE(n) BOOST_PP_EQUAL(BOOST_PP_ARRAY_SIZE(BOOST_PP_SEQ_ELEM(n, INPLACE_TYPE_STRUCT_FIELDS)), 3)
#define _PP_GET_FIELD_SIZE(n) BOOST_PP_ARRAY_ELEM(2, BOOST_PP_SEQ_ELEM(n, INPLACE_TYPE_STRUCT_FIELDS))


#ifdef INPLACE_TYPE_STRUCT_NAME

struct INPLACE_TYPE_STRUCT_NAME
#ifdef INPLACE_TYPE_STRUCT_BASE
	: INPLACE_TYPE_STRUCT_BASE
#endif
{
//fields
#ifdef INPLACE_TYPE_STRUCT_FIELDS
#define BOOST_PP_LOCAL_LIMITS (0, BOOST_PP_SEQ_SIZE(INPLACE_TYPE_STRUCT_FIELDS) - 1)
#define BOOST_PP_LOCAL_MACRO(n) \
	_PP_GET_FIELD_TYPE(n) _PP_GET_FIELD_NAME(n) BOOST_PP_IF(_PP_HAS_FIELD_SIZE(n), [##_PP_GET_FIELD_SIZE(n)##], BOOST_PP_EMPTY());
#include BOOST_PP_LOCAL_ITERATE()
#endif

//reflectSelf
	template <int _inplace_offset_, typename R, typename RA>
#if defined(INPLACE_TYPE_STRUCT_BASE) | defined(INPLACE_TYPE_STRUCT_FIELDS)
	APEX_CUDA_CALLABLE PX_INLINE void reflectSelf(R& r, RA ra)
	{
#ifdef INPLACE_TYPE_STRUCT_BASE
		INPLACE_TYPE_STRUCT_BASE::reflectSelf<_inplace_offset_>(r, ra);
#endif
#ifdef INPLACE_TYPE_STRUCT_FIELDS
#define BOOST_PP_LOCAL_LIMITS (0, BOOST_PP_SEQ_SIZE(INPLACE_TYPE_STRUCT_FIELDS) - 1)
#define BOOST_PP_LOCAL_MACRO(n) \
		InplaceTypeHelper::reflectType<APEX_OFFSETOF( INPLACE_TYPE_STRUCT_NAME, _PP_GET_FIELD_NAME(n) ) + _inplace_offset_>(r, ra, _PP_GET_FIELD_NAME(n), InplaceTypeMemberDefaultTraits());
#include BOOST_PP_LOCAL_ITERATE()
#endif
	}
#else
	APEX_CUDA_CALLABLE PX_INLINE void reflectSelf(R& , RA )
	{
	}
#endif

#ifndef INPLACE_TYPE_STRUCT_LEAVE_OPEN
};
#endif

#endif

#undef _PP_GET_FIELD_TYPE
#undef _PP_GET_FIELD_NAME
#undef _PP_HAS_FIELD_SIZE
#undef _PP_GET_FIELD_SIZE

#undef INPLACE_TYPE_STRUCT_NAME
#undef INPLACE_TYPE_STRUCT_BASE
#undef INPLACE_TYPE_STRUCT_FIELDS
#undef INPLACE_TYPE_STRUCT_LEAVE_OPEN

#pragma warning(pop)
