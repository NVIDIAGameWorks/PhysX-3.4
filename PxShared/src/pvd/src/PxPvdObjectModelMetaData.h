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
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
#ifndef PXPVDSDK_PXPVDOBJECTMODELMETADATA_H
#define PXPVDSDK_PXPVDOBJECTMODELMETADATA_H

#include "foundation/PxAssert.h"
#include "PxPvdObjectModelBaseTypes.h"
#include "PxPvdBits.h"

namespace physx
{
namespace pvdsdk
{

class PvdInputStream;
class PvdOutputStream;

struct PropertyDescription
{
	NamespacedName mOwnerClassName;
	int32_t mOwnerClassId;
	String mName;
	String mSemantic;
	// The datatype this property corresponds to.
	int32_t mDatatype;
	// The name of the datatype
	NamespacedName mDatatypeName;
	// Scalar or array.
	PropertyType::Enum mPropertyType;
	// No other property under any class has this id, it is DB-unique.
	int32_t mPropertyId;
	// Offset in bytes into the object's data section where this property starts.
	uint32_t m32BitOffset;
	// Offset in bytes into the object's data section where this property starts.
	uint32_t m64BitOffset;

	PropertyDescription(const NamespacedName& clsName, int32_t classId, String name, String semantic, int32_t datatype,
	                    const NamespacedName& datatypeName, PropertyType::Enum propType, int32_t propId,
	                    uint32_t offset32, uint32_t offset64)
	: mOwnerClassName(clsName)
	, mOwnerClassId(classId)
	, mName(name)
	, mSemantic(semantic)
	, mDatatype(datatype)
	, mDatatypeName(datatypeName)
	, mPropertyType(propType)
	, mPropertyId(propId)
	, m32BitOffset(offset32)
	, m64BitOffset(offset64)
	{
	}
	PropertyDescription()
	: mOwnerClassId(-1)
	, mName("")
	, mSemantic("")
	, mDatatype(-1)
	, mPropertyType(PropertyType::Unknown)
	, mPropertyId(-1)
	, m32BitOffset(0)
	, m64BitOffset(0)

	{
	}

	virtual ~PropertyDescription()
	{
	}
};

struct PtrOffsetType
{
	enum Enum
	{
		UnknownOffset,
		VoidPtrOffset,
		StringOffset
	};
};

struct PtrOffset
{
	PtrOffsetType::Enum mOffsetType;
	uint32_t mOffset;
	PtrOffset(PtrOffsetType::Enum type, uint32_t offset) : mOffsetType(type), mOffset(offset)
	{
	}
	PtrOffset() : mOffsetType(PtrOffsetType::UnknownOffset), mOffset(0)
	{
	}
};

inline uint32_t align(uint32_t offset, uint32_t alignment)
{
	uint32_t startOffset = offset;
	uint32_t alignmentMask = ~(alignment - 1);
	offset = (offset + alignment - 1) & alignmentMask;
	PX_ASSERT(offset >= startOffset && (offset % alignment) == 0);
	(void)startOffset;
	return offset;
}

struct ClassDescriptionSizeInfo
{
	// The size of the data section of this object, padded to alignment.
	uint32_t mByteSize;
	// The last data member goes to here.
	uint32_t mDataByteSize;
	// Alignment in bytes of the data section of this object.
	uint32_t mAlignment;
	// the offsets of string handles in the binary value of this class
	DataRef<PtrOffset> mPtrOffsets;
	ClassDescriptionSizeInfo() : mByteSize(0), mDataByteSize(0), mAlignment(0)
	{
	}
};

struct ClassDescription
{
	NamespacedName mName;
	// No other class has this id, it is DB-unique
	int32_t mClassId;
	// Only single derivation supported.
	int32_t mBaseClass;
	// If this class has properties that are of uniform type, then we note that.
	// This means that when deserialization an array of these objects we can just use
	// single function to endian convert the entire mess at once.
	int32_t mPackedUniformWidth;
	// If this class is composed uniformly of members of a given type
	// Or all of its properties are composed uniformly of members of
	// a give ntype, then this class's packed type is that type.
	// PxTransform's packed type would be float.
	int32_t mPackedClassType;
	// 0: 32Bit 1: 64Bit
	ClassDescriptionSizeInfo mSizeInfo[2];
	// No further property additions allowed.
	bool mLocked;
	// True when this datatype has an array on it that needs to be
	// separately deleted.
	bool mRequiresDestruction;

