/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IMPACT_EMITTER_ASSET_H
#define IMPACT_EMITTER_ASSET_H

#include "Apex.h"

namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

#define IMPACT_EMITTER_AUTHORING_TYPE_NAME "ImpactEmitterAsset"

class ImpactEmitterActor;

///Impact emitter asset class
class ImpactEmitterAsset : public Asset
{
protected:
	PX_INLINE ImpactEmitterAsset() {}
	virtual ~ImpactEmitterAsset() {}

public:
	/// Get the setID that corresponds to the set name \sa ImpactEmitterActor::registerImpact()
	virtual uint32_t			querySetID(const char* setName) = 0;

	/**
	\brief Get a list of the event set names.  The name index in the outSetNames list is the setID.
	\param [in,out] inOutSetNames An array of const char * with at least nameCount
	\param [in,out] nameCount The size of setNames as input and output, set to -1 if setIDs > nameCount
	*/
	virtual void				getSetNames(const char** inOutSetNames, uint32_t& nameCount) const = 0;
};

///Impact emitter asset authoring. Used to create Impact Emitter assets
class ImpactEmitterAssetAuthoring : public AssetAuthoring
{
protected:
	virtual ~ImpactEmitterAssetAuthoring() {}

public:
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // IMPACT_EMITTER_ASSET_H
