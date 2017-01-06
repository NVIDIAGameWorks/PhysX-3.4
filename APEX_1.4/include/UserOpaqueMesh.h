/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef USER_OPAQUE_MESH_H
#define USER_OPAQUE_MESH_H

/*!
\file
\brief class UserOpaqueMesh
*/

#include "ApexUsingNamespace.h"

namespace nvidia
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

//! \brief Name of OpaqueMesh authoring type namespace
#define APEX_OPAQUE_MESH_NAME_SPACE "ApexOpaqueMesh"

/**
\brief Opaque mesh description
*/
class UserOpaqueMeshDesc
{
public:
	///the name of the opaque mesh
	const char* mMeshName;
};

/**
\brief An abstract interface to an opaque mesh
*
* An 'opaque' mesh is a binding between the 'name' of a mesh and some internal mesh representation used by the
* application.  This allows the application to refer to meshes by name without involving duplciation of index buffer and
* vertex buffer data declarations.
*/
class UserOpaqueMesh
{
public:
	virtual ~UserOpaqueMesh() {}
};

PX_POP_PACK

}
} // end namespace nvidia::apex

#endif
