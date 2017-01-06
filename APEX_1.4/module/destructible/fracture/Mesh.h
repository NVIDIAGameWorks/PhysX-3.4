/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#ifndef MESH
#define MESH

#include <PxVec3.h>
#include <PsArray.h>
#include <PsAllocator.h>

#include "MeshBase.h"

namespace nvidia
{
namespace apex
{
	class RenderMeshAsset;
}
}

namespace nvidia
{
namespace fracture
{

using namespace ::nvidia::shdfnd;

class Mesh : public base::Mesh
{
public:
	void loadFromRenderMesh(const apex::RenderMeshAsset& mesh, uint32_t partIndex);
protected:
	static void gatherPartMesh(Array<PxVec3>& vertices, Array<uint32_t>&  indices, Array<PxVec3>& normals,
		Array<PxVec2>& texcoords, nvidia::Array<SubMesh>& subMeshes, const apex::RenderMeshAsset& renderMeshAsset, uint32_t partIndex);
};

}
}

#endif
#endif