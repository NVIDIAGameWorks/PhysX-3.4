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

#ifndef PXPVDSDK_PXPVDTRANSPORT_H
#define PXPVDSDK_PXPVDTRANSPORT_H

/** \addtogroup pvd
@{
*/
#include "foundation/PxErrors.h"
#include "foundation/PxFlags.h"
#include "pvd/PxPvd.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief	PxPvdTransport is an interface representing the data transport mechanism.
This class defines all services associated with the transport: configuration, connection, reading, writing etc.
It is owned by the application, and can be realized as a file or a socket (using one-line PxDefault<...> methods in
PhysXExtensions) or in a custom implementation. This is a class that is intended for use by PVD, not by the
application, the application entry points are PxPvd and PvdClient.
*/

class PxPvdTransport
{
  public:
	// connect, isConnected, disconnect, read, write, flush

	/**
	Connects to the Visual Debugger application.
	return True if success
	*/
	virtual bool connect() = 0;

	/**
	Disconnects from the Visual Debugger application.
	If we are still connected, this will kill the entire debugger connection.
	*/
	virtual void disconnect() = 0;

	/**
	 *	Return if connection to PVD is created.
	 */
	virtual bool isConnected() = 0;

	/**
	 *	write bytes to the other endpoint of the connection. should lock before witre. If an error occurs
	 *	this connection will assume to be dead.
	 */
	virtual bool write(const uint8_t* inBytes, uint32_t inLength) = 0;

	/*
	    lock this transport and return it
	*/
	virtual PxPvdTransport& lock() = 0;

	/*
	    unlock this transport
	*/
	virtual void unlock() = 0;

	/**
	 *	send any data and block until we know it is at least on the wire.
	 */
	virtual void flush() = 0;

	/**
	 *	Return size of written data.
	 */
	virtual uint64_t getWrittenDataSize() = 0;

	virtual void release() = 0;

  protected:
	virtual ~PxPvdTransport()
	{
	}
};

/**
	\brief Create a default socket transport.
	\param host host address of the pvd application.
	\param port ip port used for pvd, should same as the port setting in pvd application.
	\param timeoutInMilliseconds timeout when connect to pvd host.
*/
PX_PVDSDK_API PxPvdTransport* PX_CALL_CONV
PxDefaultPvdSocketTransportCreate(const char* host, int port, unsigned int timeoutInMilliseconds);

/**
	\brief Create a default file transport.
	\param name full path filename used save captured pvd data, or NULL for a fake/test file transport.
*/
PX_PVDSDK_API PxPvdTransport* PX_CALL_CONV PxDefaultPvdFileTransportCreate(const char* name);

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif // PXPVDSDK_PXPVDTRANSPORT_H
