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

#ifndef PX_CONVX_METADATA_H
#define PX_CONVX_METADATA_H

#include "SnConvX_Output.h"
#include "PxMetaDataFlags.h"

namespace physx { namespace Sn {

#if PX_VC
#pragma warning (push)
#pragma warning (disable : 4371) //layout of class may have changed from a previous version of the compiler due to better packing of member
#endif


	// PT: beware, must match corresponding structure in PxMetaData.h
	struct PxMetaDataEntry : public shdfnd::UserAllocated
	{
		PxMetaDataEntry()
		{
			memset(this, 0, sizeof(*this));
		}
		bool			isVTablePtr()	const;

		const char*		mType;			//!< Field type (bool, byte, quaternion, etc)
		const char*		mName;			//!< Field name (appears exactly as in the source file)
		int				mOffset;		//!< Offset from the start of the class (ie from "this", field is located at "this"+Offset)
		int				mSize;			//!< sizeof(Type)
		int				mCount;			//!< Number of items of type Type (0 for dynamic sizes)
		int				mOffsetSize;	//!< Offset of dynamic size param, for dynamic arrays
		int				mFlags;			//!< Field parameters
		int				mAlignment;		//!< Explicit alignment added for DE1340
	};

	struct MetaDataEntry32
	{
		PxI32			mType;			//!< Field type (bool, byte, quaternion, etc)
		PxI32			mName;			//!< Field name (appears exactly as in the source file)
		int				mOffset;		//!< Offset from the start of the class (ie from "this", field is located at "this"+Offset)
		int				mSize;			//!< sizeof(Type)
		int				mCount;			//!< Number of items of type Type (0 for dynamic sizes)
		int				mOffsetSize;	//!< Offset of dynamic size param, for dynamic arrays
		int				mFlags;			//!< Field parameters
		int				mAlignment;		//!< Explicit alignment added for DE1340
	};

	struct MetaDataEntry64
	{
		PxI64			mType;			//!< Field type (bool, byte, quaternion, etc)
		PxI64			mName;			//!< Field name (appears exactly as in the source file)
		int				mOffset;		//!< Offset from the start of the class (ie from "this", field is located at "this"+Offset)
		int				mSize;			//!< sizeof(Type)
		int				mCount;			//!< Number of items of type Type (0 for dynamic sizes)
		int				mOffsetSize;	//!< Offset of dynamic size param, for dynamic arrays
		int				mFlags;			//!< Field parameters
		int				mAlignment;		//!< Explicit alignment added for DE1340
	};

	struct ExtraDataEntry
	{
		PxMetaDataEntry	entry;
		int				offset;
	};

	struct ExtraDataEntry2 : ExtraDataEntry
	{
		ConvertCallback	cb;
	};

	class MetaData;

	struct MetaClass : public shdfnd::UserAllocated
	{
		bool					getFieldByType(const char* type, PxMetaDataEntry& entry)	const;
		bool					getFieldByName(const char* name, PxMetaDataEntry& entry)	const;
		bool					check(const MetaData& owner);

		ConvertCallback			mCallback;
		MetaClass*				mMaster;
		const char*				mClassName;
		int						mSize;
		int						mDepth;
		PsArray<PxMetaDataEntry>	mBaseClasses;
		PsArray<PxMetaDataEntry>	mFields;
		bool					mProcessed;

//		int						mNbEntries;
//		ExtraDataEntry2			mEntries[256];

		private:
		void					checkAndCompleteClass(const MetaData& owner, int& startOffset, int& nbBytes);
	};

	enum MetaDataType
	{
		META_DATA_NONE,
		META_DATA_SRC,
		META_DATA_DST
	};

	class ConvX;
	class MetaData : public shdfnd::UserAllocated
	{
		public:
										MetaData(Sn::ConvX&);
										~MetaData();
		
				bool					load(PxInputStream& inputStream, MetaDataType type);

		inline_	MetaDataType			getType()							const	{ return mType;						}
		inline_ int						getVersion()						const	{ return mVersion;					}
		inline_ int						getBuildNumber()					const	{ return mBuildNumber;				}
		inline_	int						getPtrSize()						const	{ return mSizeOfPtr;				}
		inline_	int						getPlatformTag()					const	{ return mPlatformTag;				}
		inline_	int						getGaussMapLimit()					const	{ return mGaussMapLimit;			}
		inline_	int						getNbMetaClasses()					const	{ return int(mMetaClasses.size());	}
		inline_	MetaClass*				getMetaClass(unsigned int i)		const	{ return mMetaClasses[i];			}
		inline_	bool					getFlip()							const	{ return mFlip;						}

				MetaClass*				getMetaClass(const char* name)	const;
				MetaClass*				getMetaClass(PxConcreteType::Enum concreteType)	const;
				MetaClass*				addNewClass(const char* name, int size, MetaClass* master=NULL, ConvertCallback callback=NULL);
		private:
				MetaData&				operator=(const MetaData&);
			Sn::ConvX&					mConvX;
				MetaDataType			mType;
				int						mNbEntries;
				PxMetaDataEntry*		mEntries;
				char*					mStringTable;
				PsArray<MetaClass*>		mMetaClasses;
				int						mVersion;
				int						mBinaryVersion;
				int						mBuildNumber;
				int						mSizeOfPtr;
				int						mPlatformTag;
				int						mGaussMapLimit;
				bool					mFlip;

				PsArray< Ps::Pair<PxConcreteType::Enum, PxU32> > mConcreteTypeTable;

		inline_ const char*				offsetToText(const char* text)	const
										{
											const size_t offset = size_t(text);
											const PxU32 offset32 = PxU32(offset);
//											if(offset==-1)
											if(offset32==0xffffffff)
												return NULL;
											return mStringTable + offset32;
										}
		friend struct MetaClass;
	};

	PxU64	peek(int size, const char* buffer, int flags=0);

#if PX_VC
#pragma warning (pop)
#endif
} }

#endif
