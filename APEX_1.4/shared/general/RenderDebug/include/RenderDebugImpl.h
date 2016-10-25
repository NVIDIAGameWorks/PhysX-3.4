/*
 * Copyright 2009-2011 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.  Users and possessors of this source code
 * are hereby granted a nonexclusive, royalty-free license to use this code
 * in individual and commercial software.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION,  ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.   This source code is a "commercial item" as
 * that term is defined at  48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer  software"  and "commercial computer software
 * documentation" as such terms are  used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 *
 * Any use of this source code in individual and commercial software must
 * include, in the user documentation and internal comments to the code,
 * the above Disclaimer and U.S. Government End Users Notice.
 */

#ifndef RENDER_DEBUG_IMPL_H
#define RENDER_DEBUG_IMPL_H

/*!
\file
\brief debug rendering classes and structures
*/
#include "RenderDebugData.h"
#include "RenderDebugImpl.h"
#include "RenderDebugTyped.h"


namespace RENDER_DEBUG
{

PX_PUSH_PACK_DEFAULT


class RenderDebugHook
{
public:

	virtual bool render(float dtime,RENDER_DEBUG::RenderDebugInterface *iface) = 0;

	/**
	\brief Begins a file-playback session. Returns the number of recorded frames in the recording file.  Zero if the file was not valid.
	*/
	virtual uint32_t setFilePlayback(const char *fileName) = 0; 

	/**
	\brief Set's the file playback to a specific frame.  Returns true if successful.
	*/
	virtual bool setPlaybackFrame(uint32_t playbackFrame) = 0;

	/**
	\brief Returns the number of recorded frames in the debug render recording file.
	*/
	virtual uint32_t getPlaybackFrameCount(void) const = 0;

	/**
	\brief Stops the current recording playback.
	*/
	virtual void stopPlayback(void) = 0;

	/**
	\brief Do a 'try' lock on the global render debug mutex.  This is simply provided as an optional convenience if you are accessing RenderDebug from multiple threads and want to prevent contention.
	*/
	virtual bool trylock(void) = 0;

	/**
	\brief Lock the global render-debug mutex to avoid thread contention.
	*/
	virtual void lock(void) = 0; 

	/**
	\brief Unlock the global render-debug mutex
	*/
	virtual void unlock(void) = 0;

	/**
	\brief Convenience method to return a unique mesh id number (simply a global counter to avoid clashing with other ids
	*/
	virtual uint32_t getMeshId(void) = 0;

	/**
	\brief Send a command from the server to the client.  This could be any arbitrary console command, it can also be mouse drag events, debug visualization events, etc.
	* the client receives this command in argc/argv format.
	*/
	virtual bool sendRemoteCommand(const char *fmt,...) = 0;

	/**
	\brief If running in client mode, poll this method to retrieve any pending commands from the server.  If it returns NULL then there are no more commands.
	*/
	virtual const char ** getRemoteCommand(uint32_t &argc) = 0;

	/**
	\brief Transmit an actual input event to the remote client

	\param ev The input event data to transmit
	*/
	virtual void sendInputEvent(const InputEvent &ev) = 0;

	/**
	\brief Returns any incoming input event for processing purposes
	*/
	virtual const InputEvent *getInputEvent(bool flush) = 0;

	/**
	\brief Report what 'Run' mode we are operation gin.
	*/
	virtual RenderDebug::RunMode getRunMode(void) = 0;

	/**
	\brief Returns true if we still have a valid connection to the server.
	*/
	virtual bool isConnected(void) const = 0;

	/**
	\brief Returns the current synchronized frame between client/server communications.  Returns zero if no active connection exists.
	*/
	virtual uint32_t getCommunicationsFrame(void) const = 0;

	virtual const char *getRemoteApplicationName(void) = 0;

	/**
	\brief Returns the optional typed methods for various render debug routines.
	*/
	virtual RenderDebugTyped *getRenderDebugTyped(void) = 0;

	/**
	\brief Transmits an arbitrary block of binary data to the remote machine.  The block of data can have a command and id associated with it.

	It is important to note that due to the fact the RenderDebug system is synchronized every single frame, it is strongly recommended
	that you only use this feature for relatively small data items; probably on the order of a few megabytes at most.  If you try to do
	a very large transfer, in theory it would work, but it might take a very long time to complete and look like a hang since it will
	essentially be blocking.

	\param nameSpace An arbitrary command associated with this data transfer, for example this could indicate a remote file request.
	\param resourceName An arbitrary id associated with this data transfer, for example the id could be the file name of a file transfer request.
	\param data The block of binary data to transmit, you are responsible for maintaining endian correctness of the internal data if necessary.
	\param dlen The length of the lock of data to transmit.

	\return Returns true if the data was queued to be transmitted, false if it failed.
	*/
	virtual bool sendRemoteResource(const char *nameSpace,
									const char *resourceName,
									const void *data,
									uint32_t dlen) = 0;


	/**
	\brief This function allows you to request a file from the remote machine by name.  If successful it will be returned via 'getRemoteData'

	\param nameSpace The command field associated with this request which will be returned by 'getRemoteData'
	\param resourceName The filename being requested from the remote machine.

	\return Returns true if the request was queued to be transmitted, false if it failed.
	*/
	virtual bool requestRemoteResource(const char *nameSpace,
									const char *resourceName) = 0;



	/**
	\brief Retrieves a block of remotely transmitted binary data.

	\param nameSpace A a reference to a pointer which will store the namespace (type) associated with this data transfer, for example this could indicate a remote file request.
	\param resourceName A reference to a pointer which will store the resource name associated with this data transfer, for example the resource name could be the file name of a file transfer request.
	\param dlen A reference that will contain length of the lock of data received.
	\param remoteIsBigEndian A reference to a boolean which will be set to true if the remote machine that sent this data is big endian format.

	\retrun A pointer to the block of data received.
	*/
	virtual const void * getRemoteResource(const char *&nameSpace,
										const char *&resourceName,
										uint32_t &dlen,
										bool &remoteIsBigEndian) = 0;

	/**
	\brief Set the base file name to record communications tream; or NULL to disable it.

	\param fileName The base name of the file to record the communications channel stream to, or NULL to disable it.
	*/
	virtual bool setStreamFilename(const char *fileName) = 0;

	/**
	\brief Begin playing back a communications stream recording

	\param fileName The name of the previously captured communications stream file
	*/
	virtual bool setStreamPlayback(const char *fileName) = 0;

	/**
	\brief Release the render debug class
	*/
	virtual void release(void) = 0;
protected:
	virtual ~RenderDebugHook(void) {}
};

/**
\brief class that draws debug rendering primitives
 */
class RenderDebugImpl : public RenderDebugTyped
{
public:
	virtual bool renderImpl(float dtime,RENDER_DEBUG::RenderDebugInterface *iface) = 0;
	virtual void releaseRenderDebug(void) = 0;
protected:
	virtual ~RenderDebugImpl(void) { }
};


PX_POP_PACK

} // end of namespace

#endif // RENDER_DEBUG_IMPL_H
