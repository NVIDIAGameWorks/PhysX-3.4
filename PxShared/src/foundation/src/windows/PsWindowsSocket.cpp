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

#include "foundation/PxIntrinsics.h"

#include "windows/PsWindowsInclude.h"
#include "PsSocket.h"
#include "PsThread.h"
#include "PsArray.h"

#include <Winsock2.h>
#pragma comment(lib, "Ws2_32")

namespace physx
{
namespace shdfnd
{

const uint32_t Socket::DEFAULT_BUFFER_SIZE = 32768;

class SocketImpl
{
  public:
	SocketImpl(bool isBlocking);
	virtual ~SocketImpl();

	bool connect(const char* host, uint16_t port, uint32_t timeout);
	bool listen(uint16_t port);
	bool accept(bool block);
	void disconnect();

	void setBlocking(bool blocking);

	virtual uint32_t write(const uint8_t* data, uint32_t length);
	virtual bool flush();
	uint32_t read(uint8_t* data, uint32_t length);

	PX_FORCE_INLINE bool isBlocking() const
	{
		return mIsBlocking;
	}
	PX_FORCE_INLINE bool isConnected() const
	{
		return mIsConnected;
	}
	PX_FORCE_INLINE const char* getHost() const
	{
		return mHost;
	}
	PX_FORCE_INLINE uint16_t getPort() const
	{
		return mPort;
	}

  protected:
	bool nonBlockingTimeout() const;
	void setBlockingInternal(SOCKET socket, bool blocking);

	mutable SOCKET mSocket;
	SOCKET mListenSocket;
	const char* mHost;
	uint16_t mPort;
	mutable bool mIsConnected;
	bool mIsBlocking;
	bool mListenMode;
	bool mSocketLayerIntialized;
};

SocketImpl::SocketImpl(bool isBlocking)
: mSocket(INVALID_SOCKET)
, mListenSocket(INVALID_SOCKET)
, mHost(NULL)
, mPort(0)
, mIsConnected(false)
, mIsBlocking(isBlocking)
, mListenMode(false)
, mSocketLayerIntialized(false)
{
	WORD vreq;
	WSADATA wsaData;
	vreq = MAKEWORD(2, 2);
	mSocketLayerIntialized = (WSAStartup(vreq, &wsaData) == 0);
}

SocketImpl::~SocketImpl()
{
	if(mSocketLayerIntialized)
		WSACleanup();
}

void SocketImpl::setBlockingInternal(SOCKET socket, bool blocking)
{
	uint32_t mode = uint32_t(blocking ? 0 : 1);
	ioctlsocket(socket, FIONBIO, (u_long*)&mode);
}

bool SocketImpl::connect(const char* host, uint16_t port, uint32_t timeout)
{
	if(!mSocketLayerIntialized)
		return false;

	sockaddr_in socketAddress;
	hostent* hp;

	intrinsics::memSet(&socketAddress, 0, sizeof(sockaddr_in));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);

	// get host
	hp = gethostbyname(host);
	if(!hp)
	{
		in_addr a;
		a.s_addr = inet_addr(host);
		hp = gethostbyaddr((const char*)&a, sizeof(in_addr), AF_INET);
		if(!hp)
			return false;
	}
	intrinsics::memCopy(&socketAddress.sin_addr, hp->h_addr_list[0], (uint32_t)hp->h_length);

	// connect
	mSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(mSocket == INVALID_SOCKET)
		return false;

	setBlockingInternal(mSocket, false);

	::connect(mSocket, (sockaddr*)&socketAddress, sizeof(socketAddress));
	// Setup select function call to monitor the connect call.
	fd_set writefs;
	fd_set exceptfs;
	FD_ZERO(&writefs);
	FD_ZERO(&exceptfs);
#pragma warning(push)
#pragma warning(disable : 4127 4548)
	FD_SET(mSocket, &writefs);
	FD_SET(mSocket, &exceptfs);
#pragma warning(pop)
	timeval timeout_;
	timeout_.tv_sec = long(timeout / 1000);
	timeout_.tv_usec = long(((timeout % 1000) * 1000));
	int selret = ::select(1, NULL, &writefs, &exceptfs, &timeout_);
	int excepted = FD_ISSET(mSocket, &exceptfs);
	int canWrite = FD_ISSET(mSocket, &writefs);
	if(selret != 1 || excepted || !canWrite)
	{
		disconnect();
		return false;
	}

	setBlockingInternal(mSocket, mIsBlocking);

