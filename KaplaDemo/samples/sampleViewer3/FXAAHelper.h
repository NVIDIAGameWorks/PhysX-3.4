#pragma once
#include <GL/glew.h>
#include "Shader.h"

class FXAAHelper{
public:
	int Width, Height;
	Shader computeLuma, fxaa;
	FXAAHelper(const char* resourcePath);
	void Resize(int w, int h);
	void StartFXAA(); // Call before rendering things
	void EndFXAA(GLuint oldFBO);  // Compute luma and then do FXAA
	void Destroy();
	GLuint FBO, depthTex, imgTex, imgWithLumaTex;
};
