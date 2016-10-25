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
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.

#include <PsHashMap.h>
#include <PsHash.h>
#include <PxAssert.h>
#include <PsUserAllocated.h>

#include "nvparameterized/NvParameterized.h"
#include "nvparameterized/NvParameterizedTraits.h"
#include "NvTraitsInternal.h"
#include "SerializerCommon.h"

namespace NvParameterized
{

class DefaultParamConversion: public Conversion
{
	Traits *mTraits;
	RefConversionMode::Enum mRefMode;

	typedef physx::shdfnd::HashMap<const char *, uint32_t, physx::shdfnd::Hash<const char *>, Traits::Allocator> MapName2Ver;
	MapName2Ver mapName2Ver;

	bool copy(Handle &legacyHandle, Handle &handle)
	{
		const Definition *pdLegacy = legacyHandle.parameterDefinition(),
			*pd = handle.parameterDefinition();

		if( pdLegacy->type() != pd->type() )
			return true;

		switch( pdLegacy->type() )
		{
		case TYPE_STRUCT:
			for(int32_t i = 0; i < pdLegacy->numChildren(); ++i)
			{
				legacyHandle.set(i);

				int32_t j = -1;
				if( pd->child(pdLegacy->child(i)->name(), j) )
				{
					handle.set(j);
					if( !copy(legacyHandle, handle) )
					{
						char longName[128];
						legacyHandle.getLongName(longName, sizeof(longName));
						NV_PARAM_TRAITS_WARNING(
							mTraits,
							"%s: failed to copy legacy data",
							longName
						);

						return false;
					}
					handle.popIndex();
				}

				legacyHandle.popIndex();
			}
			break;

		case TYPE_ARRAY:
			{
				//FIXME: this is too harsh e.g. we can convert static to dynamic
				if( pdLegacy->arraySizeIsFixed() != pd->arraySizeIsFixed() )
					return true;

				int32_t size;
				if( pdLegacy->arraySizeIsFixed() )
				{
					//FIXME: this is too harsh e.g. we may just copy first elements
					if( pdLegacy->arraySize() != pd->arraySize() )
						return true;

					size = pd->arraySize();
				}
				else
				{
					if( ERROR_NONE != legacyHandle.getArraySize(size) )
						return false;

					handle.resizeArray(size);
				}

				if( size > 100 && pdLegacy->isSimpleType(false, false) && pdLegacy->type() == pd->type() )
				{
					// Large array of simple types, fast path

					switch( pdLegacy->type() )
					{
					NV_PARAMETRIZED_NO_MATH_DATATYPE_LABELS
					NV_PARAMETRIZED_LEGACY_DATATYPE_LABELS
					default:
						// Fall to slow path
						break;

#define NV_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#define NV_PARAMETERIZED_TYPES_NO_STRING_TYPES
#define NV_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
	case TYPE_ ## enum_name: { \
		c_type *data = (c_type *)mTraits->alloc(sizeof(c_type) * size); \
		if( ERROR_NONE != legacyHandle.getParam ## type_name ## Array(data, size) ) return false; \
		if( ERROR_NONE != handle.setParam ## type_name ## Array(data, size) ) return false; \
		mTraits->free(data); \
		break; \
	}
#include "nvparameterized/NvParameterized_types.h"

					}
				}

				// Slow path for arrays of aggregates, including TYPE_MAT34

				for(int32_t i = 0; i < size; ++i)
				{
					legacyHandle.set(i);
					handle.set(i);

					copy(legacyHandle, handle);

					handle.popIndex();
					legacyHandle.popIndex();
				}

				break;
			}

		case TYPE_REF:
			{
				Interface *legacyRefObj;
				legacyHandle.getParamRef(legacyRefObj);

				// Skip references which have unsupported classes (keep default version)
				if( legacyRefObj && -1 == handle.parameterDefinition()->refVariantValIndex(legacyRefObj->className()) )
					return true;

				Interface *refObj;
				handle.getParamRef(refObj);

				switch( mRefMode )
				{
				case RefConversionMode::REF_CONVERT_LAST:
				default:
					DEBUG_ALWAYS_ASSERT();
					break;

				case RefConversionMode::REF_CONVERT_SKIP:
					break;

				case RefConversionMode::REF_CONVERT_COPY:
					{
						// TODO: check that this class is allowed in new version?

						// Update to preferred version if necessary
						if( legacyRefObj && pdLegacy->isIncludedRef() )
						{
							const char *className = legacyRefObj->className();

							const MapName2Ver::Entry *e = mapName2Ver.find(pd->longName());
							if( e )
							{
								uint32_t prefVer = e->second,
									curVer = legacyRefObj->version();

								if( curVer > prefVer )
								{
									NV_PARAM_TRAITS_WARNING(
										mTraits,
										"Unable to upgrade included reference of class %s, "
											"actual version (%u) is higher than preferred version (%u), "
											"your conversion may not work properly",
										className,
										(unsigned)curVer,
										(unsigned)prefVer
									);
									DEBUG_ALWAYS_ASSERT();
									return false;
								}
								else if( curVer < prefVer )
								{
									Interface *newLegacyRefObj = mTraits->createNvParameterized(className, prefVer);

									if( !mTraits->updateLegacyNvParameterized(*legacyRefObj, *newLegacyRefObj) )
									{
										NV_PARAM_TRAITS_WARNING(
											mTraits,
											"Failed to upgrade reference of class %s to version %u",
											className,
											(unsigned)prefVer
										);
										DEBUG_ALWAYS_ASSERT();
										newLegacyRefObj->destroy();
										return false;
									}

									legacyRefObj->destroy();
									legacyRefObj = newLegacyRefObj;

									legacyHandle.setParamRef(legacyRefObj);
								}
								else
								{
									// Object has preferred version => do not do anything
								}
							}
						}

						if( ERROR_NONE != handle.setParamRef(legacyRefObj) )
							return true;

						if( ERROR_NONE != legacyHandle.setParamRef(0) ) { //Prevent destruction in legacyObj->destroy()
							handle.setParamRef(0);
							return false;
						}

						if( refObj )
							refObj->destroy();

						return true;
					}

				case RefConversionMode::REF_CONVERT_UPDATE:
					{
						DEBUG_ASSERT( 0 && "This was not used/tested for a long time" );
						return false;

/*						if( !refObj )
						{
							//TODO: do we need to create refObj here?
							return false;
						}

						if( 0 != strcmp(legacyRefObj->className(), refObj->className()) )
						{
							if( NvParameterized::ERROR_NONE != handle.initParamRef(legacyRefObj->className(), true) )
								return false;

							if( NvParameterized::ERROR_NONE != handle.getParamRef(refObj) )
								return false;
						}

						if( legacyRefObj->version() != refObj->version() )
							return mTraits->updateLegacyNvParameterized(*legacyRefObj, *refObj);

						//Same version => recursive copy

						Handle refHandle(*refObj, "");
						DEBUG_ASSERT( refHandle.isValid() );

						Handle legacyRefHandle(*legacyRefObj, "");
						DEBUG_ASSERT( legacyRefHandle.isValid() );

						return copy(legacyRefHandle, refHandle);*/
					}
				} //switch( mRefMode )

				break;
			}

		case TYPE_POINTER:
			// do nothing?
			break;

		case TYPE_ENUM:
			{
				const char *val;
				legacyHandle.getParamEnum(val);

				int32_t valIdx = handle.parameterDefinition()->enumValIndex(val);
				if(-1 != valIdx)
				{
					const char *canonicVal = handle.parameterDefinition()->enumVal(valIdx);
					if( ERROR_NONE != handle.setParamEnum(canonicVal) )
					{
						NV_PARAM_TRAITS_WARNING(
							mTraits, "Internal error while setting enum value %s", canonicVal ? canonicVal : "" );
						DEBUG_ALWAYS_ASSERT();
						return false;
					}
				}

				break;
			}

#		define NV_PARAMETERIZED_TYPES_NO_STRING_TYPES
#		define NV_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#		define NV_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
		case TYPE_##enum_name: { c_type val; legacyHandle.getParam##type_name(val); handle.setParam##type_name(val); break; }

		NV_PARAMETERIZED_TYPE(String, STRING, const char *)

#		include "nvparameterized/NvParameterized_types.h"

		case TYPE_MAT34: { float val[12]; legacyHandle.getParamMat34Legacy(val); handle.setParamMat34Legacy(val); break; }

NV_PARAMETRIZED_UNDEFINED_AND_LAST_DATATYPE_LABELS
		default:
			DEBUG_ALWAYS_ASSERT();
		}

		return true;
	}

public:
	DefaultParamConversion(Traits *traits, const PrefVer *prefVers, RefConversionMode::Enum refMode)
		: mTraits(traits), mRefMode(refMode), mapName2Ver(0, 0.75, Traits::Allocator(traits))
	{
		for(; prefVers && prefVers->longName; ++prefVers)
			mapName2Ver[prefVers->longName] = prefVers->ver;
	}

	bool operator()(NvParameterized::Interface &legacyObj, NvParameterized::Interface &obj)
	{
		obj.initDefaults();

		Handle legacyHandle(legacyObj, "");
		DEBUG_ASSERT( legacyHandle.isValid() );

		Handle handle(obj, "");
		DEBUG_ASSERT( handle.isValid() );

		return copy(legacyHandle, handle);
	}

	void release()
	{
		this->~DefaultParamConversion();
		mTraits->free(this);
	}
};

Conversion *internalCreateDefaultConversion(Traits *traits, const PrefVer *prefVers, RefConversionMode::Enum refMode)
{
	void *buf = traits->alloc(sizeof(DefaultParamConversion));
	return PX_PLACEMENT_NEW(buf, DefaultParamConversion)(traits, prefVers, refMode);
}

}; // end of namespace
