/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IMPACT_EMITTER_PREVIEW_H
#define IMPACT_EMITTER_PREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

///Impact emitter preview. Used for preview rendering of the emitter actors.
class ImpactEmitterPreview : public AssetPreview
{
public:
	/* No need to add any functionality here */

protected:
	ImpactEmitterPreview() {}
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // IMPACT_EMITTER_PREVIEW_H
