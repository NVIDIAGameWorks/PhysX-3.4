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
#include "SampleDirManager.h"
#include "SampleAssetManager.h"
#include "SamplePlatform.h"
#include "PsUtilities.h"
#include "PxTkFile.h"
#include "PsString.h"

using namespace SampleFramework;

static void FixPathSeparator(char* pathBuffer)
{
	char separator = SampleFramework::SamplePlatform::platform()->getPathSeparator()[0];

	const size_t length = strlen(pathBuffer);
	for(size_t i=0; i<length ; ++i)
	{
		if ('/' == pathBuffer[i] || '\\' == pathBuffer[i])
			pathBuffer[i] = separator;
	}
}

SampleDirManager::SampleDirManager(const char* relativePathRoot, bool isReadOnly, int maxRecursion) : mIsReadOnly(isReadOnly)
{
	if(!relativePathRoot || !SampleFramework::searchForPath(relativePathRoot, mPathRoot, PX_ARRAY_SIZE(mPathRoot), isReadOnly, maxRecursion))
	{
		shdfnd::printFormatted("path \"%s\" not found\n", relativePathRoot);
		mPathRoot[0] = '\0';
	}
}

#define MAX_PATH_LENGTH 256

const char* SampleDirManager::getFilePath(const char* relativeFilePath, char* pathBuffer, bool testFileValidity)
{
	PX_ASSERT(pathBuffer);

	bool flattenRelativeFilePaths = false;
#if defined (RENDERER_IOS)
	//iOS doesn't allow to share files in folders...
	flattenRelativeFilePaths = !mIsReadOnly;
#endif
	
	strcpy(pathBuffer, getPathRoot());
	strcat(pathBuffer, "/");
	
	if (flattenRelativeFilePaths)
	{
		char flattendPath[MAX_PATH_LENGTH];
		strcpy(flattendPath, relativeFilePath);
		for (size_t i = 0; i < strlen(flattendPath); ++i)
		{
			if ('/' == flattendPath[i] || '\\' == flattendPath[i])
				flattendPath[i] = '_';
		}
		strcat(pathBuffer, flattendPath);
	}
	else
	{
		strcat(pathBuffer, relativeFilePath);
		if (!mIsReadOnly)
		{
			FixPathSeparator(pathBuffer);
			//strip file from path and make sure the output directory exists
			const char* ptr = strrchr(pathBuffer, '/');
			if (!ptr)
				ptr = strrchr(pathBuffer, '\\');
			
			if (ptr)
			{
				char dir[MAX_PATH_LENGTH];
				assert(MAX_PATH_LENGTH >= strlen(pathBuffer));
				strcpy(dir, pathBuffer);
				dir[ptr - pathBuffer] = '\0';
				FixPathSeparator(dir);
				SampleFramework::SamplePlatform::platform()->makeSureDirectoryPathExists(dir);
			}
		}
	}

	FixPathSeparator(pathBuffer);

	if(testFileValidity)
	{
		File* fp = NULL;
		PxToolkit::fopen_s(&fp, pathBuffer, "rb");
		if(!fp)
		{
			// Kai: user can still get full path in the path buffer (side effect)
			return NULL;
		}
		fclose(fp);
	}

	return pathBuffer;
}
