#pragma once
#include <cstdio>
#include <cstdlib>
#include "jpeglib.h"
#include "jerror.h"

class CPPJPEGWrapper{
public:
	CPPJPEGWrapper();
	~CPPJPEGWrapper();
	void Unload();

	unsigned char* GetData() const;
	unsigned GetWidth();
	unsigned GetHeight();
	unsigned GetBPP();
	unsigned GetChannels();


	bool LoadJPEG(const char* FileName, bool Fast = true);
protected:
	//if the jpeglib stuff isnt after I think stdlib then it wont work just put it at the end
	unsigned long x;
	unsigned long y;
	unsigned short int bpp; //bits per pixels   unsigned short int
	unsigned char* data;             //the data of the image
	unsigned int ID;                //the id ogl gives it
	unsigned long size;     //length of the file
	int channels;      //the channels of the image 3 = RGA 4 = RGBA


};
