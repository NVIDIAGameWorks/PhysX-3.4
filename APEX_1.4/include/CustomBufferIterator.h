/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CUSTOM_BUFFER_ITERATOR_H
#define CUSTOM_BUFFER_ITERATOR_H

/*!
\file
\brief class CustomBufferIterator
*/

#include "RenderMesh.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief This class is used to access specific elements in an untyped chunk of memory
*/
class CustomBufferIterator
{
public:
	/**
	\brief Returns the memory start of a specific vertex.

	All custom buffers are stored interleaved, so this is also the memory start of the first attribute of this vertex.
	*/
	virtual void*		getVertex(uint32_t triangleIndex, uint32_t vertexIndex) const = 0;

	/**
	\brief Returns the index of a certain custom buffer.

	\note This is constant throughout the existence of this class.
	*/
	virtual int32_t		getAttributeIndex(const char* attributeName) const = 0;

	/**
	\brief Returns a pointer to a certain attribute of the specified vertex/triangle.

	\param [in] triangleIndex Which triangle
	\param [in] vertexIndex Which of the vertices of this triangle (must be either 0, 1 or 2)
	\param [in] attributeName The name of the attribute you wish the data for
	\param [out] outFormat The format of the attribute, reinterpret_cast the void pointer accordingly.
	*/
	virtual void*		getVertexAttribute(uint32_t triangleIndex, uint32_t vertexIndex, const char* attributeName, RenderDataFormat::Enum& outFormat) const = 0;

	/**
	\brief Returns a pointer to a certain attribute of the specified vertex/triangle.

	\note This is the faster method than the one above since it won't do any string comparisons

	\param [in] triangleIndex Which triangle
	\param [in] vertexIndex Which of the vertices of this triangle (must be either 0, 1 or 2)
	\param [in] attributeIndex The indexof the attribute you wish the data for (use CustomBufferIterator::getAttributeIndex to find the index to a certain attribute name
	\param [out] outFormat The format of the attribute, reinterpret_cast the void pointer accordingly.
	\param [out] outName The name associated with the attribute
	*/
	virtual void*		getVertexAttribute(uint32_t triangleIndex, uint32_t vertexIndex, uint32_t attributeIndex, RenderDataFormat::Enum& outFormat, const char*& outName) const = 0;

protected:
	CustomBufferIterator() {}
	virtual				~CustomBufferIterator() {}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // CUSTOM_BUFFER_ITERATOR_H
