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
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef NP_MATERIALMANAGER
#define NP_MATERIALMANAGER

#include "foundation/PxMemory.h"
#include "NpMaterial.h"
#include "CmIDPool.h"

namespace physx
{
	class NpMaterialManager 
	{
	public:
		NpMaterialManager()
		{
			const PxU32 matCount = 128;
			mMaterials = reinterpret_cast<NpMaterial**>(PX_ALLOC(sizeof(NpMaterial*) * matCount,  "NpMaterialManager::initialise"));
			mMaxMaterials = matCount;
			PxMemZero(mMaterials, sizeof(NpMaterial*)*mMaxMaterials);
		}

		~NpMaterialManager() {}

		void releaseMaterials()
		{
			for(PxU32 i=0; i<mMaxMaterials; ++i)
			{
				if(mMaterials[i])
				{
					const PxU32 handle = mMaterials[i]->getHandle();
					mHandleManager.freeID(handle);
					mMaterials[i]->release();
					mMaterials[i] = NULL;
				}
			}
			PX_FREE(mMaterials);
		}

		bool setMaterial(NpMaterial& mat)
		{
			const PxU32 materialIndex = mHandleManager.getNewID();

			if(materialIndex >= mMaxMaterials)
				resize();

			mMaterials[materialIndex] = &mat;
			mat.setHandle(materialIndex);
			return true;
		}

		void updateMaterial(NpMaterial& mat)
		{
			mMaterials[mat.getHandle()] = &mat;
		}

		PX_FORCE_INLINE PxU32 getNumMaterials()	const
		{
			return mHandleManager.getNumUsedID();
		}

		void removeMaterial(NpMaterial& mat)
		{
			const PxU32 handle = mat.getHandle();
			if(handle != MATERIAL_INVALID_HANDLE)
			{
				mMaterials[handle] = NULL;
				mHandleManager.freeID(handle);
			}
		}

		PX_FORCE_INLINE NpMaterial* getMaterial(const PxU32 index)	const
		{
			PX_ASSERT(index <  mMaxMaterials);
			return mMaterials[index];
		}

		PX_FORCE_INLINE PxU32 getMaxSize()	const 
		{
			return mMaxMaterials;
		}

		PX_FORCE_INLINE NpMaterial** getMaterials() const
		{
			return mMaterials;
		}

	private:
		void resize()
		{
			const PxU32 numMaterials = mMaxMaterials;
			mMaxMaterials = mMaxMaterials*2;

			NpMaterial** mat = reinterpret_cast<NpMaterial**>(PX_ALLOC(sizeof(NpMaterial*)*mMaxMaterials,  "NpMaterialManager::resize"));
			PxMemZero(mat, sizeof(NpMaterial*)*mMaxMaterials);
			for(PxU32 i=0; i<numMaterials; ++i)
				mat[i] = mMaterials[i];

			PX_FREE(mMaterials);

			mMaterials = mat;
		}

		Cm::IDPool		mHandleManager;
		NpMaterial**	mMaterials;
		PxU32			mMaxMaterials;
	};

	class NpMaterialManagerIterator
	{
	public:
		NpMaterialManagerIterator(const NpMaterialManager& manager) : mManager(manager), mIndex(0)
		{
		}

		bool getNextMaterial(NpMaterial*& np)
		{
			const PxU32 maxSize = mManager.getMaxSize();
			PxU32 index = mIndex;
			while(index < maxSize && mManager.getMaterial(index)==NULL)
				index++;
			np = NULL;
			if(index < maxSize)
				np = mManager.getMaterial(index++);
			mIndex = index;
			return np!=NULL;
		}

	private:
		NpMaterialManagerIterator& operator=(const NpMaterialManagerIterator&);
		const NpMaterialManager&	mManager;
		PxU32						mIndex;
	};
}

#endif
