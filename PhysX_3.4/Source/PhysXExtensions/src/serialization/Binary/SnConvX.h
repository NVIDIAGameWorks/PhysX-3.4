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

#ifndef PX_CONVX_H
#define PX_CONVX_H

#include "foundation/PxErrors.h"
#include "PxBinaryConverter.h"
#include "PxTypeInfo.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PsArray.h"
#include "SnConvX_Common.h"
#include "SnConvX_Union.h"
#include "SnConvX_MetaData.h"
#include "SnConvX_Align.h"

#define CONVX_ZERO_BUFFER_SIZE	256

namespace physx { 
	
class PxSerializationRegistry;

namespace Sn {

	struct HeightFieldData;
	class PointerRemap
	{
		public:
				PointerRemap();
				~PointerRemap();

		bool	checkRefIsNotUsed(PxU32 ref)	const;
		void	setObjectRef(PxU64 object64, PxU32 ref);
		bool	getObjectRef(PxU64 object64, PxU32& ref)	const;

		struct InternalData
		{
			PxU64	object;
			PxU32	id;
		};

		Ps::Array<InternalData>	mData;
	};

	class ConvX : public physx::PxBinaryConverter, public shdfnd::UserAllocated
	{
		public:
												ConvX();
		virtual									~ConvX();

		virtual			void					release();
		virtual			void					setReportMode(PxConverterReportMode::Enum mode)	{ mReportMode = mode;	}
		PX_FORCE_INLINE	bool					silentMode()	const	{ return mReportMode==PxConverterReportMode::eNONE;		}
		PX_FORCE_INLINE	bool					verboseMode()	const	{ return mReportMode==PxConverterReportMode::eVERBOSE;	}

		virtual			bool					setMetaData(PxInputStream& srcMetaData, PxInputStream& dstMetaData);
		virtual			bool					convert(PxInputStream& srcStream, PxU32 srcSize, PxOutputStream& targetStream);
		
	private:
						ConvX&					operator=(const ConvX&);
						bool					setMetaData(PxInputStream& inputStream, MetaDataType type);

			// Meta-data
						void					releaseMetaData();
						const MetaData*			loadMetaData(PxInputStream& inputStream, MetaDataType type);
						const MetaData*			getBinaryMetaData(MetaDataType type);
						int						getNbMetaClasses(MetaDataType type);
						MetaClass*				getMetaClass(unsigned int i, MetaDataType type)				const;
						MetaClass*				getMetaClass(const char* name, MetaDataType type)	const;
						MetaClass*				getMetaClass(PxConcreteType::Enum concreteType, MetaDataType type);
						MetaData*				mMetaData_Src;
						MetaData*				mMetaData_Dst;

			// Convert
						
						bool					convert(const void* buffer, int fileSize);
						void					resetConvexFlags();
						void					_enumerateFields(const MetaClass* mc, ExtraDataEntry2* entries, int& nb, int baseOffset, MetaDataType type)	const;
						void					_enumerateExtraData(const char* address, const MetaClass* mc, ExtraDataEntry* entries, int& nb, int offset, MetaDataType type)	const;
						PxU64                   read64(const void*& buffer);
						int						read32(const void*& buffer);
						short                   read16(const void*& buffer);
						bool					convertClass(const char* buffer, const MetaClass* mc, int offset);
						const char*				convertExtraData_Array(const char* Address, const char* lastAddress, const char* objectAddress, const ExtraDataEntry& ed);
						const char*				convertExtraData_Ptr(const char* Address, const char* lastAddress, const PxMetaDataEntry& entry, int count, int ptrSize_Src, int ptrSize_Dst);
						int						getConcreteType(const char* buffer);
						bool					convertCollection(const void* buffer, int fileSize, int nbObjects);
						const void*				convertManifestTable(const void* buffer, int& fileSize);
						const void*				convertImportReferences(const void* buffer, int& fileSize);
						const void*				convertExportReferences(const void* buffer, int& fileSize);
						const void*				convertInternalReferences(const void* buffer, int& fileSize);
						const void*				convertReferenceTables(const void* buffer, int& fileSize, int& nbObjectsInCollection);
						bool					checkPaddingBytes(const char* buffer, int byteCount);

			// ---- big convex surgery ----
						PsArray<bool>			mConvexFlags;
			// Align
						const char*				alignStream(const char* buffer, int alignment=ALIGN_DEFAULT);
						void					alignTarget(int alignment);

						char					mZeros[CONVX_ZERO_BUFFER_SIZE];
			// Unions
						bool					registerUnion(const char* name);
						bool					registerUnionType(const char* unionName, const char* typeName, int typeValue);
						const char*				getTypeName(const char* unionName, int typeValue);
						void					resetUnions();
						PsArray<Union>			mUnions;
			// Output
						void					setNullPtr(bool);
						void					setNoOutput(bool);
						bool					initOutput(PxOutputStream& targetStream);
						void					closeOutput();
						int						getCurrentOutputSize();
						void					output(short value);
						void					output(int value);
						void					output(PxU64 value);
						void					output(const char* buffer, int nbBytes);
						void					convert8	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						void					convertPad8	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						void					convert16	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						void					convert32	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						void					convert64	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						void					convertFloat(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						void					convertPtr	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);
						PxOutputStream*			mOutStream;
						bool					mMustFlip;
						int						mOutputSize;
						int						mSrcPtrSize;
						int						mDstPtrSize;
						bool					mNullPtr;
						bool					mNoOutput;
						bool					mMarkedPadding;

			// Errors
						void					resetNbErrors();
						int						getNbErrors()	const;
						void					displayMessage(physx::PxErrorCode::Enum code, const char* format, ...);
						int						mNbErrors;
						int						mNbWarnings;

			// Settings						
						PxConverterReportMode::Enum		mReportMode;
						bool							mPerformConversion;

			// Remap pointers
						void					exportIntAsPtr(int value);
						void					exportInt(int value);
						void					exportInt64(PxU64 value);
						PointerRemap			mRemap;
						PointerRemap*			mActiveRemap;
						PxU32					mPointerRemapCounter;

		friend class MetaData;
		friend struct MetaClass;
	};
} }
#endif
