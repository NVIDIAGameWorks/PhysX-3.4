/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexResource.h"
#include "ApexSDKHelpers.h"

namespace nvidia
{
namespace apex
{

ApexResource::~ApexResource()
{
	removeSelf();
}

void ApexResource::removeSelf()
{
	if (m_list && m_listIndex != 0xFFFFFFFF)
	{
		m_list->remove(m_listIndex);
	}
	m_list = NULL;
	m_listIndex = 0xFFFFFFFF;
}

} // namespace apex
} // namespace nvidia