	ClassDescription(NamespacedName name, int32_t id)
	: mName(name)
	, mClassId(id)
	, mBaseClass(-1)
	, mPackedUniformWidth(-1)
	, mPackedClassType(-1)
	, mLocked(false)
	, mRequiresDestruction(false)
	{
	}
	ClassDescription()
	: mClassId(-1), mBaseClass(-1), mPackedUniformWidth(-1), mPackedClassType(-1), mLocked(false), mRequiresDestruction(false)
	{
	}
	virtual ~ClassDescription()
	{
	}

	ClassDescriptionSizeInfo& get32BitSizeInfo()
	{
		return mSizeInfo[0];
	}
	ClassDescriptionSizeInfo& get64BitSizeInfo()
	{
		return mSizeInfo[1];
	}
	uint32_t& get32BitSize()
	{
		return get32BitSizeInfo().mByteSize;
	}
	uint32_t& get64BitSize()
	{
		return get64BitSizeInfo().mByteSize;
	}

	uint32_t get32BitSize() const
	{
		return mSizeInfo[0].mByteSize;
	}
	const ClassDescriptionSizeInfo& getNativeSizeInfo() const
	{
		return mSizeInfo[(sizeof(void*) >> 2) - 1];
	}
	uint32_t getNativeSize() const
	{
		return getNativeSizeInfo().mByteSize;
	}
};

struct MarshalQueryResult
{
	int32_t srcType;
	int32_t dstType;
	// If canMarshal != needsMarshalling we have a problem.
	bool canMarshal;
	bool needsMarshalling;
	// Non null if marshalling is possible.
	TBlockMarshaller marshaller;
	MarshalQueryResult(int32_t _srcType = -1, int32_t _dstType = -1, bool _canMarshal = false, bool _needs = false,
	                   TBlockMarshaller _m = NULL)
	: srcType(_srcType), dstType(_dstType), canMarshal(_canMarshal), needsMarshalling(_needs), marshaller(_m)
	{
	}
};

struct PropertyMessageEntry
{
	PropertyDescription mProperty;
	NamespacedName mDatatypeName;
	// datatype of the data in the message.
	int32_t mDatatypeId;
	// where in the message this property starts.
	uint32_t mMessageOffset;
	// size of this entry object
	uint32_t mByteSize;

	// If the chain of properties doesn't have any array properties this indicates the
	uint32_t mDestByteSize;

	PropertyMessageEntry(PropertyDescription propName, NamespacedName dtypeName, int32_t dtype, uint32_t messageOff,
	                     uint32_t byteSize, uint32_t destByteSize)
	: mProperty(propName)
	, mDatatypeName(dtypeName)
	, mDatatypeId(dtype)
	, mMessageOffset(messageOff)
	, mByteSize(byteSize)
	, mDestByteSize(destByteSize)
	{
	}
	PropertyMessageEntry() : mDatatypeId(-1), mMessageOffset(0), mByteSize(0), mDestByteSize(0)
	{
	}
};

// Create a struct that defines a subset of the properties on an object.
struct PropertyMessageDescription
{
	NamespacedName mClassName;
	// No other class has this id, it is DB-unique
	int32_t mClassId;
	NamespacedName mMessageName;
	int32_t mMessageId;
	DataRef<PropertyMessageEntry> mProperties;
	uint32_t mMessageByteSize;
	// Offsets into the property message where const char* items are.
	DataRef<uint32_t> mStringOffsets;
	PropertyMessageDescription(const NamespacedName& nm, int32_t clsId, const NamespacedName& msgName, int32_t msgId,
	                           uint32_t msgSize)
	: mClassName(nm), mClassId(clsId), mMessageName(msgName), mMessageId(msgId), mMessageByteSize(msgSize)
	{
	}
	PropertyMessageDescription() : mClassId(-1), mMessageId(-1), mMessageByteSize(0)
	{
	}
	virtual ~PropertyMessageDescription()
	{
	}
};

class StringTable
{
  protected:
	virtual ~StringTable()
	{
	}

