/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MODULE_LOADER_H
#define MODULE_LOADER_H

#include "Apex.h"
#include "PsHashMap.h"

namespace nvidia
{
namespace apex
{

/**
\brief The ModuleLoader is a utility class for loading APEX modules

\note The most useful methods for rapid integration are "loadAllModules()",
      "loadAllLegacyModules()", and "releaseLoadedModules()".

\note If you need to release APEX modules loaded with ModuleLoader you should use one of
      the release-methods provided below. Note that you may never need it because SDK automatically
	  releases all modules when program ends.
*/
class ModuleLoader : public Module
{
public:
	/// \brief Load and initialize a specific APEX module
	virtual Module* loadModule(const char* name) = 0;

	/**
	\brief Load and initialize a list of specific APEX modules
	\param [in] names	The names of modules to load
	\param [in] size 	Number of modules to load
	\param [in] modules The modules array must be the same size as the names array to
	                    support storage of every loaded Module pointer. Use NULL if
	                    you do not need the list of created modules.
	 */
	virtual void loadModules(const char** names, uint32_t size, Module** modules = 0) = 0;

	/// \brief Load and initialize all APEX modules
	virtual void loadAllModules() = 0;

	/// \brief Load and initialize all legacy APEX modules (useful for deserializing legacy assets)
	virtual void loadAllLegacyModules() = 0;

	/// \brief Returns the number of loaded APEX modules
	virtual uint32_t getLoadedModuleCount() const = 0;

	/// \brief Returns the APEX module specified by the index if it was loaded by this ModuleLoader
	virtual Module* getLoadedModule(uint32_t idx) const = 0;

	/// \brief Returns the APEX module specified by the name if it was loaded by this ModuleLoader
	virtual Module* getLoadedModule(const char* name) const = 0;

	/// \brief Releases the APEX module specified by the index if it was loaded by this ModuleLoader
	virtual void releaseModule(uint32_t idx) = 0;

	/// \brief Releases the APEX module specified by the name if it was loaded by this ModuleLoader
	virtual void releaseModule(const char* name) = 0;

	/// \brief Releases the APEX module specified by the index if it was loaded by this ModuleLoader
	virtual void releaseModules(Module** modules, uint32_t size) = 0;

	/**
	\brief Releases the specified APEX module
	\note If the ModuleLoader is used to load modules, this method must be used to
	      release individual modules.
	      Do not use the Module::release() method.
	*/
	virtual void releaseModule(Module* module) = 0;

	/**
	\brief Releases the APEX modules specified in the names list
	\note If the ModuleLoader is used to load modules, this method must be used to
	      release individual modules.
	      Do not use the Module::release() method.
	*/
	virtual void releaseModules(const char** names, uint32_t size) = 0;

	/// Releases all APEX modules loaded by this ModuleLoader
	virtual void releaseLoadedModules() = 0;

	/// ModuleNameErrorMap
	typedef shdfnd::HashMap<const char*, nvidia::apex::ApexCreateError> ModuleNameErrorMap;
	
	/// Returns ModuleNameErrorMap
	virtual const ModuleNameErrorMap& getLoadedModulesErrors() const = 0;
};

#if !defined(_USRDLL)
/**
* If this module is distributed as a static library, the user must call this
* function before calling ApexSDK::createModule("Loader")
*/
void instantiateModuleLoader();
#endif

}
} // end namespace nvidia::apex

#endif // MODULE_LOADER_H
