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

#include "pvd/PxPvdTransport.h"

#include "PxPvdDefaultSocketTransport.h"

namespace physx
{
namespace pvdsdk
{
PvdDefaultSocketTransport::PvdDefaultSocketTransport(const char* host, int port, unsigned int timeoutInMilliseconds)
: mHost(host), mPort(uint16_t(port)), mTimeout(timeoutInMilliseconds), mConnected(false), mWrittenData(0)
{
}

PvdDefaultSocketTransport::~PvdDefaultSocketTransport()
{
}

bool PvdDefaultSocketTransport::connect()
{
	if(mConnected)
		return true;

	if(mSocket.connect(mHost, mPort, mTimeout))
	{
		mSocket.setBlocking(true);
		mConnected = true;
	}
	return mConnected;
}

void PvdDefaultSocketTransport::disconnect()
{
	mSocket.flush();
	mSocket.disconnect();
	mConnected = false;
}

bool PvdDefaultSocketTransport::isConnected()
{
	return mSocket.isConnected();
}

bool PvdDefaultSocketTransport::write(const uint8_t* inBytes, uint32_t inLength)
{
	if(mConnected)
	{
		if(inLength == 0)
			return true;

		uint32_t amountWritten = 0;
		uint32_t totalWritten = 0;
		do
		{
			// Sockets don't have to write as much as requested, so we need
			// to wrap this call in a do/while loop.
			// If they don't write any bytes then we consider them disconnected.
			amountWritten = mSocket.write(inBytes, inLength);
			inLength -= amountWritten;
			inBytes += amountWritten;
			totalWritten += amountWritten;
		} while(inLength && amountWritten);

		if(amountWritten == 0)
			return false;

		mWrittenData += totalWritten;

		return true;
	}
	else
		return false;
}

PxPvdTransport& PvdDefaultSocketTransport::lock()
{
	mMutex.lock();
	return *this;
}

void PvdDefaultSocketTransport::unlock()
{
	mMutex.unlock();
}

void PvdDefaultSocketTransport::flush()
{
	mSocket.flush();
}

uint64_t PvdDefaultSocketTransport::getWrittenDataSize()
{
	return mWrittenData;
}

void PvdDefaultSocketTransport::release()
{
	PX_DELETE(this);
}

} // namespace pvdsdk

PxPvdTransport* PxDefaultPvdSocketTransportCreate(const char* host, int port, unsigned int timeoutInMilliseconds)
{
	return PX_NEW(pvdsdk::PvdDefaultSocketTransport)(host, port, timeoutInMilliseconds);
}

} // namespace physx
