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

#ifndef TEST_CATEGORY
#define TEST_CATEGORY

#include "PhysXSampleApplication.h"
#include "SampleArray.h"

namespace Test
{
	/**
	Group of tests. The groups can be linked to a tree.
	If a test is added to a group, a leaf group is automatically created, that 
	contains the test. 
	*/
	class TestGroup
	{
		static const size_t MAX_COUNT = ~0;
	public:
		explicit TestGroup(const char* name, size_t count = MAX_COUNT);
		explicit TestGroup(TestGroup* group);
		TestGroup(SampleCreator test, const char* name, size_t count = MAX_COUNT);
		~TestGroup();

		/**
		Releases all tests from this node and its descendants. 
		Destructing a Test groups just releases the groups and not the tests themselves!
		*/
//		void releaseTests();

		/**
		Returns the name of the TestGroup. If the test group represents a single 
		test, the name of the test is returned instead.
		*/
		const char* getName() const { return mName; }

		/**
		Prints all the TestGroup names to strBuffer in this format:
		"RootName/Name1/Name2/.../ThisName"
		With omitRoot the root TestGroup is omitted.
		*/
		void getPathName(char* strBuffer, unsigned strBufferMaxLength, bool omitRoot);

		/**
		Adds a child TestGroup to this TestGroup. After this call the group is owned by this. 
		A TestGroup can't be added multiple times.
		*/
		void addGroup(TestGroup* group);

		/**
		Deep copy of all the group data, except the tests themselves. The copy is returned as a root.
		*/
		TestGroup* deepCopy();

		/**
		Adds a copy of path to this group avoiding duplicates. Returns the copy of the last element in path.
		*/
		TestGroup* addPath(SampleArray<TestGroup*>& path);

		/**
		Fills all TestGroup instances into path starting with this->getRoot() and ending with this. 
		*/
		void getPath(SampleArray<TestGroup*>& path);

		/**
		Reads a path from pathName and returns the corresponding TestGroup. 
		The format of the path has to be the same as the one used by getPathName().
		If the TestGroup doesn't exist, NULL is returned.
		*/
		TestGroup* getGroupFromPathName(const char* pathName, bool omitRoot);

		/**
		Returns whether this TestGroup corresponds to a single test.
		*/
		bool isTest() const { return mTest != NULL; }

		/**
		Automatically creates a child TestGroup representing the test passed. A TestInterface instance
		shouldn't be added multiple times, since it is owned by this TestGroup after the call.
		*/
		void addTest(SampleCreator test, const char* name, size_t count = MAX_COUNT);

		/**
		Returns the TestInterface for a TestGroup corresponding to a single test.
		*/
//		TestInterface& getTestInterface() { PX_ASSERT(isTest()); return *mTest; }
		SampleCreator getCreator() { PX_ASSERT(isTest()); return mTest; }

		/**
		Returns the parent TestGroup, if this isn't a root. 
		*/
		TestGroup* getParent() const { return mParent; }

		/**
		Returns the root of this TestGroup.
		*/
		TestGroup* getRoot() { return mParent ? mParent->getRoot() : this; }

		/**
		Returns the first direct child of this TestGroup. If this TestGroup doesn't has
		any children, NULL is returned. 
		*/
		TestGroup* getFirstChild() const { return mChildren.empty() ? NULL : mChildren.front(); }

		/**
		Returns the last direct child of this TestGroup. If this TestGroup doesn't has
		any children, NULL is returned. 
		*/
		TestGroup* getLastChild() const { return mChildren.empty() ? NULL : mChildren.back(); }

		/**
		Returns the next direct child of this TestGroup after current. If there is no next child, NULL is returned.
		The TestGroup current has to be itself a direct child of this Testgroup. 
		*/
		TestGroup* getNextChild(TestGroup& current);

		/**
		Returns the previous direct child of this TestGroup after current. If there is no previous child, NULL is returned.
		The TestGroup current has to be itself a direct child of this Testgroup. 
		*/
		TestGroup* getPreviousChild(TestGroup& current);

		/**
		Returns the child with the given name, if exists.
		*/
		TestGroup* getChildByName(const char* name, size_t count = MAX_COUNT);

		/**
		Returns the first TestGroup representing a single test (isTest() == true) within 
		the whole test group tree rooted at this TestGroup. Returns NULL if there isn't such 
		a TestGroup.
		*/
		TestGroup* getFirstTest();

		/**
		Returns the last TestGroup representing a single test (isTest() == true) within 
		the whole test group tree rooted at this TestGroup. Returns NULL if there isn't such 
		a TestGroup.
		*/
		TestGroup* getLastTest();

		/**
		Returns the next TestGroup representing a single test (isTest() == true) within 
		the whole test group tree rooted at this TestGroup. The current TestGroup has to be
		a descendant of this TestGroup.
		*/
		TestGroup* getNextTest(TestGroup* current);

		/**
		Returns the previous TestGroup representing a single test (isTest() == true) within 
		the whole test group tree rooted at this TestGroup. The current TestGroup has to be
		a descendant of this TestGroup.
		*/
		TestGroup* getPreviousTest(TestGroup* current);

		/**
		Returns the number of children
		*/
		PxU32 numChildren() const { return mChildren.size(); }

	private:
		unsigned getChildIndex(TestGroup& child);
		TestGroup* getFirstLeaf();
		TestGroup* getLastLeaf();
		TestGroup* getNextLeaf(TestGroup* current);
		TestGroup* getPreviousLeaf(TestGroup* current);

	private:
		SampleCreator mTest;
		char* mName;
		SampleArray<TestGroup*> mChildren;
		TestGroup* mParent;
	};
};

#endif // TEST_CATEGORY
