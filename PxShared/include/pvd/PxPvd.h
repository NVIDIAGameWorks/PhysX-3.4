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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#ifndef PXPVDSDK_PXPVD_H
#define PXPVDSDK_PXPVD_H

/** \addtogroup pvd
@{
*/
#include "foundation/PxFlags.h"
#include "foundation/PxProfiler.h"

#if !PX_DOXYGEN
namespace physx
{
#endif

class PxPvdTransport;

#if PX_WINDOWS_FAMILY && !PX_ARM_FAMILY
#ifndef PX_PVDSDK_DLL
#define PX_PVDSDK_API PX_DLL_IMPORT
#elif PX_PVDSDK_DLL
#define PX_PVDSDK_API PX_DLL_EXPORT
#endif
#elif PX_UNIX_FAMILY
#ifdef PX_PVDSDK_DLL
#define PX_PVDSDK_API PX_UNIX_EXPORT
#endif
#endif

#ifndef PX_PVDSDK_API
#define PX_PVDSDK_API
#endif

/**
\brief types of instrumentation that PVD can do.
*/
struct PxPvdInstrumentationFlag
{
	enum Enum
	{
		/**
			\brief Send debugging information to PVD.

			This information is the actual object data of the rigid statics, shapes,
			articulations, etc.  Sending this information has a noticeable impact on
			performance and thus this flag should not be set if you want an accurate
			performance profile.
	     */
		eDEBUG   = 1 << 0,

		/**
			\brief Send profile information to PVD.

			This information populates PVD's profile view.  It has (at this time) negligible
			cost compared to Debug information and makes PVD *much* more useful so it is quite
			highly recommended.

			This flag works together with a PxCreatePhysics parameter.
			Using it allows the SDK to send profile events to PVD.
	    */
		ePROFILE = 1 << 1,

		/**
			\brief Send memory information to PVD.

			The PVD sdk side hooks into the Foundation memory controller and listens to
			allocation/deallocation events.  This has a noticable hit on the first frame,
			however, this data is somewhat compressed and the PhysX SDK doesn't allocate much
			once it hits a steady state.  This information also has a fairly negligible
			impact and thus is also highly recommended.

			This flag works together with a PxCreatePhysics parameter,
			trackOutstandingAllocations.  Using both of them together allows users to have
			an accurate view of the overall memory usage of the simulation at the cost of
			a hashtable lookup per allocation/deallocation.  Again, PhysX makes a best effort
			attempt not to allocate or deallocate during simulation so this hashtable lookup
			tends to have no effect past the first frame.

			Sending memory information without tracking outstanding allocations means that
			PVD will accurate information about the state of the memory system before the
			actual connection happened.
	    */
		eMEMORY  = 1 << 2,

		eALL     = (eDEBUG | ePROFILE | eMEMORY)
	};
};

/**
\brief Bitfield that contains a set of raised flags defined in PxPvdInstrumentationFlag.

@see PxPvdInstrumentationFlag
*/
typedef PxFlags<PxPvdInstrumentationFlag::Enum, uint8_t> PxPvdInstrumentationFlags;
PX_FLAGS_OPERATORS(PxPvdInstrumentationFlag::Enum, uint8_t)

/**
\brief PxPvd is the top-level class for the PVD framework, and the main customer interface for PVD
configuration.It is a singleton class, instantiated and owned by the application.
*/
class PxPvd : public physx::PxProfilerCallback
{
  public:
	/**
	Connects the SDK to the PhysX Visual Debugger application.
	\param transport transport for pvd captured data.
	\param flags Flags to set.
	return True if success
	*/
	virtual bool connect(PxPvdTransport& transport, PxPvdInstrumentationFlags flags) = 0;

	/**
	Disconnects the SDK from the PhysX Visual Debugger application.
	If we are still connected, this will kill the entire debugger connection.
	*/
	virtual void disconnect() = 0;

	/**
	 *	Return if connection to PVD is created.
	  \param useCachedStatus
	    1> When useCachedStaus is false, isConnected() checks the lowlevel network status.
	       This can be slow because it needs to lock the lowlevel network stream. If isConnected() is
	       called frequently, the expense of locking can be significant.
	    2> When useCachedStatus is true, isConnected() checks the highlevel cached status with atomic access.
	       It is faster than locking, but the status may be different from the lowlevel network with latency of up to
	       one frame.
	       The reason for this is that the cached status is changed inside socket listener, which is not
	       called immediately when the lowlevel connection status changes.
	 */
	virtual bool isConnected(bool useCachedStatus = true) = 0;

	/**
	returns the PVD data transport
	returns NULL if no transport is present.
	*/
	virtual PxPvdTransport* getTransport() = 0;

	/**
	Retrieves the PVD flags. See PxPvdInstrumentationFlags.
	*/
	virtual PxPvdInstrumentationFlags getInstrumentationFlags() = 0;

	/**
	\brief Releases the pvd instance.
	*/
	virtual void release() = 0;

  protected:
	virtual ~PxPvd()
	{
	}
};

/**
	\brief Create a pvd instance. 	
	\param foundation is the foundation instance that stores the allocator and error callbacks.
*/
PX_PVDSDK_API PxPvd* PX_CALL_CONV PxCreatePvd(PxFoundation& foundation);

#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif // PXPVDSDK_PXPVD_H
