/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MEM_TRACKER_H

#define MEM_TRACKER_H

#include "PxSimpleTypes.h"
#include "ApexUsingNamespace.h"

#if PX_WINDOWS_FAMILY // only compile this source code for windows!!

namespace MEM_TRACKER
{

/**
\brief The layout format for the memory report.
*/
enum MemoryReportFormat
{
	MRF_SIMPLE_HTML,       // just a very simple HTML document containing the tables.
	MRF_CSV,               // Saves the Tables out as comma seperated value text
	MRF_TEXT,              // Saves the tables out in human readable text format.
	MRF_TEXT_EXTENDED,     // Saves the tables out in human readable text format, but uses the MS-DOS style extended ASCII character set for the borders.
};

/**
\brief This enumeration indicates the type of memory allocation that is being performed.
*/
enum MemoryType
{
	MT_NEW,						// Captured new operator
	MT_NEW_ARRAY,				// Captured new array operator
	MT_MALLOC,					// Standard heap allocation
	MT_FREE,					// Standard heap free
	MT_DELETE,					// Captured delete operator
	MT_DELETE_ARRAY,			// Captured array delete
	MT_GLOBAL_NEW,				// Allocation via Global new
	MT_GLOBAL_NEW_ARRAY,		// Allocation via global new array
	MT_GLOBAL_DELETE,			// Deallocation via global delete
	MT_GLOBAL_DELETE_ARRAY,		// Deallocation via global delete array
};

/**
\brief This data structure is used to return the current state of a particular block of allocated memory.
*/
struct TrackInfo
{
	const void		*mMemory;		// Address of memory
	MemoryType		 mType;			// Type of allocation
	size_t			mSize;			// Size of the memory allocation
	const char		*mContext;		// The context of the memory allocation.
	const char		*mClassName;	// The class or type name of this allocation.
	const char		*mFileName;		// Source code file name where this allocation occured
	uint32_t	 mLineNo;		// Source code line number where this allocation occured
	size_t			mAllocCount; 	// Indicates which time this allocation occured at this particular source file and line number.
};


class MemTracker
{
public:

  virtual void trackAlloc(size_t threadId,
                          void *mem,
                          size_t size,
                          MemoryType type,
                          const char *context,
                          const char *className,
                          const char *fileName,
                          uint32_t lineno) = 0;

  virtual void trackRealloc(size_t threadId,
	                          void *oldMem,
 	                          void *newMem,
	                          size_t newSize,
	                          const char *context,
	                          const char *className,
	                          const char *fileName,
	                          uint32_t lineno) = 0;

  virtual void trackFree(size_t threadId,
                        void *mem,
                        MemoryType type,
                        const char *context,
                        const char *fileName,uint32_t lineno) = 0;

  virtual const char * trackValidateFree(size_t threadId,
	  void *mem,
	  MemoryType type,
	  const char *context,
	  const char *fileName,uint32_t lineno) = 0;


   virtual void trackFrame(void) = 0;


   virtual bool trackInfo(const void *mem,TrackInfo &info)  = 0;


   virtual void *generateReport(MemoryReportFormat format,const char *fname,uint32_t &saveLen,bool reportAllLeaks) = 0;
   virtual void releaseReportMemory(void *mem) = 0;

    virtual void usage(void) = 0;
    virtual size_t detectLeaks(size_t &acount) = 0;
    virtual void setLogLevel(bool logEveryAllocation,bool logEveyFrame,bool verifySingleThreaded) = 0;

};

MemTracker *createMemTracker(void);
void        releaseMemTracker(MemTracker *mt);

};

#endif

#endif
