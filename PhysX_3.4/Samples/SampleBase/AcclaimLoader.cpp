// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2017 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include <stdio.h>
#include "AcclaimLoader.h"
#include "FrameworkFoundation.h"
#include "PsMathUtils.h"
#include "PxTkFile.h"
#include "SampleAllocatorSDKClasses.h"
#include "SampleArray.h"

#define MAX_FILE_BUFFER_SIZE 4096
#define MAX_TOKEN_LENGTH 512

///////////////////////////////////////////////////////////////////////////////
static inline bool isWhiteSpace(int c)
{
	return ( c == ' ') || (c == '\t');
}

///////////////////////////////////////////////////////////////////////////////
static inline bool isWhiteSpaceAndNewline(int c)
{		
	return ( c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') ;
}

///////////////////////////////////////////////////////////////////////////////
static inline bool isNumeric(int c)
{
	return ('0' <= c) && (c <= '9');
}

///////////////////////////////////////////////////////////////////////////////
class SampleFileBuffer
{
	SampleFramework::File*	mFP;
	char					mBuffer[MAX_FILE_BUFFER_SIZE];
	int						mCurrentBufferSize;
	int						mCurrentCounter;
	int						mEOF;

public:
	SampleFileBuffer(SampleFramework::File* fp) : 
		mFP(fp), 
		mCurrentBufferSize(0),
		mCurrentCounter(0),
		mEOF(0)
		{} 

	///////////////////////////////////////////////////////////////////////////
	inline void rewind(int offset = 1)
	{
		mCurrentCounter -= offset;
	}

	///////////////////////////////////////////////////////////////////////////
	void readBuffer()
	{
		mCurrentBufferSize = (int)fread(mBuffer, 1, MAX_FILE_BUFFER_SIZE, mFP);
		mEOF = feof(mFP);
		mCurrentCounter = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	char getCharacter()
	{
		if (mCurrentCounter >= mCurrentBufferSize)
		{
			if (mEOF) return EOF;
			readBuffer();
			if (mCurrentBufferSize == 0)
				return EOF;
		}
		return mBuffer[mCurrentCounter++];
	}

	///////////////////////////////////////////////////////////////////////////
	bool skipWhiteSpace(bool stopAtEndOfLine)
	{
		char c = 0;
		do 
		{
			c = getCharacter();
			bool skip = (stopAtEndOfLine) ? isWhiteSpace(c) : isWhiteSpaceAndNewline(c);
			if (skip == false)
			{
				rewind();
				return true;
			}
		} while (c != EOF);

		return false; // end of file
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextToken(char* token, bool stopAtEndOfLine)
	{
		if (skipWhiteSpace(stopAtEndOfLine) == false)
			return false;

		char* str = token;
		char c = 0;
		do
		{
			c = getCharacter();
			if (c == EOF)
			{
				*str = 0;
			}
			else if (isWhiteSpaceAndNewline(c) == true)
			{
				*str = 0;
				rewind();
				return (strlen(token) > 0);
			}
			else
				*str++ = (char) c;
		} while (c != EOF);

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextTokenButMarker(char* token)
	{
		if (skipWhiteSpace(false) == false)
			return 0;

		char* str = token;
		char c = 0;
		do
		{
			c = getCharacter();
			if (c == ':')
			{
				rewind();
				*str = 0;
				return false;
			}

			if (c == EOF)
			{
				*str = 0;
			}
			else if (isWhiteSpaceAndNewline(c) == true)
			{
				*str = 0;
				rewind();
				return (strlen(token) > 0);
			}
			else
				*str++ = (char) c;
		} while (c != EOF);

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextTokenButNumeric(char* token)
	{
		if (skipWhiteSpace(false) == false)
			return 0;

		char* str = token;
		char c = 0;
		do
		{
			c = getCharacter();
			if (isNumeric(c))
			{
				rewind();
				*str = 0;
				return false;
			}

			if (c == EOF)
			{
				*str = 0;
			}
			else if (isWhiteSpaceAndNewline(c) == true)
			{
				*str = 0;
				rewind();
				return (strlen(token) > 0);
			}
			else
				*str++ = (char) c;
		} while (c != EOF);

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
	void skipUntilNextLine()
	{
		char c = 0;
		do
		{
			c = getCharacter();
		} while ((c != '\n') && (c != EOF));
	}

	///////////////////////////////////////////////////////////////////////////////
	void skipUntilNextBlock()
	{
		char dummy[MAX_TOKEN_LENGTH];
		while (getNextTokenButMarker(dummy) == true) 
			;
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextFloat(float& val, bool stopAtEndOfLine = true)
	{
		char dummy[MAX_TOKEN_LENGTH];
		if (getNextToken(dummy, stopAtEndOfLine) == false)
			return false;

		val = float(atof(dummy));
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextInt(int& val, bool stopAtEndOfLine = true)
	{
		char dummy[MAX_TOKEN_LENGTH];
		if (getNextToken(dummy, stopAtEndOfLine) == false)
			return false;

		val = int(atoi(dummy));
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextString(char* val, bool stopAtEndOfLine = true)
	{
		char dummy[MAX_TOKEN_LENGTH];

		if (getNextToken(dummy, stopAtEndOfLine) == false)
			return false;

		strcpy(val, dummy);
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////
	bool getNextVec3(PxVec3& val, bool stopAtEndOfLine = true)
	{
		if (getNextFloat(val.x, stopAtEndOfLine) == false)
			return false;

		if (getNextFloat(val.y, stopAtEndOfLine) == false)
			return false;

		if (getNextFloat(val.z, stopAtEndOfLine) == false)
			return false;

		return true;
	}
};

///////////////////////////////////////////////////////////////////////////////
static bool readHeader(SampleFileBuffer& buffer, Acclaim::ASFData& data)
{
	using namespace Acclaim;

	char token[MAX_TOKEN_LENGTH], value[MAX_TOKEN_LENGTH];

	while (buffer.getNextTokenButMarker(token) == true)
	{

		if (strcmp(token, "mass") == 0)
		{
			if (buffer.getNextFloat(data.mHeader.mMass) == false)
				return false;
		}
		else if (strcmp(token, "length") == 0)
		{
			if (buffer.getNextFloat(data.mHeader.mLengthUnit) == false)
				return false;
		}
		else if (strcmp(token, "angle") == 0)
		{
			if (buffer.getNextToken(value, true) == false)
				return false;

			data.mHeader.mAngleInDegree = (strcmp(value, "deg") == 0);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
static bool readRoot(SampleFileBuffer& buffer, Acclaim::ASFData& data)
{
	using namespace Acclaim;

	char token[MAX_TOKEN_LENGTH];

	while (buffer.getNextTokenButMarker(token) == true)
	{
		if (strcmp(token, "order") == 0)
			buffer.skipUntilNextLine();
		else if (strcmp(token, "axis") == 0)
			buffer.skipUntilNextLine();
		else if (strcmp(token, "position") == 0)
		{
			if (buffer.getNextVec3(data.mRoot.mPosition) == false)
				return false;
		}
		else if (strcmp(token, "orientation") == 0)
		{
			if (buffer.getNextVec3(data.mRoot.mOrientation) == false)
				return false;
		}	
		else
		{
			buffer.skipUntilNextLine();
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
static bool readBone(SampleFileBuffer& buffer, Acclaim::Bone& bone)
{
	using namespace Acclaim;

	int nbDOF = 0;
	char token[MAX_TOKEN_LENGTH], dummy[MAX_TOKEN_LENGTH];

	if (buffer.getNextTokenButMarker(token) == false)
		return false;

	if (strcmp(token, "begin") != 0)
		return false;

	while (buffer.getNextToken(token, false) == true)
	{
		if (strcmp(token, "id") == 0)
		{
			if (buffer.getNextInt(bone.mID) == false) 	return false;
		}
		else if (strcmp(token, "name") == 0)
		{
			if (buffer.getNextString(bone.mName) == false) 	return false;
		}
		else if (strcmp(token, "direction") == 0)
		{
			if (buffer.getNextVec3(bone.mDirection) == false) 	return false;
		}
		else if (strcmp(token, "length") == 0)
		{
			if (buffer.getNextFloat(bone.mLength) == false) 	return false;
		}
		else if (strcmp(token, "axis") == 0)
		{
			if (buffer.getNextVec3(bone.mAxis) == false) 	return false;
			buffer.getNextToken(dummy, true);
		}
		else if (strcmp(token, "dof") == 0)
		{
			while ((buffer.getNextToken(dummy, true) == true))
			{
				if (strcmp(dummy, "rx") == 0)
				{
					bone.mDOF |= BoneDOFFlag::eRX;
					nbDOF++;					
				}
				else if (strcmp(dummy, "ry") == 0)
				{
					bone.mDOF |= BoneDOFFlag::eRY;
					nbDOF++;					
				}
				else if (strcmp(dummy, "rz") == 0)
				{
					bone.mDOF |= BoneDOFFlag::eRZ;
					nbDOF++;					
				}
				else if (strcmp(dummy, "l") == 0)
				{
					bone.mDOF |= BoneDOFFlag::eLENGTH;
					nbDOF++;					
				}

			}
			continue;
		}
		else if (strcmp(token, "limits") == 0)
		{
			int cnt = 0;
			while ( cnt++ < nbDOF)
			{
				// we ignore limit data for now
				if (buffer.getNextToken(dummy, false) == false) return false;
				if (buffer.getNextToken(dummy, false) == false) return false;
			}
		}
		else if (strcmp(token, "end") == 0)
			break;
		else
			buffer.skipUntilNextLine();
		
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
static bool readBoneData(SampleFileBuffer& buffer, Acclaim::ASFData& data)
{
	using namespace Acclaim;

	Bone tempBones[MAX_BONE_NUMBER];
	PxU32 nbBones = 0;

	// read all the temporary bones onto temporary buffer
	bool moreBone = false;
	do {
		moreBone = readBone(buffer, tempBones[nbBones]);
		if (moreBone)
			nbBones++;

		PX_ASSERT(nbBones <= MAX_BONE_NUMBER);
	} while (moreBone == true);

	// allocate the right size and copy the bone data
	data.mBones = (Bone*)malloc(sizeof(Bone) * nbBones);
	data.mNbBones = nbBones;

	for (PxU32 i = 0; i < nbBones; i++)
	{
		data.mBones[i] = tempBones[i];
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
static Acclaim::Bone* getBoneFromName(Acclaim::ASFData& data, const char* name)
{
	// use a simple linear search -> probably we could use hash map if performance is an issue 
	for (PxU32 i = 0; i < data.mNbBones; i++)
	{
		if (strcmp(name, data.mBones[i].mName) == 0)
			return &data.mBones[i];
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
static bool readHierarchy(SampleFileBuffer& buffer, Acclaim::ASFData& data)
{
	using namespace Acclaim;

	char token[MAX_TOKEN_LENGTH];
	char dummy[MAX_TOKEN_LENGTH];

	while (buffer.getNextTokenButMarker(token) == true)
	{
		if (strcmp(token, "begin") == 0)
			;
		else if (strcmp(token, "end") == 0)
			break;
		else
		{				
			Bone* parent = getBoneFromName(data, token);

			while (buffer.getNextToken(dummy, true) == true)
			{
				Bone* child = getBoneFromName(data, dummy);
				if (!child)
					return false;

				child->mParent = parent;
			}
		}

		buffer.skipUntilNextLine();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
static bool readFrameData(SampleFileBuffer& buffer, Acclaim::ASFData& asfData, Acclaim::FrameData& frameData)
{
	using namespace Acclaim;
	char token[MAX_TOKEN_LENGTH];

	while (buffer.getNextTokenButNumeric(token) == true)
	{
		if (strcmp(token, "root") == 0)
		{
			buffer.getNextVec3(frameData.mRootPosition);
			buffer.getNextVec3(frameData.mRootOrientation);
		}
		else
		{
			Bone* bone = getBoneFromName(asfData, token);

			if (bone == 0)
				return false;

			int id = bone->mID - 1; 
			float val = 0;
			if (bone->mDOF & BoneDOFFlag::eRX)
			{

				buffer.getNextFloat(val);
				frameData.mBoneFrameData[id].x = val;
			}
			if (bone->mDOF & BoneDOFFlag::eRY)
			{

				buffer.getNextFloat(val);
				frameData.mBoneFrameData[id].y = val;
			}
			if (bone->mDOF & BoneDOFFlag::eRZ)
			{

				buffer.getNextFloat(val);
				frameData.mBoneFrameData[id].z = val;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool Acclaim::readASFData(const char* filename, Acclaim::ASFData& data)
{
	using namespace Acclaim;

	char token[MAX_TOKEN_LENGTH];

	SampleFramework::File* fp = NULL;
	PxToolkit::fopen_s(&fp, filename, "r");

	if (!fp)
		return false;

	SampleFileBuffer buffer(fp);

	while (buffer.getNextToken(token, false) == true)
	{
		if (token[0] == '#') // comment
		{
			buffer.skipUntilNextLine();
			continue;
		}
		else if (token[0] == ':') // blocks
		{
			const char* str = token + 1; // remainder of the string

			if (strcmp(str, "version") == 0) // ignore version number
				buffer.skipUntilNextLine();
			else if (strcmp(str, "name") == 0) // probably 'VICON' 
				buffer.skipUntilNextLine();
			else if (strcmp(str, "units") == 0) 
			{
				if ( readHeader(buffer, data) == false)
					return false;
			}
			else if (strcmp(str, "documentation") == 0)
				buffer.skipUntilNextBlock();
			else if (strcmp(str, "root") == 0)
			{
				if (readRoot(buffer, data) == false)
					return false;
			}
			else if (strcmp(str, "bonedata") == 0)
			{
				if (readBoneData(buffer, data) == false)
					return false;
			}
			else if (strcmp(str, "hierarchy") == 0)
			{
				if (readHierarchy(buffer, data) == false)
					return false;
			}
			else
			{
				// ERROR! - unrecognized block name
			}
		}
		else
		{
			// ERRROR!
			continue;
		}
	}


	fclose(fp);

	return true;
}



///////////////////////////////////////////////////////////////////////////////
bool Acclaim::readAMCData(const char* filename, Acclaim::ASFData& asfData, Acclaim::AMCData& amcData)
{
	using namespace Acclaim;

	char token[MAX_TOKEN_LENGTH];

	SampleArray<FrameData> tempFrameData;
	tempFrameData.reserve(300);

	SampleFramework::File* fp = NULL;
	PxToolkit::fopen_s(&fp, filename, "r");

	if (!fp)
		return false;

	SampleFileBuffer buffer(fp);

	while (buffer.getNextToken(token, false) == true)
	{
		if (token[0] == '#') // comment
		{
			buffer.skipUntilNextLine();
			continue;
		}
		else if (token[0] == ':') // blocks
		{
			const char* str = token + 1; // remainder of the string

			if (strcmp(str, "FULLY-SPECIFIED") == 0)
				continue;
			else if (strcmp(str, "DEGREES") == 0) 
				continue;
		}
		else if (isNumeric(token[0]) == true)
		{
			// frame number
			//int frameNo = atoi(token);

			FrameData frameData;
			if (readFrameData(buffer, asfData, frameData) == true)
				tempFrameData.pushBack(frameData);
		}
	}

	amcData.mNbFrames = tempFrameData.size();

	amcData.mFrameData = (FrameData*)malloc(sizeof(FrameData) * amcData.mNbFrames);
	memcpy(amcData.mFrameData, tempFrameData.begin(), sizeof(FrameData) * amcData.mNbFrames);

	fclose(fp);

	return true;
}

