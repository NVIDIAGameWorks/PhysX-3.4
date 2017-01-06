/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MIRROR_SCENE_H

#define MIRROR_SCENE_H

/*!
\file
\brief classes MirrorScene, MirrorScene::MirrorFilter
*/

#include "ApexDefs.h"
#include "ApexUsingNamespace.h"
#include "foundation/PxPreprocessor.h"


namespace nvidia
{
	namespace apex
	{

		/**
		\brief MirrorScene is used to create a selected mirrored copy of a primary scene.  Works only with PhysX 3.x
		*/
		class MirrorScene
		{
		public:
			/**
			\brief MirrorFilter is a callback interface implemented by the application to confirm which actors and shapes are, or are not, replicated into the mirrored scene
			*/
			class MirrorFilter
			{
			public:
				/**
				\brief The application returns true if this actor should be mirrored into the secondary mirrored scene.

				\param[in] actor A const reference to the actor in the primary scene to be considered mirrored into the secondary scene.
				*/
				virtual bool shouldMirror(const PxActor &actor) = 0;

				/**
				\brief The application returns true if this shape should be mirrored into the secondary mirrored scene.

				\param[in] shape A const reference to the shape in the primary scene to be considered mirrored into the secondary scene.
				*/
				virtual bool shouldMirror(const PxShape &shape) = 0;

				/**
				\brief Affords the application with an opportunity to modify the contents/state of the shape before is placed into the mirrored scene.

				\param[in] shape A reference to the shape that is about to be placed into the mirrored scene.
				*/
				virtual void reviseMirrorShape(physx::PxShape &shape) = 0;

				/**
				\brief Affords the application with an opportunity to modify the contents/state of the actor before is placed into the mirrored scene.

				\param[in] actor A reference to the actor that is about to be placed into the mirrored scene
				*/
				virtual void reviseMirrorActor(physx::PxActor &actor) = 0;
			};

			/**
			\brief SynchronizePrimaryScene updates the positions of the objects around the camera relative to the static and dynamic distances specified
			These objects are then put in a thread safe queue to be processed when the mirror scene synchronize is called

			\param[in] cameraPos The current position of the camera relative to where objects are being mirrored
			*/
			virtual void synchronizePrimaryScene(const PxVec3 &cameraPos) = 0;

			/**
			\brief Processes the updates to get this mirrored scene to reflect the subset of the
			primary scene that is being mirrored.  Completely thread safe, assumes that
			the primary scene and mirrored scene are most likely being run be completely
			separate threads.
			*/
			virtual void synchronizeMirrorScene(void) = 0;

			/**
			\brief Releases the MirrorScene class and all associated mirrored objects; it is important to not that this does *not* release
			the actual APEX scnee; simply the MirrorScene helper class.
						*/
			virtual void release(void) = 0;

		};

	}; // end of apex namespace
}; // end of physx namespace

#endif
