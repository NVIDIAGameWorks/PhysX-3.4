#pragma once
extern float seedDustRadius;
extern float seedDustNumPerSite;
extern float seedDustNumPerSiteCollisionEvent;
extern float applyForcePointAreaPerSample;
extern float applyForceFromPointsDrag;
extern float noiseKIsoDecay;
extern float noiseKAniDecay;
extern float noise3DScale;
extern float noise2DScale;
extern float dustMaxLife;
extern float dustMinLife;
extern float dustParticleOpacity;
extern float dustParticleRenderRadius;
extern float particleStartFade;
extern float octaveScaling;
extern float divStrength;
extern float curDivStrength;
extern float divStrengthReductionRate;


extern bool doGaussianBlur;
extern float blurVelSigmaFDX;
extern float blurVelCenterFactor;
extern float curlScale;
extern float areaPerDustSample;
extern float areaPerDebris;
extern float minSizeDecayRate;

extern float explodeVel; // Velocity of debris
extern float numParPerMeteor;

extern float minMeteorDustLife;
extern float maxMeteorDustLife;
extern float minMeteorDustSize;
extern float maxMeteorDustSize;

extern int maxNumDebrisPerType;
extern float sleepingThresholdRB;
extern float sleepingThresholdParticles;
extern int maxNumDebrisToAdd;
extern float minMeteorSizeDecayRate;