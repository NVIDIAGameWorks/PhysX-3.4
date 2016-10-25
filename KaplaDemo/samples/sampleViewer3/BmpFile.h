#ifndef BMP_LOADER_H
#define BMP_LOADER_H

class BmpLoaderBuffer
{
public:

	BmpLoaderBuffer(void);
	~BmpLoaderBuffer(void);

	// Data is persists until the class is destructed.
	bool loadFile(const char *filename); 

	int	           mWidth;
	int	           mHeight;
	unsigned char *mRGB;
};

bool saveBmpRBG(const char *filename, int width, int height, void *data);

#endif
