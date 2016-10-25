#ifndef EXT_CLIENT_SERVER_H

#define EXT_CLIENT_SERVER_H

// This class is used to handle inter-process communication between a render debug client and the render debug server.
// The server must be already running for a client to 'connect'.
//

#include "RenderDebug.h"
#include "RenderDebugImpl.h"
#include "ProcessRenderDebug.h"

class ClientServer
{
public:
	virtual uint32_t getCommunicationsFrame(void) const = 0;
	virtual bool isConnected(void) = 0;
	virtual bool sendCommand(const char *cmd) = 0;
	virtual const char **getCommand(uint32_t &argc) = 0;

	virtual void sendInputEvent(const RENDER_DEBUG::InputEvent &ev) = 0;
	virtual const RENDER_DEBUG::InputEvent *getInputEvent(bool flush) = 0;

	// Wait this many milliseconds for the server to catch up..
	virtual bool serverWait(void) = 0;
	virtual void processFrame(RENDER_DEBUG::ProcessRenderDebug *processRenderDebug,RENDER_DEBUG::RenderDebugInterface *iface) = 0;
	virtual void recordPrimitives(uint32_t frameCount,uint32_t primType,uint32_t primCount,uint32_t dataLength,const void *data) = 0;
	virtual bool isServer(void) const = 0;
	virtual void finalizeFrame(uint32_t frameCount) = 0;
	virtual const char * getRemoteApplicationName(void) = 0;


	/**
	\brief Transmits an arbitrary block of binary data to the remote machine.  The block of data can have a command and id associated with it.

	It is important to note that due to the fact the RenderDebug system is synchronized every single frame, it is strongly recommended
	that you only use this feature for relatively small data items; probably on the order of a few megabytes at most.  If you try to do
	a very large transfer, in theory it would work, but it might take a very long time to complete and look like a hang since it will
	essentially be blocking.

	\param command An arbitrary command associated with this data transfer, for example this could indicate a remote file request.
	\param id An arbitrary id associated with this data transfer, for example the id could be the file name of a file transfer request.
	\param data The block of binary data to transmit, you are responsible for maintaining endian correctness of the internal data if necessary.
	\param dlen The length of the lock of data to transmit.

	\return Returns true if the data was queued to be transmitted, false if it failed.
	*/
	virtual bool sendRemoteResource(const char *command,
								const char *id,
								const void *data,
								uint32_t dlen) = 0;

	/**
	\brief This function allows you to request a file from the remote machine by name.  If successful it will be returned via 'getRemoteData'

	\param command The command field associated with this request which will be returned by 'getRemoteData'
	\param fileName The filename being requested from the remote machine.

	\return Returns true if the request was queued to be transmitted, false if it failed.
	*/
	virtual bool requestRemoteResource(const char *command,
									const char *fileName) = 0;

	/**
	\brief Retrieves a block of remotely transmitted binary data.

	\param command A a reference to a pointer which will store the arbitrary command associated with this data transfer, for example this could indicate a remote file request.
	\param id A reference to a pointer which will store an arbitrary id associated with this data transfer, for example the id could be the file name of a file transfer request.
	\param dlen A reference that will contain length of the lock of data received.
	\param remoteIsBigEndian A reference to a boolean which will be set to true if the remote machine that sent this data is big endian format.

	\retrun A pointer to the block of data received.
	*/
	virtual const void * getRemoteResource(const char *&command,
										const char *&id,
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

	virtual void release(void) = 0;
protected:
	virtual ~ClientServer(void)
	{
	}
};

ClientServer *createClientServer(RENDER_DEBUG::RenderDebug::Desc &desc);

#endif
