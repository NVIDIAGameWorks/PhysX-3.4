/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef GROUND_EMITTER_PREVIEW_H
#define GROUND_EMITTER_PREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{


PX_PUSH_PACK_DEFAULT

///Ground emitter preview class. Use for preview rendering
class GroundEmitterPreview : public AssetPreview
{
public:
	///Does the drawing
	virtual void	drawEmitterPreview() = 0;
	///Sets the scale factor of the preview object
	virtual void	setScale(float scale) = 0;

protected:
	GroundEmitterPreview() {}
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // GROUND_EMITTER_PREVIEW_H
