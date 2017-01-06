/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IOS_ASSET_H
#define IOS_ASSET_H

/*!
\file
\brief class IosAsset
*/

#include "Asset.h"

namespace nvidia
{
namespace apex
{

class Scene;
class Actor;

class IofxAsset;

PX_PUSH_PACK_DEFAULT

/**
\brief The base class of all Instanced Object Simulation classes
*/
class IosAsset : public Asset
{
public:
	//! \brief create a generic IOS Actor in a specific Scene
	virtual Actor*		        createIosActor(Scene& scene, IofxAsset* iofxAsset) = 0;
	//! \brief release a generic IOS Actor
	virtual void				releaseIosActor(Actor& actor) = 0;
	//! \brief get supports density
	virtual bool				getSupportsDensity() const = 0;
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif // IOS_ASSET_H
