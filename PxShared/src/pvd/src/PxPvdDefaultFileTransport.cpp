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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "pvd/PxPvdTransport.h"
#include "foundation/PxAssert.h"

#include "PxPvdDefaultFileTransport.h"

namespace physx
{
namespace pvdsdk
{

PvdDefaultFileTransport::PvdDefaultFileTransport(const char* name) : mConnected(false), mWrittenData(0), mLocked(false)
{
	mFileBuffer = PX_NEW(PsFileBuffer)(name, PxFileBuf::OPEN_WRITE_ONLY);
}

PvdDefaultFileTransport::~PvdDefaultFileTransport()
{
}

bool PvdDefaultFileTransport::connect()
{
	PX_ASSERT(mFileBuffer);
	mConnected = mFileBuffer->isOpen();
	return mConnected;
}

void PvdDefaultFileTransport::disconnect()
{
	mConnected = false;
}

bool PvdDefaultFileTransport::isConnected()
{
	return mConnected;
}

bool PvdDefaultFileTransport::write(const uint8_t* inBytes, uint32_t inLength)
{
	PX_ASSERT(mLocked);
	PX_ASSERT(mFileBuffer);
	if (mConnected)
	{
		uint32_t len = mFileBuffer->write(inBytes, inLength);
		mWrittenData += len;
		return len == inLength;
	}
	else
		return false;
}

PxPvdTransport& PvdDefaultFileTransport::lock()
{
	mMutex.lock();
	PX_ASSERT(!mLocked);
	mLocked = true;
	return *this;
}

void PvdDefaultFileTransport::unlock()
{
	PX_ASSERT(mLocked);
	mLocked = false;
	mMutex.unlock();
}

void PvdDefaultFileTransport::flush()
{
}

uint64_t PvdDefaultFileTransport::getWrittenDataSize()
{
	return mWrittenData;
}

void PvdDefaultFileTransport::release()
{
	if (mFileBuffer)
	{
		mFileBuffer->close();
		delete mFileBuffer;
	}
	mFileBuffer = NULL;
	PX_DELETE(this);
}




class NullFileTransport : public physx::PxPvdTransport, public physx::shdfnd::UserAllocated
{
	PX_NOCOPY(NullFileTransport)
  public:
	NullFileTransport();
	virtual ~NullFileTransport();

	virtual bool connect();
	virtual void disconnect();
	virtual bool isConnected();

	virtual bool write(const uint8_t* inBytes, uint32_t inLength);

	virtual PxPvdTransport& lock();
	virtual void unlock();

	virtual void flush();

	virtual uint64_t getWrittenDataSize();

	virtual void release();

  private:
	bool mConnected;
	uint64_t mWrittenData;
	physx::shdfnd::Mutex mMutex;
	bool mLocked; // for debug, remove it when finished
};

NullFileTransport::NullFileTransport() : mConnected(false), mWrittenData(0), mLocked(false)
{
}

NullFileTransport::~NullFileTransport()
{
}

bool NullFileTransport::connect()
{
	mConnected = true;
	return true;
}

void NullFileTransport::disconnect()
{
	mConnected = false;
}

bool NullFileTransport::isConnected()
{
	return mConnected;
}

bool NullFileTransport::write(const uint8_t* /*inBytes*/, uint32_t inLength)
{
	PX_ASSERT(mLocked);
	if(mConnected)
	{
		uint32_t len = inLength;
		mWrittenData += len;
		return len == inLength;
	}
	else
		return false;
}

PxPvdTransport& NullFileTransport::lock()
{
	mMutex.lock();
	PX_ASSERT(!mLocked);
	mLocked = true;
	return *this;
}

void NullFileTransport::unlock()
{
	PX_ASSERT(mLocked);
	mLocked = false;
	mMutex.unlock();
}

void NullFileTransport::flush()
{
}

uint64_t NullFileTransport::getWrittenDataSize()
{
	return mWrittenData;
}

void NullFileTransport::release()
{
	PX_DELETE(this);
}

} // namespace pvdsdk

PxPvdTransport* PxDefaultPvdFileTransportCreate(const char* name)
{
	if(name)
		return PX_NEW(pvdsdk::PvdDefaultFileTransport)(name);
	else
		return PX_NEW(pvdsdk::NullFileTransport)();
}

} // namespace physx