  public:
	virtual uint32_t getNbStrs() = 0;
	virtual uint32_t getStrs(const char** outStrs, uint32_t bufLen, uint32_t startIdx = 0) = 0;
	virtual const char* registerStr(const char* str, bool& outAdded) = 0;
	const char* registerStr(const char* str)
	{
		bool ignored;
		return registerStr(str, ignored);
	}
	virtual StringHandle strToHandle(const char* str) = 0;
	virtual const char* handleToStr(uint32_t hdl) = 0;
	virtual void release() = 0;

	static StringTable& create();
};

struct None
{
};

template <typename T>
class Option
{
	T mValue;
	bool mHasValue;

  public:
	Option(const T& val) : mValue(val), mHasValue(true)
	{
	}
	Option(None nothing = None()) : mHasValue(false)
	{
		(void)nothing;
	}
	Option(const Option& other) : mValue(other.mValue), mHasValue(other.mHasValue)
	{
	}
	Option& operator=(const Option& other)
	{
		mValue = other.mValue;
		mHasValue = other.mHasValue;
		return *this;
	}
	bool hasValue() const
	{
		return mHasValue;
	}
	const T& getValue() const
	{
		PX_ASSERT(hasValue());
		return mValue;
	}
	T& getValue()
	{
		PX_ASSERT(hasValue());
		return mValue;
	}
	operator const T&() const
	{
		return getValue();
	}
	operator T&()
	{
		return getValue();
	}
	T* operator->()
	{
		return &getValue();
	}
	const T* operator->() const
	{
		return &getValue();
	}
};

/**
 *	Create new classes and add properties to some existing ones.
 *	The default classes are created already, the simple types
 *  along with the basic math types.
 *	(uint8_t, int8_t, etc )
 *	(PxVec3, PxQuat, PxTransform, PxMat33, PxMat34, PxMat44)
 */
class PvdObjectModelMetaData
{
  protected:
	virtual ~PvdObjectModelMetaData()
	{
	}

  public:
	virtual ClassDescription getOrCreateClass(const NamespacedName& nm) = 0;
	// get or create parent, lock parent. deriveFrom getOrCreatechild.
	virtual bool deriveClass(const NamespacedName& parent, const NamespacedName& child) = 0;
	virtual Option<ClassDescription> findClass(const NamespacedName& nm) const = 0;
	template <typename TDataType>
	Option<ClassDescription> findClass()
	{
		return findClass(getPvdNamespacedNameForType<TDataType>());
	}
	virtual Option<ClassDescription> getClass(int32_t classId) const = 0;
	virtual ClassDescription* getClassPtr(int32_t classId) const = 0;

	virtual Option<ClassDescription> getParentClass(int32_t classId) const = 0;
	bool isDerivedFrom(int32_t classId, int32_t parentClass) const
	{
		if(classId == parentClass)
			return true;
		ClassDescription* p = getClassPtr(getClassPtr(classId)->mBaseClass);
		while(p != NULL)
		{
			if(p->mClassId == parentClass)
				return true;
			p = getClassPtr(p->mBaseClass);
		}
		return false;
	}

	virtual void lockClass(int32_t classId) = 0;

	virtual uint32_t getNbClasses() const = 0;
	virtual uint32_t getClasses(ClassDescription* outClasses, uint32_t requestCount, uint32_t startIndex = 0) const = 0;

