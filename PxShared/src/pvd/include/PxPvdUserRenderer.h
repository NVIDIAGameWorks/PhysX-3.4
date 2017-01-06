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
#ifndef PXPVDSDK_PXPVDUSERRENDERER_H
#define PXPVDSDK_PXPVDUSERRENDERER_H

/** \addtogroup pvd
@{
*/
#include "foundation/PxVec3.h"
#include "foundation/PxTransform.h"
#include "pvd/PxPvd.h"

#include "PxPvdDataStream.h"
#include "PxPvdRenderBuffer.h"
#include "PsUserAllocated.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxPvd;

#if !PX_DOXYGEN
namespace pvdsdk
{
#endif

class RendererEventClient;

class PvdUserRenderer : public shdfnd::UserAllocated
{
  protected:
	virtual ~PvdUserRenderer()
	{
	}

  public:
	virtual void release() = 0;
	virtual void setClient(RendererEventClient* client) = 0;

	// Instance to associate the further rendering with.
	virtual void setInstanceId(const void* instanceId) = 0;
	// Draw these points associated with this instance
	virtual void drawPoints(const PvdDebugPoint* points, uint32_t count) = 0;
	// Draw these lines associated with this instance
	virtual void drawLines(const PvdDebugLine* lines, uint32_t count) = 0;
	// Draw these triangles associated with this instance
	virtual void drawTriangles(const PvdDebugTriangle* triangles, uint32_t count) = 0;
	// Draw this text associated with this instance
	virtual void drawText(const PvdDebugText& text) = 0;

	// Draw SDK debug render
	virtual void drawRenderbuffer(const PvdDebugPoint* pointData, uint32_t pointCount, const PvdDebugLine* lineData,
	                              uint32_t lineCount, const PvdDebugTriangle* triangleData, uint32_t triangleCount) = 0;

	// Constraint visualization routines
	virtual void visualizeJointFrames(const PxTransform& parent, const PxTransform& child) = 0;
	virtual void visualizeLinearLimit(const PxTransform& t0, const PxTransform& t1, float value, bool active) = 0;
	virtual void visualizeAngularLimit(const PxTransform& t0, float lower, float upper, bool active) = 0;
	virtual void visualizeLimitCone(const PxTransform& t, float ySwing, float zSwing, bool active) = 0;
	virtual void visualizeDoubleCone(const PxTransform& t, float angle, bool active) = 0;

	// Clear the immedate buffer.
	virtual void flushRenderEvents() = 0;

	PX_PVDSDK_API static PvdUserRenderer* create(uint32_t bufferSize = 0x2000);
};

class RendererEventClient 
{
 public:
	virtual ~RendererEventClient(){}

	virtual void handleBufferFlush(const uint8_t* inData, uint32_t inLength) = 0;
};

#if !PX_DOXYGEN
}
}
#endif
/** @} */
#endif // PXPVDSDK_PXPVDUSERRENDERER_H
