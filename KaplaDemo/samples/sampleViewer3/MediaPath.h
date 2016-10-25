#ifndef __MEDIA_PATH__
#define __MEDIA_PATH__

/*
This function locates the media associated with the sample in a non platform specific way.

For example it searches the Samples\Media directory on the PC (assuming the exe is in the bin directory or
or project directory after a compile). This helps prevent duplicate resources for samples.

On the XBox360 the game specific drive alias is added to the path(ie d:\).

On PS3 no change is made.

	fname - File name of media file
	buff - buffer to put media path in
	return - buff.
*/
char* FindMediaFile(const char *fname,char *buff);

char* FindMediaFile(const char *fname, const char* path);

char* GetTempFilePath(char *buff);

#endif