	mIsConnected = true;
	mPort = port;
	mHost = host;
	return true;
}

bool SocketImpl::listen(uint16_t port)
{
	if(!mSocketLayerIntialized)
		return false;

	mListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(mListenSocket == INVALID_SOCKET)
		return false;

	mListenMode = true;

	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	return bind(mListenSocket, (sockaddr*)&addr, sizeof(addr)) == 0 && ::listen(mListenSocket, SOMAXCONN) == 0;
}

bool SocketImpl::accept(bool block)
{
	if(mIsConnected || !mListenMode)
		return false;

	// set the listen socket to be non-blocking.
	setBlockingInternal(mListenSocket, block);
	SOCKET clientSocket = ::accept(mListenSocket, 0, 0);
	if(clientSocket == INVALID_SOCKET)
		return false;

	mSocket = clientSocket;
	mIsConnected = true;
	setBlockingInternal(mSocket, mIsBlocking); // force the mode to whatever the user set

	return mIsConnected;
}

void SocketImpl::disconnect()
{
	if(mListenSocket != INVALID_SOCKET)
	{
		closesocket(mListenSocket);
		mListenSocket = INVALID_SOCKET;
	}
	if(mSocket != INVALID_SOCKET)
	{
		WSASendDisconnect(mSocket, NULL);
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
	mIsConnected = false;
	mListenMode = false;
	mPort = 0;
	mHost = NULL;
}

bool SocketImpl::nonBlockingTimeout() const
{
	return !mIsBlocking && WSAGetLastError() == WSAEWOULDBLOCK;
}

// should be cross-platform from here down

void SocketImpl::setBlocking(bool blocking)
{
	if(blocking != mIsBlocking)
	{
		mIsBlocking = blocking;
		if(isConnected())
			setBlockingInternal(mSocket, blocking);
	}
}

bool SocketImpl::flush()
{
	return true;
}

uint32_t SocketImpl::write(const uint8_t* data, uint32_t length)
{
	if(length == 0)
		return 0;

	int sent = send(mSocket, (const char*)data, (int32_t)length, 0);

	if(sent <= 0 && !nonBlockingTimeout())
		disconnect();

	return uint32_t(sent > 0 ? sent : 0);
}

uint32_t SocketImpl::read(uint8_t* data, uint32_t length)
{
	if(length == 0)
		return 0;

	int32_t received = recv(mSocket, (char*)data, (int32_t)length, 0);

	if(received <= 0 && !nonBlockingTimeout())
		disconnect();

	return uint32_t(received > 0 ? received : 0);
}

class BufferedSocketImpl : public SocketImpl
{
  public:
	BufferedSocketImpl(bool isBlocking) : SocketImpl(isBlocking), mBufferPos(0)
	{
	}
	virtual ~BufferedSocketImpl()
	{
	}
	bool flush();
	uint32_t write(const uint8_t* data, uint32_t length);

  private:
	uint32_t mBufferPos;
	uint8_t mBuffer[Socket::DEFAULT_BUFFER_SIZE];
};

bool BufferedSocketImpl::flush()
{
	uint32_t totalBytesWritten = 0;

	while(totalBytesWritten < mBufferPos && mIsConnected)
		totalBytesWritten += (int32_t)SocketImpl::write(mBuffer + totalBytesWritten, mBufferPos - totalBytesWritten);

	bool ret = (totalBytesWritten == mBufferPos);
	mBufferPos = 0;
	return ret;
}

uint32_t BufferedSocketImpl::write(const uint8_t* data, uint32_t length)
{
	uint32_t bytesWritten = 0;
	while(mBufferPos + length >= Socket::DEFAULT_BUFFER_SIZE)
	{
		uint32_t currentChunk = Socket::DEFAULT_BUFFER_SIZE - mBufferPos;
		intrinsics::memCopy(mBuffer + mBufferPos, data + bytesWritten, currentChunk);
		bytesWritten += (uint32_t)currentChunk; // for the user, this is consumed even if we fail to shove it down a
		// non-blocking socket

		uint32_t sent = SocketImpl::write(mBuffer, Socket::DEFAULT_BUFFER_SIZE);
		mBufferPos = Socket::DEFAULT_BUFFER_SIZE - sent;

		if(sent < Socket::DEFAULT_BUFFER_SIZE) // non-blocking or error
		{
			if(sent) // we can reasonably hope this is rare
				intrinsics::memMove(mBuffer, mBuffer + sent, mBufferPos);

			return bytesWritten;
		}
		length -= currentChunk;
	}

	if(length > 0)
	{
		intrinsics::memCopy(mBuffer + mBufferPos, data + bytesWritten, length);
		bytesWritten += length;
		mBufferPos += length;
	}

	return bytesWritten;
}

Socket::Socket(bool inIsBuffering, bool isBlocking)
{
	if(inIsBuffering)
	{
		void* mem = PX_ALLOC(sizeof(BufferedSocketImpl), "BufferedSocketImpl");
		mImpl = PX_PLACEMENT_NEW(mem, BufferedSocketImpl)(isBlocking);
	}
	else
	{
		void* mem = PX_ALLOC(sizeof(SocketImpl), "SocketImpl");
		mImpl = PX_PLACEMENT_NEW(mem, SocketImpl)(isBlocking);
	}
}

Socket::~Socket()
{
	mImpl->flush();
	mImpl->disconnect();
	mImpl->~SocketImpl();
	PX_FREE(mImpl);
}

bool Socket::connect(const char* host, uint16_t port, uint32_t timeout)
{
	return mImpl->connect(host, port, timeout);
}

bool Socket::listen(uint16_t port)
{
	return mImpl->listen(port);
}

bool Socket::accept(bool block)
{
	return mImpl->accept(block);
}

void Socket::disconnect()
{
	mImpl->disconnect();
}

bool Socket::isConnected() const
{
	return mImpl->isConnected();
}

const char* Socket::getHost() const
{
	return mImpl->getHost();
}

uint16_t Socket::getPort() const
{
	return mImpl->getPort();
}

bool Socket::flush()
{
	if(!mImpl->isConnected())
		return false;
	return mImpl->flush();
}

uint32_t Socket::write(const uint8_t* data, uint32_t length)
{
	if(!mImpl->isConnected())
		return 0;
	return mImpl->write(data, length);
}

uint32_t Socket::read(uint8_t* data, uint32_t length)
{
	if(!mImpl->isConnected())
		return 0;
	return mImpl->read(data, length);
}

void Socket::setBlocking(bool blocking)
{
	mImpl->setBlocking(blocking);
}

bool Socket::isBlocking() const
{
	return mImpl->isBlocking();
}

} // namespace shdfnd
} // namespace physx
