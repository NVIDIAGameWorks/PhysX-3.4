/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULES_H_
#define MODULES_H_

#include <ApexDefs.h>

#include <ModuleLegacy.h>
#include <ModuleClothing.h>

#if PX_PHYSICS_VERSION_MAJOR == 3
#include <ModuleDestructible.h>

#if APEX_USE_PARTICLES

#include <ModuleBasicFS.h>
#include <ModuleBasicIos.h>
#include <ModuleEmitter.h>
#include <ModuleFieldSampler.h>
#include <ModuleForceField.h>
#include <ModuleIofx.h>
#include <ModuleParticleIos.h>
#include <ModuleParticles.h>
#include <ModuleTurbulenceFS.h>

#endif

#endif

#endif
