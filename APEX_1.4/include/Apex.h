/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_H
#define APEX_H

/**
\file
\brief The top level include file for all of the APEX API.

Include this whenever you want to use anything from the APEX API
in a source file.
*/

#include "foundation/Px.h"

#include "ApexUsingNamespace.h"


namespace NvParameterized
{
class Traits;
class Interface;
class Serializer;
};


#include "foundation/PxPreprocessor.h"
#include "foundation/PxSimpleTypes.h"
#include "foundation/PxAssert.h"
#include "foundation/PxBounds3.h"
#include "foundation/PxVec2.h"
#include "foundation/PxVec3.h"


// APEX public API:
// In general, APEX public headers will not be included 'alone', so they
// should not include their prerequisites.

#include "ApexDefs.h"
#include "ApexDesc.h"
#include "ApexInterface.h"
#include "ApexSDK.h"

#include "Actor.h"
#include "Context.h"
#include "ApexNameSpace.h"
#include "PhysXObjectDesc.h"
#include "RenderDataProvider.h"
#include "Renderable.h"
#include "AssetPreview.h"
#include "Asset.h"
#include "RenderContext.h"
#include "Scene.h"
#include "ApexSDKCachedData.h"
#include "IProgressListener.h"
#include "Module.h"
#include "IosAsset.h"

#include "RenderDataFormat.h"
#include "RenderBufferData.h"
#include "UserRenderResourceManager.h"
#include "UserRenderVertexBufferDesc.h"
#include "UserRenderInstanceBufferDesc.h"
#include "UserRenderSpriteBufferDesc.h"
#include "UserRenderIndexBufferDesc.h"
#include "UserRenderBoneBufferDesc.h"
#include "UserRenderResourceDesc.h"
#include "UserRenderSurfaceBufferDesc.h"
#include "UserRenderSurfaceBuffer.h"
#include "UserRenderResource.h"
#include "UserRenderVertexBuffer.h"
#include "UserRenderInstanceBuffer.h"
#include "UserRenderSpriteBuffer.h"
#include "UserRenderIndexBuffer.h"
#include "UserRenderBoneBuffer.h"
#include "UserRenderer.h"

#include "VertexFormat.h"
#include "RenderMesh.h"
#include "RenderMeshActorDesc.h"
#include "RenderMeshActor.h"
#include "RenderMeshAsset.h"
#include "ResourceCallback.h"
#include "ResourceProvider.h"

#endif // APEX_H
