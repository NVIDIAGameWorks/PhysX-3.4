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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "ScbShape.h"

using namespace physx;

bool Scb::Shape::setMaterialsHelper(PxMaterial* const* materials, PxU16 materialCount)
{
	PX_ASSERT(!isBuffering());

	if(materialCount == 1)
	{
		PxU16 materialIndex = Ps::to16((static_cast<NpMaterial*>(materials[0]))->getHandle());

		mShape.setMaterialIndices(&materialIndex, 1);
	}
	else
	{
		PX_ASSERT(materialCount > 1);

		PX_ALLOCA(materialIndices, PxU16, materialCount);

		if(materialIndices)
		{
			NpMaterial::getMaterialIndices(materials, materialIndices, materialCount);
			mShape.setMaterialIndices(materialIndices, materialCount);
		}
		else
		{
			Ps::getFoundation().error(PxErrorCode::eOUT_OF_MEMORY, __FILE__, __LINE__, 
				"PxShape::setMaterials() failed. Out of memory. Call will be ignored.");
			return false;
		}
	}

	Scb::Scene* sc = getScbScene();
	if(sc)
		sc->getScScene().notifyNphaseOnUpdateShapeMaterial(mShape);

	return true;
}

void Scb::Shape::syncState()
{
	const PxU32 flags = getBufferFlags();
	if(flags)
	{
		const PxShapeFlags oldShapeFlags = mShape.getFlags();

		const Scb::ShapeBuffer&	buffer = *getBufferedData();

		Scb::Scene* scbScene = getScbScene();	// PT: can be NULL. See e.g. RbShapeTest.ReleaseShapeWithPendingUpdate UT.

		if(flags & Buf::BF_Geometry)
		{
			if(scbScene)
				scbScene->getScScene().unregisterShapeFromNphase(mShape);

			mShape.setGeometry(buffer.geometry.getGeometry());

			if(scbScene)
				scbScene->getScScene().registerShapeInNphase(mShape);

#if PX_SUPPORT_PVD
			if(getControlState() == ControlState::eIN_SCENE)
			{
				PX_ASSERT(scbScene);
				scbScene->getScenePvdClient().releaseAndRecreateGeometry(this);
			}
#endif
		}

		if(flags & Buf::BF_Material)
		{
			// PT: not sure if this is correct. Added the check for PX-800 but "getMaterialBuffer" doesn't always need the scene pointer...
			if(scbScene)
			{
				const PxU16* materialIndices = getMaterialBuffer(*scbScene, buffer);
				mShape.setMaterialIndices(materialIndices, buffer.materialCount);
				scbScene->getScScene().notifyNphaseOnUpdateShapeMaterial(mShape);
			}
			UPDATE_PVD_MATERIALS()
			// TODO: So far we did not bother to fail gracefully in the case of running out of memory. If that should change then this
			// method is somewhat problematic. The material ref counters have been adjusted at the time when the public API was called.
			// Could be that one of the old materials was deleted afterwards. The problem now is what to do if this method fails?
			// We can't adjust the material ref counts any longer since some of the old materials might have been deleted.
			// One solution could be that this class allocates an array of material pointers when the buffered method is called.
			// This array is then passed into the core object and is used by the core object, i.e., the core object does not allocate the
			// buffer itself.
		}

		flush<Buf::BF_Shape2Actor>(buffer);
		flush<Buf::BF_SimulationFilterData>(buffer);
		flush<Buf::BF_ContactOffset>(buffer);
		flush<Buf::BF_RestOffset>(buffer);
		flush<Buf::BF_Flags>(buffer);

		Sc::RigidCore* scRigidCore = NpShapeGetScRigidObjectFromScbSLOW(*this);

		if(scRigidCore) // may be NULL for exclusive shapes because of pending shape updates after buffered release of actor.
			scRigidCore->onShapeChange(mShape, Sc::ShapeChangeNotifyFlags(flags), oldShapeFlags, true);
	}

	postSyncState();
}
