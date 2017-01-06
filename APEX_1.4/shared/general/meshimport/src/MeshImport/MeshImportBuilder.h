/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MESH_IMPORT_BUILDER_H

#define MESH_IMPORT_BUILDER_H

#include "MeshImport.h"

namespace mimp
{

class MeshBuilder : public MeshSystem, public MeshImportInterface
{
public:
  virtual void gather(void) = 0;
  virtual void scale(MiF32 s) = 0;
  virtual void rotate(MiF32 rotX,MiF32 rotY,MiF32 rotZ) = 0;
  virtual void setMeshImportBinary(bool state) = 0;


};

MeshBuilder * createMeshBuilder(const char *meshName,
                                const void *data,
                                MiU32 dlen,
                                MeshImporter *mi,
                                const char *options,
                                MeshImportApplicationResource *appResource);


MeshBuilder * createMeshBuilder(MeshImportApplicationResource *appResource);
void          releaseMeshBuilder(MeshBuilder *m);

}; // end of namespace

#endif
