#ifndef PS_COMM_LAYER_H

#define PS_COMM_LAYER_H

// Simple class to create a client/server connection and pass messages back and forth over TCP/IP
// Supports compression

#include <stdint.h>

#define COMM_LAYER_DEFAULT_HOST "localhost"
#define COMM_LAYER_DEFAULT_PORT 5525

class CommLayer
{
public:

	virtual bool isServer(void) const = 0; // return true if we are in server mode.
	virtual bool hasClient(void) const = 0;	// return true if a client connection is currently established
	virtual bool isConnected(void) const = 0; // return true if we are still connected to the server.  The server is always in a 'connected' state.
	virtual int32_t getPendingReadSize(void) const = 0; // returns the number of bytes of data which is pending to be read.
	virtual int32_t getPendingSendSize(void) const = 0; // return the number of bytes of data pending to be sent.  This can be used for flow control

	virtual bool sendMessage(const void *msg,uint32_t len) = 0;

	virtual uint32_t peekMessage(bool &isBigEndianPacket) = 0; // return the length of the next pending message

	virtual uint32_t getMessage(void *msg,uint32_t maxLength,bool &isBigEndianPacket) = 0; // receives a pending message

	virtual void release(void) = 0;

protected:
	virtual ~CommLayer(void)
	{
	}
};

CommLayer *createCommLayer(const char *ipaddress,
						  uint16_t portNumber,
						  bool isServer);

#endif
