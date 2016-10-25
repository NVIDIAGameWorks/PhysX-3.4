#ifndef PS_COMM_STREAM_H

#define PS_COMM_STREAM_H

#include "PsCommLayer.h"

class CommStream : public CommLayer
{
public:
	virtual bool isValid(void) const = 0;
	virtual CommLayer *getCommLayer(void) = 0;

protected:
	virtual ~CommStream(void)
	{

	}
};

CommStream *createCommStream(const char *streamFile,
							bool recordFile,
							CommLayer *c);

#endif
