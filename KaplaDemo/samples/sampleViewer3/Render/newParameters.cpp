float seedDustRadius = 0.02f;
//float seedDustNumPerSite = 15;
//float seedDustNumPerSiteCollisionEvent = 3;
float seedDustNumPerSite = 20;
float seedDustNumPerSiteCollisionEvent = 4.5f;

float applyForcePointAreaPerSample = 0.005f;
float applyForceFromPointsDrag = 0.1f;
//float noise3DScale = 1.5f;
//float noise2DScale = 0.5f;
float noiseKIsoDecay = 0.99f;
float noiseKAniDecay = 0.99f;
float noise3DScale = 0.75f*0.5f;
float noise2DScale = 0.32f*0.5f;
float dustMaxLife = 5.0f;
float dustMinLife = 1.0f;
float dustParticleOpacity = 0.1f;
float dustParticleRenderRadius = 0.1f;
float particleStartFade = 5.0f;
float octaveScaling = 7.0f;
float divStrength = 5.0f;
float curDivStrength = 0.0f;
float divStrengthReductionRate = 8.0f/30;

bool doGaussianBlur = false;
float blurVelSigmaFDX = 1.0f;
float blurVelCenterFactor = 30.0f;
float curlScale = 10.0f;

float areaPerDustSample = 0.005f;
float areaPerDebris = 0.02f;

float minSizeDecayRate = 1.0f;
float explodeVel = 3.0f;
float numParPerMeteor = 3;

float minMeteorDustLife = 2.0f;
float maxMeteorDustLife = 2.3f;
float minMeteorDustSize = 0.5f;
float maxMeteorDustSize = 1.5f;
float minMeteorSizeDecayRate = 1.0f;

int maxNumDebrisPerType = 500; // Maximum number of debris per each type of debris
float sleepingThresholdRB = 0.1f;
float sleepingThresholdParticles = 0.1f;
int maxNumDebrisToAdd = 100;