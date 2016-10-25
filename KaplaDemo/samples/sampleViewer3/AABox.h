//--------------------------------------------------------------------------------------
// FroggyAA
// Author: Tristan Lorach
// Email: tlorach@nvidia.com
//
// Implementation of different antialiasing methods.
// - typical MSAA
// - CSAA
// - Hardware AA mixed with FBO for supersampling pass
//   - simple downsampling
//   - downsampling with 1 or 2 kernel filters
//
// AABox is the class that will handle everything related to supersampling through 
// an offscreen surface defined thanks to FBO
// Basic use is :
//
//  Initialize()
//  ...
//  Activate(int x=0, int y=0)
//    Draw the scene (so, in the offscreen supersampled buffer)
//  Deactivate()
//  Draw() : downsample to backbuffer
//  ...
//  Destroy()
//
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define FB_SS 0
#include <Cg/CgGL.h>
#include <string>
class AABox
{
public:
  AABox(std::string path); 
  ~AABox();

  bool Initialize(int w, int h, float ssfact, int depthSamples, int coverageSamples);
  void Destroy();

  void Activate(int x=0, int y=0);
  void Deactivate();
  void Rebind();
  void Draw(int technique);
  int getBufW() const {return bufw;};
  int getBufH() const {return bufh;};
  GLuint getTextureID() {return textureID;};

  CGparameter   cgBlendFactor;
//protected:
  bool          bValid;
  bool          bCSAA;

  int           vpx, vpy, vpw, vph;
  int           posx, posy;
  int           bufw, bufh;

  CGcontext     cgContext;
  CGeffect      cgEffect;
  CGtechnique   cgTechnique[4];
  CGpass        cgPassDownSample;
  CGpass        cgPassDrawFinal;
  GLuint        textureID;
  GLuint        textureDepthID;
  CGparameter   cgSrcSampler;
  CGparameter   cgSSsampler;
  CGparameter   cgDepthSSsampler;
  CGparameter	cgTexelSize;
  GLuint        fb;
  GLuint        fbms;
  GLuint        depth_rb;
  GLuint        color_rb;
  GLuint		oldFbo;
  std::string path;

  bool          initRT(int depthSamples, int coverageSamples);
};