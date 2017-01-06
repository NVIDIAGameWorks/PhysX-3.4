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


#ifndef PSFILEBUFFER_PXFILEBUF_H
#define PSFILEBUFFER_PXFILEBUF_H

/** \addtogroup foundation
  @{
*/

#if !PX_DOXYGEN
namespace physx
{

namespace general_PxIOStream2
{
#endif

PX_PUSH_PACK_DEFAULT

/**
\brief Callback class for data serialization.

The user needs to supply an PxFileBuf implementation to a number of methods to allow the SDK to read or write
chunks of binary data. This allows flexibility for the source/destination of the data. For example the PxFileBuf
could store data in a file, memory buffer or custom file format.

\note It is the users responsibility to ensure that the data is written to the appropriate offset.

*/
class PxFileBuf
{
public:

	enum EndianMode
	{
		ENDIAN_NONE		= 0, // do no conversion for endian mode
		ENDIAN_BIG		= 1, // always read/write data as natively big endian (Power PC, etc.)
		ENDIAN_LITTLE	= 2 // always read/write data as natively little endian (Intel, etc.) Default Behavior!
	};

	PxFileBuf(EndianMode mode=ENDIAN_LITTLE)
	{
		setEndianMode(mode);
	}

	virtual ~PxFileBuf(void)
	{

	}

	/**
	\brief Declares a constant to seek to the end of the stream.
	*
	* Does not support streams longer than 32 bits
	*/
	static const uint32_t STREAM_SEEK_END=0xFFFFFFFF;

	enum OpenMode
	{
		OPEN_FILE_NOT_FOUND,
		OPEN_READ_ONLY,					// open file buffer stream for read only access
		OPEN_WRITE_ONLY,				// open file buffer stream for write only access
		OPEN_READ_WRITE_NEW,			// open a new file for both read/write access
		OPEN_READ_WRITE_EXISTING		// open an existing file for both read/write access
	};

	virtual OpenMode	getOpenMode(void) const  = 0;

	bool isOpen(void) const
	{
		return getOpenMode()!=OPEN_FILE_NOT_FOUND;
	}

	enum SeekType
	{
		SEEKABLE_NO			= 0,
		SEEKABLE_READ		= 0x1,
		SEEKABLE_WRITE		= 0x2,
		SEEKABLE_READWRITE 	= 0x3
	};

	virtual SeekType isSeekable(void) const = 0;

	void	setEndianMode(EndianMode e)
	{
		mEndianMode = e;
		if ( (e==ENDIAN_BIG && !isBigEndian() ) ||
			 (e==ENDIAN_LITTLE && isBigEndian() ) )
		{
			mEndianSwap = true;
		}
 		else
		{
			mEndianSwap = false;
		}
	}

	EndianMode	getEndianMode(void) const
	{
		return mEndianMode;
	}

	virtual uint32_t getFileLength(void) const = 0;

	/**
	\brief Seeks the stream to a particular location for reading
	*
	* If the location passed exceeds the length of the stream, then it will seek to the end.
	* Returns the location it ended up at (useful if you seek to the end) to get the file position
	*/
	virtual uint32_t	seekRead(uint32_t loc) = 0;

	/**
	\brief Seeks the stream to a particular location for writing
	*
	* If the location passed exceeds the length of the stream, then it will seek to the end.
	* Returns the location it ended up at (useful if you seek to the end) to get the file position
	*/
	virtual uint32_t	seekWrite(uint32_t loc) = 0;

	/**
	\brief Reads from the stream into a buffer.

	\param[out] mem  The buffer to read the stream into.
	\param[in]  len  The number of bytes to stream into the buffer

	\return Returns the actual number of bytes read.  If not equal to the length requested, then reached end of stream.
	*/
	virtual uint32_t	read(void *mem,uint32_t len) = 0;


	/**
	\brief Reads from the stream into a buffer but does not advance the read location.

	\param[out] mem  The buffer to read the stream into.
	\param[in]  len  The number of bytes to stream into the buffer

	\return Returns the actual number of bytes read.  If not equal to the length requested, then reached end of stream.
	*/
	virtual uint32_t	peek(void *mem,uint32_t len) = 0;

