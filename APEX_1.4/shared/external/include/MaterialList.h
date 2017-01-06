/*
 * Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef MATERIAL_LIST_H
#define MATERIAL_LIST_H

#include <map>
#include <string>
#include <vector>

namespace Samples
{

class MaterialList
{
public:
	MaterialList();
	~MaterialList();

	void clear();
	void addPath(const char* path);

	struct MaterialInfo
	{
		MaterialInfo();
		bool isLit;
		bool vshaderStatic;
		bool vshader1bone;
		bool vshader4bones;
		unsigned int fromPath;

		std::string diffuseTexture;
		std::string normalTexture;
	};

	struct TextureInfo
	{
		TextureInfo();
		unsigned int fromPath;
	};

	const MaterialInfo* containsMaterial(const char* materialName) const;
	const char* findClosest(const char* materialName) const;

	const TextureInfo* containsTexture(const char* textureName) const;

	void getFirstMaterial(std::string& name, MaterialInfo& info);
	bool getNextMaterial(std::string& name, MaterialInfo& info);

private:
	unsigned int addMaterial(const char* directory, const char* prefix, const char* materialName);
	unsigned int addTexture(const char* directory, const char* prefix, const char* textureName);

	std::vector<std::string> mPaths;

	typedef std::map<std::string, MaterialInfo> tMaterialNames;
	tMaterialNames mMaterialNames;
	tMaterialNames::const_iterator mMaterialIterator;

	typedef std::map<std::string, TextureInfo> tTextureNames;
	tTextureNames mTextureNames;
};

} // namespace Samples


#endif // MATERIAL_LIST_H
