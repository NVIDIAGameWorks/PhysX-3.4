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


#ifndef PX_PHYSICS_SCP_CLOTH_FABRIC_CORE
#define PX_PHYSICS_SCP_CLOTH_FABRIC_CORE

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PxPhysXConfig.h"
#include "PxClothFabric.h"
#include "PsArray.h"

namespace physx
{

#if PX_USE_CLOTH_API

class PxSerializationContext;

namespace cloth
{
	class Fabric;
}


namespace Sc
{
	struct ClothFabricBulkData : public Ps::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		// constructor and destructor are needed because some compilers zero the memory in the
		// default constructor and that breaks the serialization related memory markers for 
		// implicitly padded bytes
		ClothFabricBulkData() {}
		~ClothFabricBulkData() {}

		void	exportExtraData(PxSerializationContext& stream);
		void	importExtraData(PxDeserializationContext& context);
		static void	getBinaryMetaData(PxOutputStream& stream);

		PxU32 mNbParticles;

		shdfnd::Array<PxU32> mPhases;
		shdfnd::Array<PxU32> mSets;
		shdfnd::Array<float> mRestvalues;
		shdfnd::Array<PxU32> mIndices;
		shdfnd::Array<PxU32> mTetherAnchors;
		shdfnd::Array<PxReal> mTetherLengths;
		shdfnd::Array<PxU32> mTriangles;
	};


	class ClothFabricCore : public Ps::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
								ClothFabricCore(const PxEMPTY);		
		void					exportExtraData(PxSerializationContext& stream);
		void					importExtraData(PxDeserializationContext& context);
		static	void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
								ClothFabricCore();
								~ClothFabricCore();

		PX_FORCE_INLINE cloth::Fabric& getLowLevelFabric() const { PX_ASSERT(mLowLevelFabric); return *mLowLevelFabric; }
		PX_FORCE_INLINE void	setLowLevelGpuFabric(cloth::Fabric* fabric) { mLowLevelGpuFabric = fabric; }

		bool					load(PxInputStream& stream);
		bool					load(const PxClothFabricDesc& desc);

        PxU32                   getNbParticles() const;
		PxU32                   getNbPhases() const;
        PxU32					getNbSets() const;
        PxU32					getNbParticleIndices() const;
        PxU32                   getNbRestvalues() const;
		PxU32					getNbTethers() const;
        
		PxU32                   getPhases(PxClothFabricPhase* userPhaseIndexBuffer, PxU32 bufferSize) const;
		PxU32			        getRestvalues(PxReal* userRestvalueBuffer, PxU32 bufferSize) const;

		PxU32                   getSets(PxU32* userPhaseBuffer, PxU32 bufferSize) const;
        PxU32                   getParticleIndices(PxU32* userParticleIndexBuffer, PxU32 bufferSize) const;

		PxU32					getTetherAnchors(PxU32* userAnchorBuffer, PxU32 bufferSize) const;
		PxU32					getTetherLengths(PxReal* userLengthBuffer, PxU32 bufferSize) const;

		PxClothFabricPhaseType::Enum getPhaseType(PxU32 phaseIndex) const { return mPhaseTypes[phaseIndex]; }
                
        void                    scaleRestlengths(PxReal scale);

	private:
        cloth::Fabric*			mLowLevelFabric;
		cloth::Fabric*			mLowLevelGpuFabric;

		// Each phase has a type defined in PxClothFabricPhaseType::Enum.
		// The size of this array is the same as number of phases (mPhases.size())
		Ps::Array<PxClothFabricPhaseType::Enum> mPhaseTypes;
	};

} // namespace Sc


#endif // PX_USE_CLOTH_API

}

#endif