	/**
	\brief Writes a buffer of memory to the stream

	\param[in] mem The address of a buffer of memory to send to the stream.
	\param[in] len  The number of bytes to send to the stream.

	\return Returns the actual number of bytes sent to the stream.  If not equal to the length specific, then the stream is full or unable to write for some reason.
	*/
	virtual uint32_t	write(const void *mem,uint32_t len) = 0;

	/**
	\brief Reports the current stream location read aqccess.

	\return Returns the current stream read location.
	*/
	virtual uint32_t	tellRead(void) const = 0;

	/**
	\brief Reports the current stream location for write access.

	\return Returns the current stream write location.
	*/
	virtual uint32_t	tellWrite(void) const = 0;

	/**
	\brief	Causes any temporarily cached data to be flushed to the stream.
	*/
	virtual	void	flush(void) = 0;

	/**
	\brief	Close the stream.
	*/
	virtual void close(void) {}

	void release(void)
	{
		delete this;
	}

    static PX_INLINE bool isBigEndian()
     {
       int32_t i = 1;
        return *(reinterpret_cast<char*>(&i))==0;
    }

    PX_INLINE void swap2Bytes(void* _data) const
    {
		char *data = static_cast<char *>(_data);
		char one_byte;
		one_byte = data[0]; data[0] = data[1]; data[1] = one_byte;
    }

    PX_INLINE void swap4Bytes(void* _data) const
    {
		char *data = static_cast<char *>(_data);
		char one_byte;
		one_byte = data[0]; data[0] = data[3]; data[3] = one_byte;
		one_byte = data[1]; data[1] = data[2]; data[2] = one_byte;
    }

    PX_INLINE void swap8Bytes(void *_data) const
    {
		char *data = static_cast<char *>(_data);
		char one_byte;
		one_byte = data[0]; data[0] = data[7]; data[7] = one_byte;
		one_byte = data[1]; data[1] = data[6]; data[6] = one_byte;
		one_byte = data[2]; data[2] = data[5]; data[5] = one_byte;
		one_byte = data[3]; data[3] = data[4]; data[4] = one_byte;
    }


	PX_INLINE void storeDword(uint32_t v)
	{
		if ( mEndianSwap )
		    swap4Bytes(&v);

		write(&v,sizeof(v));
	}

	PX_INLINE void storeFloat(float v)
	{
		if ( mEndianSwap )
			swap4Bytes(&v);
		write(&v,sizeof(v));
	}

	PX_INLINE void storeDouble(double v)
	{
		if ( mEndianSwap )
			swap8Bytes(&v);
		write(&v,sizeof(v));
	}

	PX_INLINE  void storeByte(uint8_t b)
	{
		write(&b,sizeof(b));
	}

	PX_INLINE void storeWord(uint16_t w)
	{
		if ( mEndianSwap )
			swap2Bytes(&w);
		write(&w,sizeof(w));
	}

	uint8_t readByte(void) 
	{
		uint8_t v=0;
		read(&v,sizeof(v));
		return v;
	}

	uint16_t readWord(void) 
	{
		uint16_t v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap2Bytes(&v);
		return v;
	}

	uint32_t readDword(void) 
	{
		uint32_t v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap4Bytes(&v);
		return v;
	}

	float readFloat(void) 
	{
		float v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap4Bytes(&v);
		return v;
	}

	double readDouble(void) 
	{
		double v=0;
		read(&v,sizeof(v));
		if ( mEndianSwap )
		    swap8Bytes(&v);
		return v;
	}

private:
	bool		mEndianSwap;	// whether or not the endian should be swapped on the current platform
	EndianMode	mEndianMode;  	// the current endian mode behavior for the stream
};

PX_POP_PACK

#if !PX_DOXYGEN
} // end of namespace

using namespace general_PxIOStream2;

namespace general_PxIOStream = general_PxIOStream2;

} // end of namespace
#endif

/** @} */

#endif // PSFILEBUFFER_PXFILEBUF_H
