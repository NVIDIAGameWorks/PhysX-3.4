/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#ifndef APEX_RENDER_MATERIAL_H
#define APEX_RENDER_MATERIAL_H

#include "Apex.h"
#include "Utils.h"
#include "DirectXTex.h"

#pragma warning(push)
#pragma warning(disable : 4350)
#include <string>
#include <vector>
#include <list>
#pragma warning(pop)

#pragma warning(disable : 4512)

using namespace nvidia;
using namespace nvidia::apex;
using namespace std;

class ApexResourceCallback;

struct TextureResource
{
	DirectX::TexMetadata metaData;
	DirectX::ScratchImage image;
};

class ApexRenderMaterial
{
  public:

	enum BlendMode
	{
		BLEND_NONE,
		BLEND_ALPHA_BLENDING,
		BLEND_ADDITIVE
	};

    ApexRenderMaterial(ApexResourceCallback* resourceProvider, const char* xmlFilePath);
	ApexRenderMaterial(ApexResourceCallback* resourceProvider, const NvParameterized::Interface *graphicMaterialData);
	ApexRenderMaterial(ApexResourceCallback* resourceProvider, const char* shaderFileName, const char* textureFileName, BlendMode blendMode = BLEND_NONE);
	~ApexRenderMaterial();

	void setBlending(BlendMode blendMode);
	void reload();

	class Instance
	{
	public:
		Instance(ApexRenderMaterial& material, ID3D11InputLayout* inputLayout, uint32_t shaderNum = 0) : mMaterial(material), mInputLayout(inputLayout), mShaderNum(shaderNum) {}
		~Instance() { SAFE_RELEASE(mInputLayout); }

		bool isValid();
		void bind(ID3D11DeviceContext& context, uint32_t slot);
	private:
		ApexRenderMaterial& mMaterial;
		ID3D11InputLayout* mInputLayout;
		uint32_t mShaderNum;
	};

	Instance* getMaterialInstance(const D3D11_INPUT_ELEMENT_DESC* elementDescs, uint32_t numElements);

  private:
	void initialize(ApexResourceCallback* resourceCallback, const char* shaderFileName, const char* textureFileName, BlendMode blendMode);
	void initialize(ApexResourceCallback* resourceProvider, vector<string> shaderFileNames, const char* textureFileName, BlendMode blendMode);

	void releaseReloadableResources();

	string mShaderFileName;
	string mTextureFileName;

	struct ShaderGroup
	{
		ShaderGroup() : vs(NULL), gs(NULL), ps(NULL)
		{
		}
		~ShaderGroup()
		{
			Release();
		}
		void Release()
		{
			SAFE_RELEASE(vs);
			SAFE_RELEASE(gs);
			SAFE_RELEASE(ps);
			SAFE_RELEASE(buffer);
		}
		void Set(ID3D11DeviceContext* c)
		{
			c->VSSetShader(vs, nullptr, 0);
			c->GSSetShader(gs, nullptr, 0);
			c->PSSetShader(ps, nullptr, 0);
		}
		bool IsValid()
		{
			return vs != NULL && ps != NULL;
		}
		ID3D11VertexShader* vs;
		ID3D11GeometryShader* gs;
		ID3D11PixelShader* ps;
		ID3DBlob* buffer;
	};

	list<Instance*> mInstances;
	TextureResource* mTexture;
	ID3D11ShaderResourceView* mTextureSRV;
	ID3D11BlendState* mBlendState;
	vector<string> mShaderFilePathes;
	vector<ShaderGroup*> mShaderGroups;
};

#endif