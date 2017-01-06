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


#ifndef PXS_MATERIALCOMBINER_H
#define PXS_MATERIALCOMBINER_H

#include "PxsMaterialCore.h"

namespace physx
{

	class PxsMaterialCombiner
	{
	public:

		class PxsCombinedMaterial
		{
		public:
			PxReal staFriction;
			PxReal dynFriction;
			PxU32  flags;	//PxMaterialFlag::eDISABLE_FRICTION, PxMaterialFlag::eDISABLE_STRONG_FRICTION.
		};

		static PxReal combineRestitution(const PxsMaterialData& material0, const PxsMaterialData& material1);

		PxsMaterialCombiner(PxReal staticFrictionScaling, PxReal dynamicFrictionScaling);

		PxsCombinedMaterial combineIsotropicFriction(const PxsMaterialData& material0, const PxsMaterialData& material1);

		//ML:: move this function to header file to avoid LHS in Xbox
		PX_FORCE_INLINE void combineIsotropicFriction(const PxsMaterialData& mat0, const PxsMaterialData& mat1, PxReal& dynamicFriction, PxReal& staticFriction, PxU32& flags)
		{
		
			const PxU32 combineFlags= (mat0.flags | mat1.flags); //& (PxMaterialFlag::eDISABLE_STRONG_FRICTION|PxMaterialFlag::eDISABLE_FRICTION);	//eventually set DisStrongFric flag, lower all others.

			if (!(combineFlags & PxMaterialFlag::eDISABLE_FRICTION))
			{
				const PxI32 fictionCombineMode = PxMax(mat0.getFrictionCombineMode(), mat1.getFrictionCombineMode());
				PxReal dynFriction = 0.f;
				PxReal staFriction = 0.f;


				switch (fictionCombineMode)
				{
				case PxCombineMode::eAVERAGE:
					dynFriction = 0.5f * (mat0.dynamicFriction + mat1.dynamicFriction);
					staFriction = 0.5f * (mat0.staticFriction + mat1.staticFriction);
					break;
				case PxCombineMode::eMIN:
					dynFriction = PxMin(mat0.dynamicFriction, mat1.dynamicFriction);
					staFriction = PxMin(mat0.staticFriction, mat1.staticFriction);
					break;
				case PxCombineMode::eMULTIPLY:
					dynFriction = (mat0.dynamicFriction * mat1.dynamicFriction);
					staFriction = (mat0.staticFriction * mat1.staticFriction);
					break;
				case PxCombineMode::eMAX:
					dynFriction = PxMax(mat0.dynamicFriction, mat1.dynamicFriction);
					staFriction = PxMax(mat0.staticFriction, mat1.staticFriction);
					break;
				}   

				dynFriction*=mDynamicFrictionScaling;
				staFriction*=mStaticFrictionScaling;
				//isotropic case
				const PxReal fDynFriction = PxMax(dynFriction, 0.f);

				const PxReal fStaFriction = physx::intrinsics::fsel(staFriction - fDynFriction, staFriction*mStaticFrictionScaling, fDynFriction);
				/*dest.dynFriction = fDynFriction;
				dest.staFriction = fStaFriction;*/

				dynamicFriction = fDynFriction;
				staticFriction = fStaFriction;
				flags = combineFlags;
			}
			else
			{
			/*	dest.flags |= PxMaterialFlag::eDISABLE_STRONG_FRICTION;
				dest.staFriction = 0.0f;
				dest.dynFriction = 0.0f;*/
				flags = (combineFlags | PxMaterialFlag::eDISABLE_STRONG_FRICTION);
				dynamicFriction = 0.f;
				staticFriction = 0.f;
			}

		}


	//private:
	protected:
		static PX_FORCE_INLINE PxReal combineScalars(PxReal a, PxReal b, PxI32 nxCombineMode)
		{
			switch (nxCombineMode)
			{
			case PxCombineMode::eAVERAGE:
				return 0.5f * (a + b);
			case PxCombineMode::eMIN:
				return PxMin(a,b);
			case PxCombineMode::eMULTIPLY:
				return a * b;
			case PxCombineMode::eMAX:
				return PxMax(a,b);
			default:
			/*	Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, 
					"Sc::MaterialCombiner::combineScalars(): unknown combine mode");*/
				return PxReal(0);
			}   
		}

		PxReal mStaticFrictionScaling, mDynamicFrictionScaling;

	};

}

#endif
