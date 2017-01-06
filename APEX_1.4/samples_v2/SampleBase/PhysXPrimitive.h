/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef PHYSX_PRIMITIVE_H
#define PHYSX_PRIMITIVE_H

#include "Utils.h"
#include "PxPhysicsAPI.h"
#include <DirectXMath.h>

using namespace physx;

class ApexRenderMaterial;
class Mesh;

class PhysXPrimitive
{
  public:
	PhysXPrimitive(PxRigidActor* actor, PxVec3 scale);
	~PhysXPrimitive();

	PxRigidActor* getActor()
	{
		return mActor;
	}

	void setColor(DirectX::XMFLOAT3 color)
	{
		mColor = color;
	}
	DirectX::XMFLOAT3 getColor()
	{
		return mColor;
	}

	PxMat44 getModelMatrix();

	void render(ID3D11DeviceContext& context);

  private:
	DirectX::XMFLOAT3 mColor;
	PxRigidActor* mActor;
	Mesh* mMesh;
	PxVec3 mScale;
};

#endif