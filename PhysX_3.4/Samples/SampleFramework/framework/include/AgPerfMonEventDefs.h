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


// This file is used to define a list of AgPerfMon events.
//
// This file is included exclusively by AgPerfMonEventSrcAPI.h
// and by AgPerfMonEventSrcAPI.cpp, for the purpose of building
// an enumeration (enum xx) and an array of strings ()
// that contain the list of events.
//
// This file should only contain event definitions, using the
// DEFINE_EVENT macro.  E.g.:
//
//     DEFINE_EVENT(sample_name_1)
//     DEFINE_EVENT(sample_name_2)
//     DEFINE_EVENT(sample_name_3)


// Statistics from the fluid mesh packet cooker
DEFINE_EVENT(renderFunction)
DEFINE_EVENT(SampleRendererVBwriteBuffer)
DEFINE_EVENT(SampleOnTickPreRender)
DEFINE_EVENT(SampleOnTickPostRender)
DEFINE_EVENT(SampleOnRender)
DEFINE_EVENT(SampleOnDraw)
DEFINE_EVENT(D3D9Renderer_createVertexBuffer)
DEFINE_EVENT(Renderer_render)
DEFINE_EVENT(Renderer_render_depthOnly)
DEFINE_EVENT(Renderer_render_deferred)
DEFINE_EVENT(Renderer_render_lit)
DEFINE_EVENT(Renderer_render_unlit)
DEFINE_EVENT(Renderer_renderMeshes)
DEFINE_EVENT(Renderer_renderDeferredLights)
DEFINE_EVENT(D3D9RendererMesh_renderIndices)
DEFINE_EVENT(D3D9RendererMesh_renderVertices)
DEFINE_EVENT(D3D9Renderer_createIndexBuffer)
DEFINE_EVENT(D3D9RendererMesh_renderVerticesInstanced)
DEFINE_EVENT(D3D9Renderer_createInstanceBuffer)
DEFINE_EVENT(D3D9Renderer_createTexture2D)
DEFINE_EVENT(D3D9Renderer_createTarget)
DEFINE_EVENT(D3D9Renderer_createMaterial)
DEFINE_EVENT(D3D9Renderer_createMesh)
DEFINE_EVENT(D3D9Renderer_createLight)
DEFINE_EVENT(D3D9RendererMesh_renderIndicesInstanced)
DEFINE_EVENT(OGLRenderer_createVertexBuffer)
DEFINE_EVENT(OGLRenderer_createIndexBuffer)
DEFINE_EVENT(OGLRenderer_createInstanceBuffer)
DEFINE_EVENT(OGLRenderer_createTexture2D)
DEFINE_EVENT(OGLRenderer_createTarget)
DEFINE_EVENT(OGLRenderer_createMaterial)
DEFINE_EVENT(OGLRenderer_createMesh)
DEFINE_EVENT(OGLRenderer_createLight)
DEFINE_EVENT(OGLRendererMaterial_compile_vertexProgram)
DEFINE_EVENT(OGLRendererMaterial_load_vertexProgram)
DEFINE_EVENT(OGLRendererMaterial_compile_fragmentProgram)
DEFINE_EVENT(OGLRendererMaterial_load_fragmentProgram)
DEFINE_EVENT(OGLRendererVertexBufferBind)
DEFINE_EVENT(OGLRendererSwapBuffers)
DEFINE_EVENT(writeBufferSemanticStride)
DEFINE_EVENT(writeBufferfixUV)
DEFINE_EVENT(writeBufferConvertFromApex)
DEFINE_EVENT(writeBufferGetFormatSemantic)
DEFINE_EVENT(writeBufferlockSemantic)
DEFINE_EVENT(OGLRendererVertexBufferLock)
DEFINE_EVENT(Renderer_meshRenderLast)
DEFINE_EVENT(Renderer_atEnd)
DEFINE_EVENT(renderMeshesBindMeshContext)
DEFINE_EVENT(renderMeshesFirstIf)
DEFINE_EVENT(renderMeshesSecondIf)
DEFINE_EVENT(renderMeshesThirdIf)
DEFINE_EVENT(renderMeshesForthIf)
DEFINE_EVENT(OGLRendererBindMeshContext)
DEFINE_EVENT(OGLRendererBindMeshcg)
DEFINE_EVENT(cgGLSetMatrixParameter)
DEFINE_EVENT(D3D9RenderVBlock)
DEFINE_EVENT(D3D9RenderVBunlock)
DEFINE_EVENT(D3D9RenderIBlock)
DEFINE_EVENT(D3D9RenderIBunlock)
