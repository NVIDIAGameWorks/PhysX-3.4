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

#include "PxTkMatrixUtils.h"

using namespace PxToolkit;

void PxToolkit::setRotX(PxMat33& m, PxReal angle)
{
	m = PxMat33(PxIdentity);

	const PxReal cos = cosf(angle);
	const PxReal sin = sinf(angle);

	m[1][1] = m[2][2] = cos;
	m[1][2] = sin;
	m[2][1] = -sin;
}

void PxToolkit::setRotY(PxMat33& m, PxReal angle)
{
	m = PxMat33(PxIdentity);

	const PxReal cos = cosf(angle);
	const PxReal sin = sinf(angle);

	m[0][0] = m[2][2] = cos;
	m[0][2] = -sin;
	m[2][0] = sin;
}

void PxToolkit::setRotZ(PxMat33& m, PxReal angle)
{
	m = PxMat33(PxIdentity);

	const PxReal cos = cosf(angle);
	const PxReal sin = sinf(angle);

	m[0][0] = m[1][1] = cos;
	m[0][1] = sin;
	m[1][0] = -sin;
}

PxQuat PxToolkit::getRotXQuat(float angle)
{
	PxMat33 m;
	setRotX(m, angle);
	return PxQuat(m);
}

PxQuat PxToolkit::getRotYQuat(float angle)
{
	PxMat33 m;
	setRotY(m, angle);
	return PxQuat(m);
}

PxQuat PxToolkit::getRotZQuat(float angle)
{
	PxMat33 m;
	setRotZ(m, angle);
	return PxQuat(m);
}
