/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_CONVERSIONIOFXASSETPARAMETERS_0P0_0P1H_H
#define MODULE_CONVERSIONIOFXASSETPARAMETERS_0P0_0P1H_H

#include "NvParamConversionTemplate.h"
#include "IofxAssetParameters_0p0.h"
#include "IofxAssetParameters_0p1.h"

#include "nvparameterized/NvParamUtils.h"

#define PARAM_RET(x) if( (x) != NvParameterized::ERROR_NONE ) \
		{ PX_ASSERT(0 && "INVALID Parameter"); return false; }

namespace nvidia {
namespace apex {
namespace legacy {


typedef NvParameterized::ParamConversionTemplate<nvidia::parameterized::IofxAssetParameters_0p0, 
						nvidia::parameterized::IofxAssetParameters_0p1, 
						nvidia::parameterized::IofxAssetParameters_0p0::ClassVersion, 
						nvidia::parameterized::IofxAssetParameters_0p1::ClassVersion>
						ConversionIofxAssetParameters_0p0_0p1Parent;

class ConversionIofxAssetParameters_0p0_0p1: public ConversionIofxAssetParameters_0p0_0p1Parent
{
public:
	static NvParameterized::Conversion* Create(NvParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionIofxAssetParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionIofxAssetParameters_0p0_0p1)(t) : 0;
	}

private:
	ConversionIofxAssetParameters_0p0_0p1(NvParameterized::Traits* t) : ConversionIofxAssetParameters_0p0_0p1Parent(t) {}
	// TODO: make it a generic "array copy" method
	bool deepCopyModifiers(NvParameterized::Interface* newModListParams, const char* modListString)
	{
		int32_t arraySize = 0, modIgnoreCount = 0;
		NvParameterized::Handle hOld(*mLegacyData, modListString);
		NvParameterized::Handle hNew(*newModListParams, modListString);

		PX_ASSERT(hOld.isValid() && hNew.isValid());

		// resize the new array
		NvParameterized::getParamArraySize(*mLegacyData, modListString, arraySize);
		PARAM_RET(hNew.resizeArray(arraySize));

		// copy the modifiers
		for (int32_t i = 0; i < arraySize; i++)
		{
			NvParameterized::Interface* curOldMod = NULL;

			PARAM_RET(hNew.set(i - modIgnoreCount));
			PARAM_RET(hOld.set(i));

			PARAM_RET(hOld.getParamRef(curOldMod));
			PX_ASSERT(curOldMod);

			if (!nvidia::strcmp(curOldMod->className(), "RotationModifierParams") &&
			        !nvidia::strcmp(newModListParams->className(), "SpriteIofxParameters"))
			{
				modIgnoreCount++;

				hNew.popIndex();
				hOld.popIndex();
				continue;
			}

			PARAM_RET(hNew.setParamRef(curOldMod));
			PARAM_RET(hOld.setParamRef(0));

			hNew.popIndex();
			hOld.popIndex();
		}

		if (modIgnoreCount)
		{
			PARAM_RET(hNew.resizeArray(arraySize - modIgnoreCount));
		}

		return true;
	}

	bool copyRenderMeshList(NvParameterized::Interface* meshIofxParams)
	{
		char indexedWeightString[32];

		NvParameterized::Handle hOld(*mLegacyData, "renderMeshList");
		PX_ASSERT(hOld.isValid());

		NvParameterized::Handle hNew(*meshIofxParams, "renderMeshList");
		PX_ASSERT(hNew.isValid());

		int32_t arraySize = 0;
		PARAM_RET(hOld.getArraySize(arraySize));
		PARAM_RET(hNew.resizeArray(arraySize));

		for (int32_t i = 0; i < arraySize; i++)
		{
			PARAM_RET(hNew.set(i));
			PARAM_RET(hOld.set(i));

			// first handle the weight
			uint32_t oldWeight;
			shdfnd::snprintf(indexedWeightString, sizeof(indexedWeightString), "renderMeshList[%i].weight", i);
			NvParameterized::getParamU32(*mLegacyData, indexedWeightString, oldWeight);
			NvParameterized::setParamU32(*meshIofxParams, indexedWeightString, oldWeight);

			// then get the meshAssetName (named ref)
			NvParameterized::Handle hChild(mLegacyData);
			PARAM_RET(hOld.getChildHandle(mLegacyData, "meshAssetName", hChild));
			PX_ASSERT(hOld.isValid());

			NvParameterized::Interface* oldMeshRef = NULL;

			PARAM_RET(hChild.getParamRef(oldMeshRef));
			PARAM_RET(hChild.setParamRef(0));
			PX_ASSERT(oldMeshRef);

			PARAM_RET(hNew.getChildHandle(meshIofxParams, "meshAssetName", hChild));
			PARAM_RET(hChild.setParamRef(oldMeshRef));

			hNew.popIndex();
			hOld.popIndex();
		}

		return true;
	}

protected:

	bool convert()
	{
		// if sprite name is present, then it's a sprite modifier, else, mesh
		NvParameterized::Interface* iofxType = 0;
		if (mLegacyData->spriteMaterialName && mLegacyData->spriteMaterialName->name())
		{
			// create new sprite type

			iofxType = mTraits->createNvParameterized("SpriteIofxParameters", 0);
			PX_ASSERT(iofxType);

			// copy the new spriteMaterialName named reference
			NvParameterized::Handle h(*iofxType, "spriteMaterialName");
			h.setParamRef(mLegacyData->spriteMaterialName);
			mLegacyData->spriteMaterialName = 0;
		}
		else
		{
			// create new mesh iofx type

			iofxType = mTraits->createNvParameterized("MeshIofxParameters", 0);
			PX_ASSERT(iofxType);

			// copy the renderMeshList array (renderMesh names and weights)
			copyRenderMeshList(iofxType);
		}

		// deep copy the spawn and continuous list
		deepCopyModifiers(iofxType, "spawnModifierList");
		deepCopyModifiers(iofxType, "continuousModifierList");

		mNewData->iofxType = iofxType;

		return true;
	}
};


}
}
} //nvidia::apex::legacy

#endif
