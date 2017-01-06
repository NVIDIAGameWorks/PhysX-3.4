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

#ifndef PX_PVD_SCENE_CLIENT_H
#define PX_PVD_SCENE_CLIENT_H

/** \addtogroup pvd
@{
*/

#include "foundation/PxFlags.h"

namespace physx
{
	namespace pvdsdk
	{
		class PvdClient;   
		struct PvdDebugPoint;
		struct PvdDebugLine;
		struct PvdDebugTriangle;
		struct PvdDebugText;
	}
}

#if !PX_DOXYGEN
namespace physx
{
#endif

/**
\brief PVD scene Flags. They are disabled by default, and only works if PxPvdInstrumentationFlag::eDEBUG is set.
*/
struct PxPvdSceneFlag
{
	enum Enum
	{
		eTRANSMIT_CONTACTS     = (1 << 0), //! Transmits contact stream to PVD.
		eTRANSMIT_SCENEQUERIES = (1 << 1), //! Transmits scene query stream to PVD.
		eTRANSMIT_CONSTRAINTS  = (1 << 2)  //! Transmits constraints visualize stream to PVD.
	};
};

/**
\brief Bitfield that contains a set of raised flags defined in PxPvdSceneFlag.

@see PxPvdSceneFlag
*/
typedef PxFlags<PxPvdSceneFlag::Enum, PxU8> PxPvdSceneFlags;
PX_FLAGS_OPERATORS(PxPvdSceneFlag::Enum, PxU8)

/**
\brief Special client for PxScene.
It provides access to the PxPvdSceneFlag.
It also provides simple user debug services that associated scene position such as immediate rendering and camera updates.
*/
class PxPvdSceneClient
{
  public:
	/**
	Sets the PVD flag. See PxPvdSceneFlag.
	\param flag Flag to set.
	\param value value the flag gets set to.
	*/
	virtual void setScenePvdFlag(PxPvdSceneFlag::Enum flag, bool value) = 0;

	/**
	Sets the PVD flags. See PxPvdSceneFlags.
	\param flags Flags to set.
	*/
	virtual void setScenePvdFlags(PxPvdSceneFlags flags) = 0;

	/**
	Retrieves the PVD flags. See PxPvdSceneFlags.
	*/
	virtual PxPvdSceneFlags getScenePvdFlags() const = 0;

	/**
	update camera on PVD application's DirectX render window
	*/
	virtual void updateCamera(const char* name, const PxVec3& origin, const PxVec3& up, const PxVec3& target) = 0;

	/**
	draw points on PVD application's DirectX render window
	*/
	virtual void drawPoints(const physx::pvdsdk::PvdDebugPoint* points, PxU32 count) = 0;

	/**
	draw lines on PVD application's DirectX render window
	*/
	virtual void drawLines(const physx::pvdsdk::PvdDebugLine* lines, PxU32 count) = 0;

	/**
	draw triangles on PVD application's DirectX render window
	*/
	virtual void drawTriangles(const physx::pvdsdk::PvdDebugTriangle* triangles, PxU32 count) = 0;

	/**
	draw text on PVD application's DirectX render window
	*/
	virtual void drawText(const physx::pvdsdk::PvdDebugText& text) = 0;

	/**
	get the underlying client, for advanced users
	*/
	virtual physx::pvdsdk::PvdClient* getClientInternal() = 0;

protected:
	virtual ~PxPvdSceneClient(){}
};

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif // PX_PVD_SCENE_CLIENT_H
