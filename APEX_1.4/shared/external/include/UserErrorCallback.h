/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_ERROR_CALLBACK_H
#define USER_ERROR_CALLBACK_H

#include "PxErrorCallback.h"
#include "PxErrors.h"
#include <PsString.h>
#include <ApexUsingNamespace.h>

#include <map>
#include <string>
#include <vector>


class UserErrorCallback : public physx::PxErrorCallback
{
public:
	UserErrorCallback(const char* filename, const char* mode, bool header, bool reportErrors);
	~UserErrorCallback();

	void		printError(const char* message, const char* errorCode = NULL, const char* file = NULL, int line = 0);
	void		reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
	void		printError(physx::PxErrorCode::Enum code, const char* file, int line, const char* fmt, ...);
	int			getNumErrors();
	void		clearErrorCounter();
	const char*	getFirstEror();
	void		addFilteredMessage(const char* msg, bool fullMatch, bool* trigger = NULL);

	void		reportErrors(bool enabled);

	static UserErrorCallback* instance()
	{
		if (!s_instance)
		{
			// Allocate a stub (bitbucket) error handler
			s_instance = ::new UserErrorCallback(NULL, NULL, false, false);
		}
		return s_instance;
	}

private:
	bool	messageFiltered(const char * code, const char * msg);
	void	openFile();

	uint32_t				mNumErrors;
	FILE*						mOutFile;
	std::string					mOutFileName;
	const char*					mOutFileMode;
	bool						mOutFileHeader;
	bool						mReportErrors;
	char						mFirstErrorBuffer[2048];
	bool						mFirstErrorBufferUpdated;

	std::map<std::string, bool*> mFilteredMessages;
	std::vector<std::pair<std::string, bool*> >	mFilteredParts;

	static UserErrorCallback* s_instance;
};

// gcc uses names ...s
#define ERRORSTREAM_INVALID_PARAMETER(_A, ...) \
	UserErrorCallback::instance()->printError(physx::PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__, _A, ##__VA_ARGS__)
#define ERRORSTREAM_INVALID_OPERATION(_A, ...) \
	UserErrorCallback::instance()->printError(physx::PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, _A, ##__VA_ARGS__)
#define ERRORSTREAM_DEBUG_ERROR(_A, ...) \
	UserErrorCallback::instance()->printError(physx::PxErrorCode::eINTERNAL_ERROR   , __FILE__, __LINE__, _A, ##__VA_ARGS__)
#define ERRORSTREAM_DEBUG_INFO(_A, ...) \
	UserErrorCallback::instance()->printError(physx::PxErrorCode::eDEBUG_INFO       , __FILE__, __LINE__, _A, ##__VA_ARGS__)
#define ERRORSTREAM_DEBUG_WARNING(_A, ...) \
	UserErrorCallback::instance()->printError(physx::PxErrorCode::eDEBUG_WARNING    , __FILE__, __LINE__, _A, ##__VA_ARGS__)

#endif
