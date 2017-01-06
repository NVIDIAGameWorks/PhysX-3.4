/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FIND_H
#define FIND_H

namespace nvidia {
namespace apex {

class FileHandler
{
public:
	virtual void handle(const char*) = 0;
};

void Find(const char* root, FileHandler& f, const char** ignoredFiles = 0);

}}

#endif
