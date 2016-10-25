#pragma once
#include <GL/glew.h>
#include "Shader.h"
#define SAMPLEDIV 4
#define NUMSAMPLES SAMPLEDIV*SAMPLEDIV

class SSAOHelper{
public:
	int Width, Height;
	float padding;
	int SWidth, SHeight;
	float fov;
	float fovPad;
	float zNear, zFar;
	float scale;
	int realWidth, realHeight;

	Shader SSAO, SSAOFilterH, SSAOFilterV;
	SSAOHelper(float fov, float padding, float zNear, float zFar, const char* resourcePath, float scale);
	void Resize(int w, int h);

	void DoSSAO(void (*renderScene)(), GLuint oldFBO = 0);
	void Destroy();
	GLuint FBO, unitVecTex, normalTex, depthTex, ssaoTex, blurTex;
};
