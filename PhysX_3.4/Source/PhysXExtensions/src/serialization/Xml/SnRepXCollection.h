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
#ifndef PX_REPXCOLLECTION_H
#define PX_REPXCOLLECTION_H

#include "common/PxTolerancesScale.h"
#include "PxRepXSerializer.h"

namespace physx { namespace Sn {
	
	struct	XmlNode;
	
	struct RepXCollectionItem
	{
		PxRepXObject		liveObject;
		XmlNode*			descriptor;
		RepXCollectionItem( PxRepXObject inItem = PxRepXObject(), XmlNode* inDescriptor = NULL )
			: liveObject( inItem )
			, descriptor( inDescriptor )
		{
		}
	};

	struct RepXDefaultEntry
	{
		const char* name;
		const char* value;
		RepXDefaultEntry( const char* pn, const char* val ) : name( pn ), value( val ){}
	};

	/**
	*	The result of adding an object to the collection.
	*/
	struct RepXAddToCollectionResult
	{
		enum Enum
		{
			Success,
			SerializerNotFound,
			InvalidParameters, //Null data passed in.
			AlreadyInCollection
		};

		PxSerialObjectId	collectionId;
		Enum				result;

		RepXAddToCollectionResult( Enum inResult = Success, const PxSerialObjectId inId = 0 )
			: collectionId( inId )
			, result( inResult )
		{
		}
		bool isValid() { return result == Success && collectionId != 0; }
	};
	/**
	*	A RepX collection contains a set of static data objects that can be transformed
	*	into live objects.  It uses RepX serializer to do two transformations:
	*	live object <-> collection object (descriptor)
	*	collection object <-> file system.
	*
	*	A live object is considered to be something live in the physics
	*	world such as a material or a rigidstatic.
	*
	*	A collection object is a piece of data from which a live object
	*	of identical characteristics can be created.  
	*
	*	Clients need to pass PxCollection so that objects can resolve
	*	references.  In addition, objects must be added in an order such that
	*	references can be resolved in the first place.  So objects must be added
	*	to the collection *after* objects they are dependent upon.
	*
	*	When deserializing from a file, the collection will allocate char*'s that will
	*	not be freed when the collection itself is freed.  The user must be responsible
	*	for these character allocations.
	*/
	class RepXCollection 
	{
	protected:
		virtual ~RepXCollection(){}

	public:
		virtual void destroy() = 0;

		/**
		*	Set the scale on this collection.  The scale is saved with the collection.
		*
		*	If the scale wasn't set, it will be invalid.
		*/
		virtual void  setTolerancesScale( const PxTolerancesScale& inScale ) = 0;

		/**
		*	Get the scale that was set at collection creation time or at load time.
		*	If this is a loaded file and the source data does not contain a scale
		*	this value will be invalid (PxTolerancesScale::isValid()).
		*/
		virtual PxTolerancesScale getTolerancesScale() const = 0;

		/**
		*	Set the up vector on this collection.  The up vector is saved with the collection.
		*
		*	If the up vector wasn't set, it will be (0,0,0).
		*/
		virtual void  setUpVector( const PxVec3& inUpVector ) = 0;

		/**
		* If the up vector wasn't set, it will be (0,0,0).  Else this will be the up vector
		* optionally set when the collection was created.
		*/
		virtual PxVec3	getUpVector() const = 0;

		virtual const char* getVersion() = 0;
		static const char* getLatestVersion();

		//Necessary accessor functions for translation/upgrading.
		virtual const RepXCollectionItem* begin() const = 0;
		virtual const RepXCollectionItem* end() const = 0;


		//Performs a deep copy of the repx node.
		virtual XmlNode* copyRepXNode( const XmlNode* srcNode ) = 0;

		virtual void addCollectionItem( RepXCollectionItem inItem ) = 0;

		//Create a new repx node with this name.  Its value is unset.
		virtual XmlNode& createRepXNode( const char* name ) = 0;

		virtual RepXCollection& createCollection( const char* inVersionStr ) = 0;
		//Release this when finished.
		virtual XmlReaderWriter& createNodeEditor() = 0;

		virtual PxAllocatorCallback& getAllocator() = 0;

		virtual bool instantiateCollection( PxRepXInstantiationArgs& inArgs, PxCollection& inPxCollection ) = 0;

		
		virtual RepXAddToCollectionResult addRepXObjectToCollection( const PxRepXObject& inObject, PxCollection* inCollection, PxRepXInstantiationArgs& inArgs ) = 0;

		/**
		 *	Save this collection out to a file stream.  Uses the RepX serialize to perform 
		 *	collection object->file conversions.
		 *
		 *	/param[in] inStream Write-only stream to save collection out to.
		 */
		virtual void save( PxOutputStream& inStream ) = 0;
	};
} }

#endif
