/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "AgMMFile.h"

using namespace nvidia;

AgMMFile::AgMMFile():
mAddr(0), mSize(0), mFileH(0)
{}

AgMMFile::AgMMFile(char *name, unsigned int size, bool &alreadyExists)
{
	this->create(name, size, alreadyExists);
}

void AgMMFile::create(char *name, unsigned int size, bool &alreadyExists)
{
	alreadyExists = false;
	mSize = size;

	mFileH = CreateFileMapping(INVALID_HANDLE_VALUE,	// use paging file
		NULL,											// default security
		PAGE_READWRITE,									// read/write access
		0,												// buffer size (upper 32bits)
		mSize,											// buffer size (lower 32bits)
		name);											// name of mapping object
	if (mFileH == NULL || mFileH == INVALID_HANDLE_VALUE)
	{
		mSize=0;
		mAddr=0;
		return;
	}

	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		alreadyExists = true;
	}

	mAddr = MapViewOfFile(mFileH,		// handle to map object
		FILE_MAP_READ|FILE_MAP_WRITE,	// read/write permission
		0,                   
		0,                   
		mSize);

	if (mFileH == NULL || mAddr == NULL)
	{
		mSize=0;
		mAddr=0;
		return;
	}
}

void AgMMFile::destroy()
{
	if (!mAddr || !mFileH || !mSize)
		return;

	UnmapViewOfFile(mAddr);
	CloseHandle(mFileH);

	mAddr = 0;
	mFileH = 0;
	mSize = 0;
}

AgMMFile::~AgMMFile()
{
	destroy();
}
