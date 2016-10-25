#ifndef RENDER_TARGET
#define RENDER_TARGET

#include "FrameBufferObject.h"

// -------------------------------------------------------------------------------------------
class RenderTarget 
{
public:
	RenderTarget(int width, int height);
	~RenderTarget();

	void resize(int width, int height);

	void beginCapture();
	void endCapture();

	GLuint getColorTexId() { return mColorTexId; }
	GLuint getDepthTexId() { return mDepthTexId; }

private:
	void clear();
	GLuint createTexture(GLenum target, int width, int height, GLint internalFormat, GLenum format);

	GLuint mColorTexId;
	GLuint mDepthTexId;

	FrameBufferObject *mFBO;
};

#endif