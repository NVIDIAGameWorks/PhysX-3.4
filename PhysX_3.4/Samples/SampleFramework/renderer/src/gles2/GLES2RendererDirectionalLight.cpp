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


#include "GLES2RendererDirectionalLight.h"
#include "GLES2RendererMaterial.h"
#include "GLES2Renderer.h"
#include "RendererMaterialInstance.h"

#if defined(RENDERER_ENABLE_GLES2)
namespace SampleRenderer
{

GLES2RendererDirectionalLight::GLES2RendererDirectionalLight(const RendererDirectionalLightDesc &desc, 
															GLES2Renderer &renderer, GLfloat (&_lightColor)[3], 
															GLfloat& _lightIntensity, GLfloat (&_lightDirection)[3]) :
	RendererDirectionalLight(desc), m_lightColor(_lightColor), m_lightIntensity(_lightIntensity), m_lightDirection(_lightDirection)
{
}

GLES2RendererDirectionalLight::~GLES2RendererDirectionalLight(void)
{
}

extern GLES2RendererMaterial* g_hackCurrentMat;
extern RendererMaterialInstance* g_hackCurrentMatInstance;

void GLES2RendererDirectionalLight::bind(void) const
{
	m_lightColor[0] = m_color.r/255.0f;
	m_lightColor[1] = m_color.g/255.0f;
	m_lightColor[2] = m_color.b/255.0f;
	m_lightIntensity = m_intensity;
	m_lightDirection[0] = m_direction.x;
	m_lightDirection[1] = m_direction.y;
	m_lightDirection[2] = m_direction.z;
}

}

#endif // #if defined(RENDERER_ENABLE_GLES2)
