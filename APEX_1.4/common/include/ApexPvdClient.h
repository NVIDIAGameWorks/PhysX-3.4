/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_PVD_CLIENT_H
#define APEX_PVD_CLIENT_H

#include "Px.h"
#include "ApexDefs.h"

#define APEX_PVD_NAMESPACE "Apex"

//#define WITHOUT_PVD 1
#ifdef WITHOUT_PVD
namespace physx
{
	class PxPvd;
	namespace pvdsdk
	{
		class PvdDataStream;
		class PvdUserRenderer;
	}
}
#else
#include "PsPvd.h"
#include "PxPvdClient.h"
#include "PxPvdObjectModelBaseTypes.h"
#include "PxPvdDataStream.h"
#include "PxPvdUserRenderer.h"
#endif

namespace NvParameterized
{
	class Interface;
	class Definition;
	class Handle;
}

namespace physx
{
namespace pvdsdk
{
	/**
	\brief Define what action needs to be done when updating pvd with an NvParameterized object.
	*/
	struct PvdAction
	{
		/**
		\brief Enum
		*/
		enum Enum
		{
			/**
			\brief Create instances and update properties.
			*/
			UPDATE,

			/**
			\brief Destroy instances.
			*/
			DESTROY
		};
	};



	/**
	\brief The ApexPvdClient class allows APEX and PhysX to both connect to the PhysX Visual Debugger (PVD)
	*/
	class ApexPvdClient : public PvdClient
	{
	public:
		/**
		\brief Check if the PVD connection is active
		*/
		virtual bool isConnected() const = 0;

		/**
		\brief Called when PVD connection established
		*/
		virtual void onPvdConnected() = 0;

		/**
		\brief Called when PVD connection finished
		*/
		virtual void onPvdDisconnected() = 0;

		/**
		\brief Flush data streams etc.
		*/
		virtual void flush() = 0;

		/**
		\brief Retrieve the PxPvd
		*/
		virtual PxPvd& getPxPvd() = 0;

		/**
		\brief Returns the data stream if Pvd is connected.
		*/
		virtual PvdDataStream* getDataStream() = 0;

		/**
		\brief Returns the PvdUserRenderer if Pvd is connected.
		*/
		virtual PvdUserRenderer* getUserRender() = 0;

		//virtial PvdMetaDataBinding* getMetaDataBinding() = 0;

		/**
		\brief Initializes the classes sent to pvd.
		*/
		virtual void initPvdClasses() = 0;

		/**
		\brief Sends the existing instances to pvd.
		*/
		virtual void initPvdInstances() = 0;

		/**
		\brief Adds properties of an NvParameterized object to the provided class and creates necessary subclasses for structs.

		\note The pvd class pvdClassName must already exist. Pvd classes for structs are being created, but not for references.
		*/
		virtual void initPvdClasses(const NvParameterized::Definition& paramsHandle, const char* pvdClassName) = 0;

		/**
		\brief Creates or destroys pvdInstances and/or updates properties.
		*/
		virtual void updatePvd(const void* pvdInstance, NvParameterized::Interface& params, PvdAction::Enum pvdAction = PvdAction::UPDATE) = 0;

		//////////////////

		/**
		\brief Start the profiling frame
		\note inInstanceId must *not* be used already by pvd
		*/
		virtual void beginFrame( void* inInstanceId ) = 0;

		/**
		\brief End the profiling frame
		*/
		virtual void endFrame( void* inInstanceId ) = 0;

		/**
		\brief Destroy this instance
		*/
		virtual void release() = 0;

		/**
		 *	Assumes foundation is already booted up.
		 */
		static ApexPvdClient* create( PxPvd* pvd );
	};

}
}



#endif // APEX_PVD_CLIENT_H
