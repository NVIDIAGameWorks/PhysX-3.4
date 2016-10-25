

#ifndef __PVRT_H__
#define __PVRT_H__

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <assert.h>

#include <CoreFoundation/CFByteOrder.h>

#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

static const char* gPVRTexIdentifier = "PVR!";

enum
{
	kPVRTextureFlagTypePVRTC_2 = 24,
	kPVRTextureFlagTypePVRTC_4
};

struct PVRTexHeader
{
	uint32_t headerLength;
	uint32_t height;
	uint32_t width;
	uint32_t numMipmaps;
	uint32_t flags;
	uint32_t dataLength;
	uint32_t bpp;
	uint32_t bitmaskRed;
	uint32_t bitmaskGreen;
	uint32_t bitmaskBlue;
	uint32_t bitmaskAlpha;
	uint32_t pvrTag;
	uint32_t numSurfs;
};

struct PVRTextureInfo
{
	PVRTextureInfo(const char* buffer) : 
		data(NULL),
		width(0),
		height(0),
		glFormat(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG),
		hasAlpha(false)
	{
		const PVRTexHeader* header = reinterpret_cast<const PVRTexHeader*>(buffer);
		uint32_t pvrTag = CFSwapInt32LittleToHost(header->pvrTag);
		
		if (gPVRTexIdentifier[0] != ((pvrTag >>  0) & 0xff) ||
			gPVRTexIdentifier[1] != ((pvrTag >>  8) & 0xff) ||
			gPVRTexIdentifier[2] != ((pvrTag >> 16) & 0xff) ||
			gPVRTexIdentifier[3] != ((pvrTag >> 24) & 0xff))
		{
			return;
		}
		
		uint32_t flags = CFSwapInt32LittleToHost(header->flags);
		uint32_t formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;
		
		if (formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2)
		{			
			if (formatFlags == kPVRTextureFlagTypePVRTC_4)
				glFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			else if (formatFlags == kPVRTextureFlagTypePVRTC_2)
				glFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			
			width = CFSwapInt32LittleToHost(header->width);
			height = CFSwapInt32LittleToHost(header->height);
			
			hasAlpha = CFSwapInt32LittleToHost(header->bitmaskAlpha) == true;
			mipCount = CFSwapInt32LittleToHost(header->numMipmaps);
			data = buffer + sizeof(PVRTexHeader);
		}
	}
	
	const char* data;
	uint32_t width;
	uint32_t height;
	uint32_t mipCount;
	GLenum glFormat;
	bool hasAlpha;
};

#endif
