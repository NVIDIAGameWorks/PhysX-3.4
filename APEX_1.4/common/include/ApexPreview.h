/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_PREVIEW_H__
#define __APEX_PREVIEW_H__

#include "ApexRenderable.h"

namespace nvidia
{
namespace apex
{

class ApexContext;

/**
	Class that implements preview interface
*/
class ApexPreview : public ApexRenderable
{
public:
	ApexPreview();
	virtual  						~ApexPreview();

	// Each class that derives from ApexPreview may optionally implement this function
	virtual Renderable*		getRenderable()
	{
		return NULL;
	}

	virtual void					setPose(const PxMat44& pose);
	virtual const PxMat44			getPose() const;

	virtual void					release() = 0;
	void							destroy();

protected:
	bool					mInRelease;

	PxMat44					mPose;
};

}
} // end namespace nvidia::apex

#endif // __APEX_PREVIEW_H__
