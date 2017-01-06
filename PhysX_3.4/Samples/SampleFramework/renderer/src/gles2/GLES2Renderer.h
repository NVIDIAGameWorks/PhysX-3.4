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

#ifndef GLES2_RENDERER_H
#define GLES2_RENDERER_H

#include <RendererConfig.h>

#if defined(RENDERER_ENABLE_GLES2)

#if defined(RENDERER_ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

extern PFNGLMAPBUFFEROESPROC   glMapBufferOES;
extern PFNGLUNMAPBUFFEROESPROC glUnmapBufferOES;

#elif defined(RENDERER_IOS)

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#endif

#define GL_TEXTURE0_ARB GL_TEXTURE0
#define glBindBufferARB glBindBuffer
#define glBufferDataARB glBufferData
#define GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
#define GL_ARRAY_BUFFER_ARB GL_ARRAY_BUFFER
#define GL_STATIC_DRAW_ARB GL_STATIC_DRAW
#define GL_DYNAMIC_DRAW_ARB GL_DYNAMIC_DRAW
#define glGenBuffersARB glGenBuffers
#define glDeleteBuffersARB glDeleteBuffers
#define GL_CLAMP GL_CLAMP_TO_EDGE
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES
#define GL_READ_WRITE GL_WRITE_ONLY_OES

extern bool GLEW_ARB_vertex_buffer_object;

#define glActiveTextureARB glActiveTexture
#define glClientActiveTextureARB(a)

/* GL_EXT_texture_compression_dxt1 */
#ifndef GL_EXT_texture_compression_dxt1
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif

/* GL_EXT_texture_compression_s3tc */
#ifndef GL_EXT_texture_compression_s3tc
/* GL_COMPRESSED_RGB_S3TC_DXT1_EXT defined in GL_EXT_texture_compression_dxt1 already. */
/* GL_COMPRESSED_RGBA_S3TC_DXT1_EXT defined in GL_EXT_texture_compression_dxt1 already. */
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif

#define GL_RGBA8 GL_RGBA
#define GL_BGRA8 GL_BGRA

#include <Renderer.h>

namespace SampleFramework {
	class SamplePlatform;
}

namespace SampleRenderer
{

class RendererVertexBuffer;
class RendererIndexBuffer;
class RendererMaterial;

void PxToGL(GLfloat *gl44, const physx::PxMat44 &mat);
void PxToGLColumnMajor(GLfloat *gl44, const physx::PxMat44 &mat);
void RenToGL(GLfloat *gl44, const RendererProjection &proj);
void RenToGLColumnMajor(GLfloat *gl44, const RendererProjection &proj);

class GLES2Renderer : public Renderer
{
	public:
		GLES2Renderer(const RendererDesc &desc, const char* assetDir);
		virtual ~GLES2Renderer(void);

	private:
		bool begin(void);
		void end(void);
		void checkResize(void);
	
	public:
		// clears the offscreen buffers.
		virtual void clearBuffers(void);
		
		// presents the current color buffer to the screen.
		virtual bool swapBuffers(void);
		// get the device pointer (void * abstraction)
		virtual void *getDevice(); 

		virtual bool captureScreen(const char*);
		virtual bool captureScreen(PxU32 &width, PxU32& height, PxU32& sizeInBytes, const void*& screenshotData);

		// get the window size
		void getWindowSize(PxU32 &width, PxU32 &height) const;		
		// copy common renderer variables to the material (like g_MVP, g_modelMatrix, etc)
		virtual void setCommonRendererParameters();

		virtual RendererVertexBuffer   *createVertexBuffer(  const RendererVertexBufferDesc   &desc);
		virtual RendererIndexBuffer    *createIndexBuffer(   const RendererIndexBufferDesc    &desc);
		virtual RendererInstanceBuffer *createInstanceBuffer(const RendererInstanceBufferDesc &desc);
		virtual RendererTexture2D      *createTexture2D(     const RendererTexture2DDesc      &desc);
		virtual RendererTexture3D      *createTexture3D(     const RendererTexture3DDesc      &desc);
		virtual RendererTarget         *createTarget(        const RendererTargetDesc         &desc);
		virtual RendererMaterial       *createMaterial(      const RendererMaterialDesc       &desc);
		virtual RendererMesh           *createMesh(          const RendererMeshDesc           &desc);
		virtual RendererLight          *createLight(         const RendererLightDesc          &desc);

		void 							finalizeTextRender();
		void  							finishRendering();
		virtual void                    setVsync(bool on);	
	private:
		virtual void bindViewProj(const physx::PxMat44 &inveye, const RendererProjection &proj);
		virtual void bindFogState(const RendererColor &fogColor, float fogDistance);
		virtual void bindAmbientState(const RendererColor &ambientColor);
		virtual void bindDeferredState(void);
		virtual void bindMeshContext(const RendererMeshContext &context);
		virtual void beginMultiPass(void);
		virtual void endMultiPass(void);
		virtual void beginTransparentMultiPass(void);
		virtual void endTransparentMultiPass(void);
		virtual void renderDeferredLight(const RendererLight &light);
		virtual PxU32 convertColor(const RendererColor& color) const;
		
		virtual bool isOk(void) const;
		
		virtual	bool initTexter();
		virtual	void setupTextRenderStates();
		virtual	void resetTextRenderStates();
		virtual	void renderTextBuffer(const void* vertices, PxU32 nbVerts, const PxU16* indices, PxU32 nbIndices, SampleRenderer::RendererMaterial* material);
		virtual	void renderLines2D(const void* vertices, PxU32 nbVerts);
		virtual	void setupScreenquadRenderStates();
		virtual	void resetScreenquadRenderStates();
	private:
		SampleFramework::SamplePlatform*				m_platform;
		RendererVertexBuffer*							m_textVertexBuffer;
		RendererIndexBuffer*							m_textIndexBuffer;
        PxU32                                           m_textVertexBufferOffset; 
        PxU32                                           m_textIndexBufferOffset;
        RendererMaterial*                               m_textMaterial;
		RendererMesh*									m_textMesh;
		PxU32         m_displayWidth;
		PxU32         m_displayHeight;
		
		physx::PxMat44 m_viewMatrix;
		GLfloat		  m_glProjectionMatrix[16], m_glProjectionMatrixC[16];
		PxVec3		  m_eyePosition;
		PxVec3		  m_eyeDirection;
		GLfloat		  m_ambientColor[3];
		GLfloat		  m_lightColor[3];
		GLfloat		  m_intensity;
		GLfloat		  m_lightDirection[3];
		GLfloat		  m_fogColorAndDistance[4];
};
}
#endif // #if defined(RENDERER_ENABLE_GLES2)
#endif
