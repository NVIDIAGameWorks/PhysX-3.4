/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "PxPreprocessor.h"

#if PX_WINDOWS_FAMILY
#include <windows.h>
#include <windowsx.h>
#endif

#include "MeshImport.h"

#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


namespace mimp
{

MeshImport* gMeshImport = NULL;

#if PX_WINDOWS_FAMILY

static void* getMeshBindingInterface(const char* dll, MiI32 version_number) // loads the tetra maker DLL and returns the interface pointer.
{
	void* ret = 0;

	UINT errorMode = 0;
	errorMode = SEM_FAILCRITICALERRORS;
	UINT oldErrorMode = SetErrorMode(errorMode);
	HMODULE module = LoadLibraryA(dll);
	SetErrorMode(oldErrorMode);
	if (module)
	{
		void* proc = GetProcAddress(module, "getInterface");
		if (proc)
		{
			typedef void* (__cdecl* NX_GetToolkit)(MiI32 version);
			ret = ((NX_GetToolkit)proc)(version_number);
		}
	}
	return ret;
}

#endif

}; // end of namespace

#ifdef LINUX_GENERIC
#include <sys/types.h>
#include <sys/dir.h>
#endif

#define MAXNAME 512

namespace mimp
{

class FileFind
{
public:
	FileFind::FileFind(const char *dirname,const char *spec)
	{
		if (dirname && strlen(dirname))
		{
			sprintf(mSearchName,"%s\\%s", dirname, spec);
		}
		else
		{
			sprintf(mSearchName,"%s",spec);
		}
	}

	FileFind::~FileFind(void)
	{
	}


	bool FindFirst(char* name)
	{
		bool ret = false;

	#if PX_WINDOWS_FAMILY

		hFindNext = FindFirstFileA(mSearchName, &finddata);
		if (hFindNext == INVALID_HANDLE_VALUE)
		{
			ret = false;
		}
		else
		{
			bFound = 1; // have an initial file to check.
			strncpy(name, finddata.cFileName, MAXNAME);
			ret = true;
		}
	#endif

	#ifdef LINUX_GENERIC
		mDir = opendir(".");
		ret = FindNext(name);
	#endif
		return ret;
	}

	bool FindNext(char* name)
	{
		bool ret = false;

		#if PX_WINDOWS_FAMILY
		while (bFound)
		{
			bFound = FindNextFileA(hFindNext, &finddata);
			if (bFound && (finddata.cFileName[0] != '.') && !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strncpy(name, finddata.cFileName, MAXNAME);
				ret = true;
				break;
			}
		}
		if (!ret)
		{
			bFound = 0;
			FindClose(hFindNext);
		}
		#endif

		#ifdef LINUX_GENERIC

		if (mDir)
		{
			while (1)
			{
				struct direct* di = readdir(mDir);
				if (!di)
				{
					closedir(mDir);
					mDir	= 0;
					ret		= false;
					break;
				}

				if (::strcmp(di->d_name,".") == 0 || ::strcmp(di->d_name,"..") == 0)
				{
					// skip it!
				}
				else
				{
					strncpy(name, di->d_name, MAXNAME);
					ret		= true;
					break;
				}
			}
		}
		#endif
		return ret;
	}

private:
	char	mSearchName[MAXNAME];
#if PX_WINDOWS_FAMILY
	WIN32_FIND_DATAA finddata;
	HANDLE	hFindNext;
	MiI32	bFound;
#endif
#ifdef LINUX_GENERIC
	DIR*	mDir;
#endif
};

}; // end of namespace

namespace mimp
{

static const char* lastSlash(const char* foo)
{
	const char* ret = foo;
	const char* last_slash = 0;

	while (*foo)
	{
		if (*foo == '\\')
		{
			last_slash = foo;
		}
		if (*foo == '/')
		{
			last_slash = foo;
		}
		if (*foo == ':')
		{
			last_slash = foo;
		}
		foo++;
	}
	if (last_slash)
	{
		ret = last_slash + 1;
	}
	return ret;
}

mimp::MeshImport* loadMeshImporters(const char* directory) // loads the mesh import library (dll) and all available importers from the same directory.
{
	mimp::MeshImport*	ret = 0;
#if PX_X86
	const char*		baseImporter = "MeshImport_x86.dll";
#else
	const char*		baseImporter = "MeshImport_x64.dll";
#endif
	char scratch[512];
	if (directory && strlen(directory))
	{
		sprintf(scratch,"%s\\%s", directory, baseImporter);
	}
	else
	{
		strcpy(scratch, baseImporter);
	}

	ret = reinterpret_cast<mimp::MeshImport*>(getMeshBindingInterface(scratch, MESHIMPORT_VERSION));

	if (ret)
	{
	#if PX_X86
		mimp::FileFind ff(directory,"MeshImport*_x86.dll");
	#else
		mimp::FileFind ff(directory,"MeshImport*_x64.dll");
	#endif
		char name[MAXNAME];
		if (ff.FindFirst(name))
		{
			do
			{
				const char* scan = lastSlash(name);
				if ( stricmp(scan,baseImporter) == 0 )
				{
					//printf("Skipping 'MeshImport.dll'\r\n");
				}
				else
				{
					//printf("Loading '%s'\r\n", scan );
					const char* fname;

					if (directory && strlen(directory))
					{
						sprintf(scratch,"%s\\%s", directory, scan);
						fname = scratch;
					}
					else
					{
						fname = name;
					}

			#if PX_WINDOWS_FAMILY
					mimp::MeshImporter* imp = reinterpret_cast<mimp::MeshImporter*>(getMeshBindingInterface(fname, MESHIMPORT_VERSION));
			#else
					mimp::MeshImporter* imp = 0;
			#endif
					if (imp)
					{
						ret->addImporter(imp);
						//printf("Added importer '%s'\r\n", name );
					}
				}
			} while (ff.FindNext(name));
		}
	}

	return ret;
}

}; // end of namespace
