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

#ifndef NV_PHYS_XTO_DRV_H
#define NV_PHYS_XTO_DRV_H

// The puprose of this interface is to provide graphics drivers with information 
// about PhysX state to draw PhysX visual indicator 

// We share information between modules using a memory section object. PhysX creates 
// such object, graphics drivers try to open it. The name of the object has 
// fixed part (NvPhysXToDrv_SectionName) followed by the process id. This allows 
// each process to have its own communication channel. 

namespace physx
{

#define NvPhysXToDrv_SectionName "PH71828182845_" 

// Vista apps cannot create stuff in Global\\ namespace when NOT elevated, so use local scope
#define NvPhysXToDrv_Build_SectionName(PID, buf) sprintf(buf, NvPhysXToDrv_SectionName "%x", PID) 
#define NvPhysXToDrv_Build_SectionNameXP(PID, buf) sprintf(buf, "Global\\" NvPhysXToDrv_SectionName "%x", PID) 

typedef struct NvPhysXToDrv_Header_ 
{ 
    int signature; // header interface signature 
    int version; // version of the interface 
    int size; // size of the structure 
    int reserved; // reserved, must be zero 
}	NvPhysXToDrv_Header; 

// this structure describes layout of data in the shared memory section 
typedef struct NvPhysXToDrv_Data_V1_ 
{ 
    NvPhysXToDrv_Header header; // keep this member first in all versions of the interface. 

    int bCpuPhysicsPresent; // nonzero if cpu physics is initialized 
    int bGpuPhysicsPresent; // nonzero if gpu physics is initialized 

}	NvPhysXToDrv_Data_V1; 

// some random magic number as our interface signature 
#define NvPhysXToDrv_Header_Signature 0xA7AB 

// use the macro to setup the header to the latest version of the interface 
// update the macro when a new verson of the interface is added 
#define NvPhysXToDrv_Header_Init(header)               \
{                                                      \
    header.signature = NvPhysXToDrv_Header_Signature;  \
    header.version = 1;                                \
    header.size = sizeof(NvPhysXToDrv_Data_V1);        \
    header.reserved = 0;                               \
} 

// validate the header against all known interface versions 
// add validation checks when new interfaces are added 
#define NvPhysXToDrv_Header_Validate(header, curVersion)   \
  (                                                        \
   (header.signature == NvPhysXToDrv_Header_Signature) &&  \
   (header.version   == curVersion) &&                     \
   (curVersion  == 1) &&                                   \
   (header.size == sizeof(NvPhysXToDrv_Data_V1))           \
  )

}

#endif	// __NVPHYSXTODRV_H__
