#pragma once

#include "GFSDK_SSAO.h"
#include "Shader.h"

class HBAOHelper
{
public:
	HBAOHelper(float fov, float zNear, float zFar);
	~HBAOHelper();

	bool init();
	bool renderAO(void(*renderScene)(), GLuint oldFBO = 0, bool useNormalTexture = true);
	void resize(int wAA, int hAA, int realw, int realH);

private:
	GFSDK_SSAO_Parameters	mAoParams;
	GFSDK_SSAO_GLFunctions	mGLFunctions;
	GFSDK_SSAO_Context_GL*	mHbaoGlContext;
	float mNormalMapTransform[16];
	float mFov;
	float mZnear;
	float mZFar;
	GLint mWidthAA;
	GLint mHeightAA;
	GLint mWidthReal;
	GLint mHeightReal;
	GLuint mNormalTex;
	GLuint mDepthTex;
	GLuint mFBO;
	GLuint mDownScaledFBO;
	GLuint mColorTex;
};
