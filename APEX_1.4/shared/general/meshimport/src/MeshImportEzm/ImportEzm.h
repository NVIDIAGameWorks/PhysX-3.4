/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef IMPORT_EZM_H

#define IMPORT_EZM_H

#include "MeshImport.h"

namespace mimp
{

MeshImporter * createMeshImportEZM(void);
void         releaseMeshImportEZM(MeshImporter *iface);

}; // end of namespace

#endif
