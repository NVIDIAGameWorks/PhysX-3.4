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


#ifndef GLES2_RENDERER_MATERIAL_H
#define GLES2_RENDERER_MATERIAL_H

#include <RendererConfig.h>

#if defined(RENDERER_ENABLE_GLES2)

#include <RendererMaterial.h>
#include "GLES2Renderer.h"

#if defined(RENDERER_ANDROID)
#include "android/AndroidSamplePlatform.h"
#elif defined(RENDERER_IOS)
#include "ios/IosSamplePlatform.h"
#endif

#include <set>

#include <set>

namespace SampleRenderer
{

class GLES2RendererMaterial : public RendererMaterial
{
	public:
		GLES2RendererMaterial(GLES2Renderer &renderer, const RendererMaterialDesc &desc);
		virtual ~GLES2RendererMaterial(void);
		virtual void setModelMatrix(const float *matrix) 
		{
			PX_UNUSED(matrix);
			PX_ALWAYS_ASSERT();
		}
		virtual const Renderer& getRenderer() const { return m_renderer; }
		
		/* Actually executes glUniform* and submits data saved in the m_program[m_currentPass].vsUniformsVec4/psUniformsVec4 */
		void submitUniforms();

	private:
		virtual void bind(RendererMaterial::Pass pass, RendererMaterialInstance *materialInstance, bool instanced) const;
		virtual void bindMeshState(bool instanced) const;
		virtual void unbind(void) const;
		virtual void bindVariable(Pass pass, const Variable &variable, const void *data) const;
    
        VariableType 	GLTypetoVariableType(GLenum type);
		GLint			getAttribLocation(size_t usage, size_t index, Pass pass);
		std::string 	mojoSampleNameToOriginal(Pass pass, const std::string& name);
		
	private:
		GLES2RendererMaterial &operator=(const GLES2RendererMaterial&) { return *this; }
		
	private:
        void loadCustomConstants(Pass pass);

        friend class GLES2Renderer;
        friend class GLES2RendererVertexBuffer;
        friend class GLES2RendererDirectionalLight;
        GLES2Renderer &m_renderer;
		
		GLenum       m_glAlphaTestFunc;

		mutable struct shaderProgram {
			GLuint      vertexShader;
			GLuint      fragmentShader;
			GLuint      program;

			GLint       modelMatrixUniform;
			GLint       viewMatrixUniform;
			GLint       projMatrixUniform;
			GLint       modelViewMatrixUniform;
			GLint       boneMatricesUniform;
			GLint		modelViewProjMatrixUniform;
		
			GLint       positionAttr;
			GLint       colorAttr;
			GLint       normalAttr;
			GLint       tangentAttr;
			GLint       boneIndexAttr;
			GLint       boneWeightAttr;

			GLint       texcoordAttr[8];
			
			mutable std::set<std::string>	vsUniformsCollected;
			mutable std::set<std::string>	psUniformsCollected;
			
			size_t		vsUniformsTotal;
			size_t		psUniformsTotal;
			
			char*		vsUniformsVec4;
			GLuint		vsUniformsVec4Location;
			size_t		vsUniformsVec4Size;
			size_t		vsUniformsVec4SizeInBytes;
			char*		psUniformsVec4;
			GLuint		psUniformsVec4Location;
			size_t		psUniformsVec4Size;
			size_t		psUniformsVec4SizeInBytes;
			
			mojoResult* vertexMojoResult;
			mojoResult* fragmentMojoResult;
			
			shaderProgram();
			~shaderProgram();
		}	m_program[NUM_PASSES];

		mutable Pass	m_currentPass;
};

}

#endif // #if defined(RENDERER_ENABLE_GLES2)
#endif
