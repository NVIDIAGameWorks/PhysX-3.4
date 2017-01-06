/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_VERTEX_FORMAT_H__
#define __APEX_VERTEX_FORMAT_H__


#include "VertexFormat.h"
#include "PsUserAllocated.h"
#include "PsArray.h"
#include "VertexFormatParameters.h"
#include "ApexSharedUtils.h"

namespace nvidia
{
namespace apex
{


class ApexVertexFormat : public VertexFormat, public UserAllocated
{
public:
	ApexVertexFormat();
	ApexVertexFormat(VertexFormatParameters* params);
	~ApexVertexFormat();
	explicit ApexVertexFormat(const ApexVertexFormat& f);

	// VertexFormat methods

	/** \brief Resets the format to the initial state */
	virtual void							reset();


	/** \brief Sets the winding (cull mode) for this format */
	virtual void							setWinding(RenderCullMode::Enum winding);

	/** \brief Sets whether or not a separate bone buffer is used */
	virtual void							setHasSeparateBoneBuffer(bool hasSeparateBoneBuffer);

	/** \brief Accessor to read winding (cull mode) */
	virtual RenderCullMode::Enum			getWinding() const;

	/** \brief Accessor to read if a seperate vertex buffer for bone indices and weights is generated */
	virtual bool							hasSeparateBoneBuffer() const;

	/** \brief Returns a buffer name for a semantic. Returns NULL if the semantic is invalid. */
	virtual const char*						getSemanticName(RenderVertexSemantic::Enum semantic) const;

	/** \brief Returns a buffer ID for a semantic. For custom buffers, use the getID() function. */
	virtual BufferID						getSemanticID(RenderVertexSemantic::Enum semantic) const;

	/** \brief Returns a buffer ID for a named buffer. For standard semantics, the getSemanticID( semantic ) function is faster, but
	is equivalent to getID( getSemanticName( semantic ) ). Returns 0 if name == NULL */
	virtual BufferID						getID(const char* name) const;


	/** \brief Adds a vertex buffer channel to this format
	\param [in] name the name of a new buffer (use getSemanticName for standard semantics)
	\return The buffer index. If the buffer for the semantic already exists, the index of the existing buffer is returned. Returns -1 if there is an error (e.g. name == NULL).
	*/
	virtual int32_t					addBuffer(const char* name);

	/** \brief Removes a buffer
	\param [in] index the buffer to remove
	\return True if successful, false otherwise (if the buffer index was invalid)
	*/
	virtual bool							bufferReplaceWithLast(uint32_t index);


	/** \brief Set the format for a buffer
	\return True if successful, false otherwise (if the buffer index was invalid)
	*/
	virtual bool							setBufferFormat(uint32_t index, RenderDataFormat::Enum format);

	/** \brief Set the access type for a buffer (static, dynamic, etc.)
	\return True if successful, false otherwise (if the buffer index was invalid)
	*/
	virtual bool							setBufferAccess(uint32_t index, RenderDataAccess::Enum access);

	/** \brief Set whether or not the buffer should be serialized
	\return True if successful, false otherwise (if the buffer index was invalid)
	*/
	virtual bool							setBufferSerialize(uint32_t index, bool serialize);


	/** \brief Accessor to read the name of a given buffer
	\return The buffer name if successful, NULL otherwise.
	*/
	virtual const char*						getBufferName(uint32_t index) const;

	/** \brief Accessor to read the semantic of a given buffer
	\return The buffer semantic if successful, RenderVertexSemantic::NUM_SEMANTICS otherwise.
	*/
	virtual RenderVertexSemantic::Enum	getBufferSemantic(uint32_t index) const;

	/** \brief Accessor to read the ID of a given buffer
	\return The buffer semantic if successful, 0 otherwise.
	*/
	virtual BufferID						getBufferID(uint32_t index) const;

	/** \brief Get the format for a buffer
	\return The buffer format if successful, RenderDataFormat::UNSPECIFIED otherwise.
	*/
	virtual RenderDataFormat::Enum		getBufferFormat(uint32_t index) const;

	/** \brief Get the access type for a buffer (static, dynamic, etc.)
	\return The buffer access if successful, RenderDataAccess::ACCESS_TYPE_COUNT otherwise.
	*/
	virtual RenderDataAccess::Enum		getBufferAccess(uint32_t index) const;

	/** \brief Get whether or not the buffer should be serialized
	\return Whether or not the buffer should be serialized if successful, false otherwise.
	*/
	virtual bool							getBufferSerialize(uint32_t index) const;


	/** \brief Accessor to read the number of buffers */
	virtual uint32_t					getBufferCount() const;

	/** \brief Returns the number of buffers that are user-specified */
	virtual uint32_t					getCustomBufferCount() const;

	/** \brief Accessor to get the buffer index
	If the buffer is not found, -1 is returned
	*/
	virtual int32_t					getBufferIndexFromID(BufferID id) const;

	// ApexVertexFormat internal methods

	bool								operator == (const VertexFormat& format) const;
	bool								operator != (const VertexFormat& format) const
	{
		return !(*this == format);
	}

	void								copy(const ApexVertexFormat& other);

private:
	void								clearBuffers();

	ApexVertexFormat&					operator = (const ApexVertexFormat&)
	{
		return *this;    // No assignment
	}

	struct CustomBuffer
	{
		char*						name;
		RenderDataFormat::Enum	format;
		bool						serialize;
	};

	VertexFormatParameters*	mParams;
	bool					mOwnsParams;

	friend class ApexVertexBuffer;
};

}
} // end namespace nvidia::apex


#endif // __APEX_VERTEX_FORMAT_H__
