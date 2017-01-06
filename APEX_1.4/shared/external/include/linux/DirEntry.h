/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef DIR_ENTRY_H
#define DIR_ENTRY_H

#include "foundation/PxPreprocessor.h"

#if defined PX_LINUX || defined PX_ANDROID
#	include <dirent.h>
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <string>
#	include <sstream>
#else
#	error Unsupported platform
#endif

namespace physx
{
	class DirEntry
	{
	public:

		DirEntry()
		{
			mDir = NULL;
			mEntry = NULL;
			mIdx = 0;
			mCount = 0;
		}

		~DirEntry()
		{
			if (!isDone())
			{
				while (next());
			}

			// The Find.cpp loop behaves badly and doesn't cleanup the DIR pointer
			if (mDir)
			{
				closedir(mDir);
				mDir = NULL;
			}
		}

		// Get successive element of directory.
		// Returns true on success, error otherwise.
		bool next()
		{
			if (mIdx < mCount)
			{
				mEntry = readdir(mDir);
				++mIdx;
			}
			else
			{
				bool ret = (0 == closedir(mDir));
				mDir = NULL;
				mEntry = NULL;
				return ret;
			}
			return true;
		}

		// No more entries in directory?
		bool isDone() const
		{
			return mIdx >= mCount;

		}

		// Is this entry a directory?
		bool isDirectory() const
		{
			if (mEntry)
			{
				if(DT_UNKNOWN == mEntry->d_type)
				{
					// on some fs d_type is DT_UNKNOWN, so we need to use stat instead
					std::ostringstream path;
					path << mDirPath << "/" << mEntry->d_name;
					struct stat s;
					if(stat(path.str().c_str(), &s) == 0)
					{
						return S_ISDIR(s.st_mode);
					}
					else 
					{
						return false;
					}
				}
				else 
				{
					return DT_DIR == mEntry->d_type;
				}
			}
			else
			{
				return false;
			}
		}

		// Get name of this entry.
		const char* getName() const
		{
			if (mEntry)
			{
				return mEntry->d_name;
			}
			else
			{
				return NULL;
			}
		}

		// Get first entry in directory.
		static bool GetFirstEntry(const char* path, DirEntry& dentry)
		{
			dentry.mDir = opendir(path);
			dentry.mDirPath.assign(path, strlen(path));
			if (!dentry.mDir)
			{
				return false;
			}

			dentry.mIdx = 0;

			// count files
			dentry.mCount = 0;
			if (dentry.mDir != NULL)
			{
				while (readdir(dentry.mDir))
				{
					dentry.mCount++;
				}
			}
			closedir(dentry.mDir);
			dentry.mDir = opendir(path);
			dentry.mEntry = readdir(dentry.mDir);
			return true;
		}

	private:

		DIR* mDir;
		std::string mDirPath;
		struct dirent* mEntry;
		long mIdx, mCount;
	};
}

#endif
