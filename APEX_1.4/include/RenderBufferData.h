/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef RENDER_BUFFER_DATA_H
#define RENDER_BUFFER_DATA_H

/*!
\file
\brief classes RenderSemanticData, RenderBufferData, ModuleSpecificRenderBufferData
*/

#include "ApexUsingNamespace.h"
#include "RenderDataFormat.h"
#include "ApexSDK.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Describes the data location and layout of a single semantic
*/
class RenderSemanticData
{
public:
	const void*				data;	//!< Pointer to first semantic data element
	uint32_t				stride;	//!< Semantic stride, in bytes
	void*					ident;	//!< Used for custom semantics only: value retrieved from NRP
	RenderDataFormat::Enum	format;	//!< Format used for this semantic
	/**
	\brief When this class is used for mesh authoring, this is the format of the source (user's) buffer.
		If different from 'format', and not RenderDataFormat::UNSPECIFIED, the data will be converted to
		from srcFormat to format (if a converter exists).
		If srcFormat = RenderDataFormat::UNSPECIFIED (default value), it is assumed that srcFormat = format.
	*/
	RenderDataFormat::Enum	srcFormat;
	/**
	\brief Copy this buffer from the authoring asset to the asset.

	This will only be used at render mesh asset creation time.
	*/
	bool					serialize;
public:
	PX_INLINE RenderSemanticData(void)
	{
		data   = 0;
		stride = 0;
		ident  = 0;
		format = RenderDataFormat::UNSPECIFIED;
		srcFormat = RenderDataFormat::UNSPECIFIED;
		serialize = false;
	}
};

/**
\brief Describes all of the semantics of a render buffer
*/
template < class SemanticClass, class SemanticEnum>
class RenderBufferData
{
public:
	PX_INLINE RenderBufferData(void)
	{
		m_numCustomSemantics = 0;
		m_customSemanticData = 0;
	}

	/**
	\brief Returns the properties of a base semantic
	*/
	PX_INLINE const RenderSemanticData& getSemanticData(SemanticEnum semantic) const
	{
		PX_ASSERT(semantic < SemanticClass::NUM_SEMANTICS);
		return m_semanticData[semantic];
	}

	/**
	\brief Returns the number of custom semantics
	*/
	PX_INLINE uint32_t getNumCustomSemantics() const
	{
		return m_numCustomSemantics;
	}

	/**
	\brief Returns the properties of a custom semantic
	*/
	PX_INLINE const RenderSemanticData& getCustomSemanticData(uint32_t index) const
	{
		PX_ASSERT(index < m_numCustomSemantics);
		return m_customSemanticData[index];
	}

	/**
	\brief Set a specific set of custom semantics on this data buffer.

	If there are named custom semantics in a RenderMeshAsset, its RenderMeshActor will
	call this function to set those semantics on this buffer.
	*/
	PX_INLINE void setCustomSemanticData(RenderSemanticData* data, uint32_t num)
	{
		m_numCustomSemantics = num;
		m_customSemanticData = data;
	}

	/**
	\brief Set the properties (pointer and stride) of a base semantic.
	*/
	PX_INLINE void setSemanticData(SemanticEnum semantic, const void* data, uint32_t stride, RenderDataFormat::Enum format, RenderDataFormat::Enum srcFormat = RenderDataFormat::UNSPECIFIED)
	{
		PX_ASSERT(semantic < SemanticClass::NUM_SEMANTICS);
		RenderSemanticData& sd = m_semanticData[semantic];
		sd.data   = data;
		sd.stride = stride;
		sd.format = format;
		sd.srcFormat = srcFormat == RenderDataFormat::UNSPECIFIED ? format : srcFormat;
	}

private:
	RenderSemanticData  m_semanticData[SemanticClass::NUM_SEMANTICS]; //!< Base semantics for this buffer type
	uint32_t			  m_numCustomSemantics;							//!< Number of custom semantics
	RenderSemanticData* m_customSemanticData;							//!< An array of custom semantics
};

/**
\brief Describes data semantics provided by a particular module
*/
class ModuleSpecificRenderBufferData
{
public:
	AuthObjTypeID           moduleId;						//!< Unique module identifier, from Module::getModuleID()
	RenderSemanticData* 	moduleSpecificSemanticData;   //!< Pointer to an array of custom semantics
	uint32_t              	numModuleSpecificSemantics;   //!< Count of custom semantics provided by this module

public:
	PX_INLINE ModuleSpecificRenderBufferData(void)
	{
		moduleId                   = 0;
		moduleSpecificSemanticData = 0;
		numModuleSpecificSemantics = 0;
	}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // RENDER_BUFFER_DATA_H
