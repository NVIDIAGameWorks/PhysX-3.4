/*----------------------------------------------------------------------------*\
|
|						     AGEIA PhysX Technology
|
|							     www.ageia.com
|
\*----------------------------------------------------------------------------*/

#ifndef __GL_FONT_RENDERER__
#define __GL_FONT_RENDERER__

#include "foundation/PxVec3.h"
#include "PsArray.h"

using namespace physx;

enum FontColor
{
	FNT_COLOR_BLUE			= 0xffff0000,
	FNT_COLOR_GREEN			= 0xff00ff00,
	FNT_COLOR_RED			= 0xff0000ff,

	FNT_COLOR_DARK_BLUE		= 0xff800000,
	FNT_COLOR_DARK_GREEN	= 0xff008000,
	FNT_COLOR_DARK_RED		= 0xff000080,

	FNT_COLOR_WHITE			= 0xffffffff
};

struct GLFontMeasureResult
{
	float width;
	float height;
	GLFontMeasureResult( float w, float h )
		: width( w )
		, height( h )
	{
	}
};

class GLFontRenderer{
	
private:

	bool m_isInit;
	unsigned int m_textureObject;
	int m_screenWidth;
	int m_screenHeight;
	unsigned int m_color;
	shdfnd::Array<PxF32>	mVertList;
	shdfnd::Array<PxF32>	mTextureCoordList;
	template<typename TOperator>
	void print( float fontSize, const char* pString, bool forceMonoSpace, int monoSpaceWidth, bool doOthoProj, TOperator inOperator );

public:
	GLFontRenderer() : m_isInit(false), m_textureObject(0), m_screenWidth(0), m_screenHeight(0) {}
	
	bool init();
	void print(float x, float y, float fontSize, const char* pString, bool forceMonoSpace=false, int monoSpaceWidth=11, bool doOrthoProj=true);
	GLFontMeasureResult measure( float fontSize, const char* pString, bool forceMonoSpace=false, int monoSpaceWidth=11 );

	void print3d(const physx::PxVec3& pos, const physx::PxVec3& cameraDir, const physx::PxVec3& up, float fontSize, const char* pString, bool forceMonoSpace=false, int monoSpaceWidth=11);
	void setScreenResolution(int screenWidth, int screenHeight);
	void getScreenResolution( int& screenWidth, int& screenHeight ) { screenWidth = m_screenWidth; screenHeight = m_screenHeight; }

	// PT: contrary to what the comment said before the format is abgr:
	// 0xffff0000 = blue
	// 0xff00ff00 = green
	// 0xff0000ff = red
	void setColor(unsigned int abgr);
};

#endif // __GL_FONT_RENDERER__
