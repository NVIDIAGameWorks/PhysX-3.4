/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexDefs.h"

#include "ModuleClothingHelpers.h"
#include "AbstractMeshDescription.h"

namespace nvidia
{
namespace apex
{

void AbstractMeshDescription::UpdateDerivedInformation(RenderDebugInterface* renderDebug)
{
	if (numIndices > 0)
	{
		pMin = pPosition[pIndices[0]];
		pMax = pMin;
	}
	avgEdgeLength = 0;
	avgTriangleArea = 0;

	uint32_t triCount(numIndices / 3);
	uint32_t edgeCount(numIndices);
	for (uint32_t j = 0; j < numIndices; j += 3)
	{
		uint32_t i0 = pIndices[j + 0];
		uint32_t i1 = pIndices[j + 1];
		uint32_t i2 = pIndices[j + 2];

		const PxVec3& v0 = pPosition[i0];
		const PxVec3& v1 = pPosition[i1];
		const PxVec3& v2 = pPosition[i2];

		pMin.minimum(v0);
		pMin.minimum(v1);
		pMin.minimum(v2);

		pMax.maximum(v0);
		pMax.maximum(v1);
		pMax.maximum(v2);

		PxVec3 e0 = v1 - v0;
		PxVec3 e1 = v2 - v1;
		PxVec3 e2 = v0 - v2;

		avgEdgeLength += e0.magnitude();
		avgEdgeLength += e1.magnitude();
		avgEdgeLength += e2.magnitude();


		if (renderDebug)
		{
			using RENDER_DEBUG::DebugColors;
			RENDER_DEBUG_IFACE(renderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(renderDebug)->getDebugColor(DebugColors::DarkBlue));
			RENDER_DEBUG_IFACE(renderDebug)->debugLine(v0, v1);
			RENDER_DEBUG_IFACE(renderDebug)->debugLine(v1, v2);
			RENDER_DEBUG_IFACE(renderDebug)->debugLine(v2, v0);
			RENDER_DEBUG_IFACE(renderDebug)->setCurrentColor(RENDER_DEBUG_IFACE(renderDebug)->getDebugColor(DebugColors::Green));
			RENDER_DEBUG_IFACE(renderDebug)->debugPoint(v0, 0.1f);
			RENDER_DEBUG_IFACE(renderDebug)->debugPoint(v1, 0.1f);
			RENDER_DEBUG_IFACE(renderDebug)->debugPoint(v2, 0.1f);
		}

		float triangleArea = e0.cross(e2).magnitude() * 0.5f;
		avgTriangleArea += triangleArea;
		triCount++;
	}

	avgEdgeLength /= edgeCount;
	avgTriangleArea /= triCount;
	centroid = 0.5f * (pMin + pMax);
	radius = 0.5f * (pMax - pMin).magnitude();

	//printf("Min = <%f, %f, %f>; Max = <%f, %f, %f>; centroid = <%f, %f, %f>; radius = %f; avgEdgeLength = %f; avgTriangleArea = %f;\n",
	//	pMin.x, pMin.y, pMin.z, pMax.x, pMax.y, pMax.z, centroid.x, centroid.y, centroid.z, radius, avgEdgeLength, avgTriangleArea);
}

}
} // namespace nvidia

