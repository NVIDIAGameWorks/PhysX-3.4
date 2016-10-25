#pragma once
#include <GL/glew.h>
#include "Shader.h"

class HDRHelper{
public:
	int Width, Height;
	int realWidth, realHeight;

	float padding;
	float fov;
	float fovPad;
	float zNear, zFar;
	float scale;

	Shader mShaderBloomH;
	Shader mShaderBloomV;
	Shader mShaderDOF;

	HDRHelper(float fov, float padding, float zNear, float zFar, const char* resourcePath, float scale);

	void beginHDR(bool useOwnFbo = true);
	void endHDR(bool useOwnFbo = true);

	void Resize(int w, int h);

	void DoHDR(GLuint oldFBO = 0, bool useDOF = true);
	void Destroy();

	GLuint mHDRFbo;
	GLuint mHDRBlurFbo;

	GLuint mHDRColorTex;
	GLuint mHDRDepthTex;
	GLuint mHDRBlurTex;
	GLuint mHDRBloomTex;
};
