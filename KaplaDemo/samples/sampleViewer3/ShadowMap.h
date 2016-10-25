//----------------------------------------------------------------------------------
// File:   ShadowMapping.cpp
// Original Author: Rouslan Dimitrov
// Modified by: Nuttapong Chentanez and Matthias Müller-Fischer
// Email:  sdkfeedback@nvidia.com
// 
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//----------------------------------------------------------------------------------


#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "foundation/PxVec3.h"
#include <GL/glew.h>

using namespace physx;

#define FAR_DIST 200.0
#define MAX_SPLITS 1
#define LIGHT_FOV 45.0

//--------------------------------------------------------------------------
class ShadowMap
{
public:
	ShadowMap( int w, int h, float fovi, int matOffseti, int resolution = 4096);

	void makeShadowMap(const PxVec3 &cameraPos, const PxVec3 &cameraDir, const PxVec3 &lightDir, float znear, float zfar,
		void (*renderShadowCasters)());

	// call before the map is used for rendering
	void prepareForRender(float* cam_modelview, float* cam_proj);
	// call when rendering is done
	void doneRender();

	int    getTextureSize() { return depth_size; }
	GLuint getDepthTexArray() { return depth_tex_ar; }
	float  getFarBound(int i) { return far_bound[i]; }
	int    getNumSplits() { return cur_num_splits; }

private:
	struct Frustum {
		float neard;
		float fard;
		float fov;
		float ratio;
		PxVec3 point[8];
	};

	struct Vec4;
	struct Matrix44;

	void updateFrustumPoints(Frustum &f, const PxVec3 &center, const PxVec3 &view_dir);
	void updateSplitDist(Frustum f[MAX_SPLITS], float nd, float fd);
	float applyCropMatrix(Frustum &f);

	void cameraInverse(float dst[16], float src[16]);
	void init();

	float far_bound[MAX_SPLITS];

	float minZAdd;
	float maxZAdd;

	float shadowOff;
	float shadowOff2;
	float fov;

	int cur_num_splits;
	int width;
	int height;
	int depth_size ;

	GLuint depth_fb;
	GLuint depth_tex_ar;
	
	Frustum f[MAX_SPLITS];
	float shad_cpm[MAX_SPLITS][16];

	float split_weight;
	int matOffset;
};
#endif