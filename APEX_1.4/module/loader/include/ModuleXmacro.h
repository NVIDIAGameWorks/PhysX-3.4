/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// Run X-macro over all modules
// #define MODULE(moduleName, instFunc, hasInit)

#include <PhysXSDKVersion.h>

#pragma warning(push)
#pragma warning(disable:4127)

#ifndef MODULE_LEGACY
#define MODULE_LEGACY(moduleName, instFunc, hasInit) MODULE(moduleName, instFunc, hasInit)
#endif


MODULE("Clothing", instantiateModuleClothing, true)
MODULE("Destructible", instantiateModuleDestructible, true)
MODULE("Legacy", instantiateModuleLegacy, false);


#if APEX_USE_PARTICLES
MODULE("Particles", instantiateModuleParticles, true)
// No correct init method yet...
MODULE("TurbulenceFS", instantiateModuleTurbulenceFS, false)
#endif // APEX_USE_PARTICLES


// Clear inputs
#undef MODULE
#undef MODULE_LEGACY

#pragma warning(pop)