	// Create a nested property.
	// This way you can have obj.p.x without explicity defining the class p.
	virtual Option<PropertyDescription> createProperty(int32_t classId, String name, String semantic, int32_t datatype,
	                                                   PropertyType::Enum propertyType = PropertyType::Scalar) = 0;
	Option<PropertyDescription> createProperty(NamespacedName clsId, String name, String semantic, NamespacedName dtype,
	                                           PropertyType::Enum propertyType = PropertyType::Scalar)
	{
		return createProperty(findClass(clsId)->mClassId, name, semantic, findClass(dtype)->mClassId, propertyType);
	}
	Option<PropertyDescription> createProperty(NamespacedName clsId, String name, NamespacedName dtype,
	                                           PropertyType::Enum propertyType = PropertyType::Scalar)
	{
		return createProperty(findClass(clsId)->mClassId, name, "", findClass(dtype)->mClassId, propertyType);
	}
	Option<PropertyDescription> createProperty(int32_t clsId, String name, int32_t dtype,
	                                           PropertyType::Enum propertyType = PropertyType::Scalar)
	{
		return createProperty(clsId, name, "", dtype, propertyType);
	}
	template <typename TDataType>
	Option<PropertyDescription> createProperty(int32_t clsId, String name, String semantic = "",
	                                           PropertyType::Enum propertyType = PropertyType::Scalar)
	{
		return createProperty(clsId, name, semantic, getPvdNamespacedNameForType<TDataType>(), propertyType);
	}
	virtual Option<PropertyDescription> findProperty(const NamespacedName& cls, String prop) const = 0;
	virtual Option<PropertyDescription> findProperty(int32_t clsId, String prop) const = 0;
	virtual Option<PropertyDescription> getProperty(int32_t propId) const = 0;
	virtual void setNamedPropertyValues(DataRef<NamedValue> values, int32_t propId) = 0;
	// for enumerations and flags.
	virtual DataRef<NamedValue> getNamedPropertyValues(int32_t propId) const = 0;

	virtual uint32_t getNbProperties(int32_t classId) const = 0;
	virtual uint32_t getProperties(int32_t classId, PropertyDescription* outBuffer, uint32_t bufCount,
	                               uint32_t startIdx = 0) const = 0;

	// Check that a property path, starting at the given class id and first property is value.  Return the resolved
	// properties.
	// outbuffer.size *must* equal the propPath.size().
	Option<PropertyDescription> resolvePropertyPath(int32_t clsId, const int32_t propId) const
	{
		Option<PropertyDescription> prop(getProperty(propId));
		if(prop.hasValue() == false)
			return prop;
		if(isDerivedFrom(clsId, prop.getValue().mOwnerClassId) == false)
			return None();
		return prop;
	}
	// Does one cls id differ marshalling to another and if so return the functions to do it.
	virtual MarshalQueryResult checkMarshalling(int32_t srcClsId, int32_t dstClsId) const = 0;

	// messages and classes are stored in separate maps, so a property message can have the same name as a class.
	virtual Option<PropertyMessageDescription> createPropertyMessage(const NamespacedName& cls,
	                                                                 const NamespacedName& msgName,
	                                                                 DataRef<PropertyMessageArg> entries,
	                                                                 uint32_t messageSize) = 0;
	virtual Option<PropertyMessageDescription> findPropertyMessage(const NamespacedName& msgName) const = 0;
	virtual Option<PropertyMessageDescription> getPropertyMessage(int32_t msgId) const = 0;

	virtual uint32_t getNbPropertyMessages() const = 0;
	virtual uint32_t getPropertyMessages(PropertyMessageDescription* msgBuf, uint32_t bufLen,
	                                     uint32_t startIdx = 0) const = 0;

	virtual StringTable& getStringTable() const = 0;

	virtual void write(PvdOutputStream& stream) const = 0;
	void save(PvdOutputStream& stream) const
	{
		write(stream);
	}

	virtual PvdObjectModelMetaData& clone() const = 0;

	virtual void addRef() = 0;
	virtual void release() = 0;

	static uint32_t getCurrentPvdObjectModelVersion();
	static PvdObjectModelMetaData& create();
	static PvdObjectModelMetaData& create(PvdInputStream& stream);
};
}
}
#endif // PXPVDSDK_PXPVDOBJECTMODELMETADATA_H
