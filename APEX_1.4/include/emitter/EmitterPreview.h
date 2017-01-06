/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef EMITTER_PREVIEW_H
#define EMITTER_PREVIEW_H

#include "Apex.h"
#include "AssetPreview.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
	Preview class for ApexEmitter. Used for preview rendeing.
*/
class EmitterPreview : public AssetPreview
{
public:
	///Draws the emitter
	virtual void	drawEmitterPreview() = 0;
	///Sets the scaling factor of the renderable
	virtual void	setScale(float scale) = 0;

protected:
	EmitterPreview() {}
};


PX_POP_PACK

}
} // end namespace nvidia

#endif // EMITTER_PREVIEW_H
