#include <stdio.h>
#define NOMINMAX
#include <windows.h>
#include "Timing.h"


unsigned long getTime()
{
	return timeGetTime();
}


float getCurrentTime()
{
	unsigned int currentTime = timeGetTime();
	return (float)(currentTime)*0.001f;
}


float getElapsedTime()
{
	static LARGE_INTEGER previousTime;
	static LARGE_INTEGER freq;
	static bool init = false;
	if(!init){
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&previousTime);
		init=true;
	}
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	unsigned long long elapsedTime = currentTime.QuadPart - previousTime.QuadPart;
	previousTime = currentTime;
	return (float)(elapsedTime)/(freq.QuadPart);
}

