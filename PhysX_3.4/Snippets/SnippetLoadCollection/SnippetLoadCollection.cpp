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
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

// ****************************************************************************
// This snippet illustrates loading xml or binary serialized collections and instantiating the objects in a scene.
//
// It only compiles and runs on authoring platforms (windows, osx and linux). 
// The snippet supports connecting to PVD in order to display the scene.
//
// It is a simple command-line tool supporting the following options::
// SnippetLoadCollection [--pvdhost=<ip address> ] [--pvdport=<ip port> ] [--pvdtimeout=<time ms> ] [--generateExampleFiles] <filename>...		   
//
// --pvdhost=<ip address>              Defines ip address of PVD, default is 127.0.0.1
// --pvdport=<ip port>                 Defines ip port of PVD, default is 5425
// --pvdtimeout=<time ms>              Defines time out of PVD, default is 10
// --generateExampleFiles              Generates a set of example files
//   <filename>...	                   Input files containing serialized collections (either xml or binary)
//
// Multiple collection files can be specified. The snippet is currently restricted to load a list of collections which obey
// the following rule: The first collection needs to be complete. All following collections - if any - may only maintain
// dependencies to objects in the first collection.
//
// The set of example files that can be generated consists of
// collection.xml|collection.bin: Can be used individually.
// collectionA.xml|collectionA.bin: Can also be used individually but only contain materials and shapes without actors.
// collectionB.xml|collectionB.bin: Need to be used together with collectionA files. The actors contained in collectionB
// maintain references to objects in the collectionA.
//
// ****************************************************************************

#include "PxPhysicsAPI.h"

#include <iostream>

#include "../SnippetCommon/SnippetPrint.h"
#include "../SnippetCommon/SnippetPVD.h"
#include "../SnippetUtils/SnippetUtils.h"

#define MAX_INPUT_FILES   16

using namespace physx;

PxDefaultAllocator		    gAllocator;
PxDefaultErrorCallback	    gErrorCallback;

PxFoundation*			    gFoundation = NULL;
PxPhysics*				    gPhysics	= NULL;
PxCooking*				    gCooking	= NULL;
PxSerializationRegistry*    gSerializationRegistry = NULL;

PxDefaultCpuDispatcher*	    gDispatcher = NULL;
PxScene*				    gScene		= NULL;

PxPvd*                      gPvd        = NULL;

PxU8*					    gMemBlocks[MAX_INPUT_FILES];
PxU32                       gNbMemBlocks = 0;

struct CmdLineParameters
{
	const char*		pvdhost;		
	PxU32		    pvdport;			
	PxU32		    pvdtimeout;	
	const char*     inputFiles[MAX_INPUT_FILES];
	PxU32           nbFiles;
	bool			generateExampleFiles;

	CmdLineParameters()	: 
	      pvdhost(PVD_HOST)
		, pvdport(5425)
		, pvdtimeout(10)
		, nbFiles(0)
		, generateExampleFiles(false)
	{}
} gParameters;

static bool match(const char* opt, const char* ref)
{
	std::string s1(opt);
	std::string s2(ref);
	return !s1.compare(0, s2.length(), s2);
}

static void printHelpMsg()
{
	printf("SnippetLoadCollection usage:\n"
		"SnippetLoadCollection "
		"[--pvdhost=<ip address> ] "
		"[--pvdport=<ip port> ]"
		"[--pvdtimeout=<time ms> ] "
		"[--generateExampleFiles]"		
		"<filename>...\n\n"		   
		"Load binary or xml serialized collections and instatiate the objects in a PhysX scene.\n");

	printf("--pvdhost=<ip address> \n");
	printf("  Defines ip address of PVD, default is 127.0.0.1 \n");

	printf("--pvdport=<ip port> \n");
	printf("  Defines ip port of PVD, default is 5425\n");

	printf("--pvdtimeout=<time ms> \n");
	printf("  Defines timeout of PVD, default is 10\n");

	printf("--generateExampleFiles\n");
	printf("  Generates a set of example files\n");	

	printf("<filename>...\n");
	printf("  Input files (xml or binary), if a collection contains shared objects, it needs to be provided with the first file. \n\n");

}

static bool parseCommandLine(CmdLineParameters& result, int argc, const char *const*argv)
{
	if( argc <= 1 )
	{
		printHelpMsg();	
		return false;
	}

	for(int i = 1; i < argc; ++i)
	{
		if(argv[i][0] != '-' || argv[i][1] != '-')
		{
			if (result.nbFiles < MAX_INPUT_FILES)
			{
				result.inputFiles[result.nbFiles++] = argv[i];
			}
			else
				printf( "[WARNING] more input files are specified than supported (maximum %d). Ignoring the file  %s\n", MAX_INPUT_FILES, argv[i] );				
		}
		else if(match(argv[i], "--pvdhost="))
		{
			const char* hostStr = argv[i] + strlen("--pvdhost=");
			if(hostStr)
				result.pvdhost = hostStr;
		}
		else if(match(argv[i], "--pvdport="))
		{			
			const char* portStr = argv[i] + strlen("--pvdport=");		
			if (portStr)
				result.pvdport = PxU32(atoi(portStr));	
		}
		else if(match(argv[i], "--pvdtimeout="))
		{			
			const char* timeoutStr = argv[i] + strlen("--pvdtimeout=");		
			if (timeoutStr)
				result.pvdtimeout = PxU32(atoi(timeoutStr));	
		}
		else if(match(argv[i], "--generateExampleFiles"))
		{
			result.generateExampleFiles = true;
		}
		else
		{
			printf( "[ERROR] Unknown command line parameter \"%s\"\n", argv[i] );
			printHelpMsg();
			return false;
		}
	}
	
	if(result.nbFiles == 0 &&  !result.generateExampleFiles)
	{
		printf( "[ERROR] parameter missing.\n" );
		printHelpMsg();
		return false;				   
	}

	return true;
}

static bool checkFile(bool& isBinary, const char* filename)
{
	PxDefaultFileInputData fileStream(filename);
	if (fileStream.getLength() == 0)
	{
		printf( "[ERROR] input file %s can't be opened!\n", filename);
		return false;
	}

	char testString[512];
	fileStream.read(testString, 18);
	testString[18] = 0;

	if (strcmp("SEBD", testString) == 0)
	{
		isBinary = true;
		return true;
	}

	if (strcmp("<PhysX30Collection", testString) == 0)
	{
		isBinary = false;
		return true;
	}

	printf( "[ERROR] input file %s seems neither an xml nor a binary serialized collection file!\n", filename);
	return false;
}

static PxCollection* deserializeCollection(PxInputData& inputData, bool isBinary, PxCollection* sharedCollection, PxSerializationRegistry& sr)
{
	PxCollection* collection = NULL;
	if(isBinary)
	{
		PxU32 length = inputData.getLength();
		PxU8* memBlock = static_cast<PxU8*>(malloc(length+PX_SERIAL_FILE_ALIGN-1));
		gMemBlocks[gNbMemBlocks++] = memBlock;
		void* alignedBlock = reinterpret_cast<void*>((size_t(memBlock)+PX_SERIAL_FILE_ALIGN-1)&~(PX_SERIAL_FILE_ALIGN-1));
		inputData.read(alignedBlock, length);
		collection = PxSerialization::createCollectionFromBinary(alignedBlock, sr, sharedCollection);
	}
	else
	{
		collection = PxSerialization::createCollectionFromXml(inputData, *gCooking, sr, sharedCollection);		
	}

	return collection;
}

void initPhysics()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport,PxPvdInstrumentationFlag::eALL);
	
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);
	
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0, -9.81f, 0);	
	gDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));	
	
	gSerializationRegistry = PxSerialization::createSerializationRegistry(*gPhysics);

}

void cleanupPhysics()
{	
	gSerializationRegistry->release();
	gScene->release();
	gDispatcher->release();
	PxCloseExtensions();
		
	gPhysics->release();	// releases of all objects	
	gCooking->release();

	for(PxU32 i=0; i<gNbMemBlocks; i++)
		free(gMemBlocks[i]); // now that the objects have been released, it's safe to release the space they occupy

	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();

	gFoundation->release();

	printf("SnippetLoadCollection done.\n");
}

static void serializeCollection(PxCollection& collection, PxCollection* externalRefs, const char* filename, bool toBinary)
{
	PxDefaultFileOutputStream outputStream(filename);
	if (!outputStream.isValid())
	{
		printf( "[ERROR] Could not open file %s!\n", filename);
		return;
	}

	bool bret;
	if (toBinary)
	{
		bret = PxSerialization::serializeCollectionToBinary(outputStream, collection, *gSerializationRegistry, externalRefs);
	}
	else
	{
		bret = PxSerialization::serializeCollectionToXml(outputStream, collection, *gSerializationRegistry, NULL, externalRefs);
	}

	if(bret)
		printf( "Generated: \"%s\"\n", filename);
	else
		printf( "[ERROR] Failure when generating %s!\n", filename);
}

static void generateExampleFiles()
{
	PxCollection* collection = PxCreateCollection();
	PxCollection* collectionA = PxCreateCollection();
	PxCollection* collectionB = PxCreateCollection();
	PX_ASSERT( (collection != NULL) && (collectionA != NULL) && (collectionB != NULL) );
	
	PxMaterial *material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
	PX_ASSERT( material );
	PxShape* planeShape = gPhysics->createShape(PxPlaneGeometry(), *material);
	PxShape* boxShape = gPhysics->createShape(PxBoxGeometry(2.f, 2.f, 2.f), *material);
	PxRigidStatic* rigidStatic = PxCreateStatic(*gPhysics, PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxHalfPi, PxVec3(0.f, 0.f, 1.f))), *planeShape);
	PxRigidDynamic* rigidDynamic = PxCreateDynamic(*gPhysics, PxTransform(PxVec3(0.f, 2.f, 0.f)), *boxShape, 1.f); 
	
	collection->add(*material);
	collection->add(*planeShape);
	collection->add(*boxShape);
	collection->add(*rigidStatic);
	collection->add(*rigidDynamic);
	PxSerialization::complete(*collection, *gSerializationRegistry);
	PX_ASSERT(PxSerialization::isSerializable(*collection, *gSerializationRegistry));

	collectionA->add(*material);
	collectionA->add(*planeShape);
	collectionA->add(*boxShape);
	PxSerialization::complete(*collectionA, *gSerializationRegistry);
	PxSerialization::createSerialObjectIds(*collectionA, PxSerialObjectId(1));
	PX_ASSERT(PxSerialization::isSerializable(*collectionA, *gSerializationRegistry));

	collectionB->add(*rigidStatic);
	collectionB->add(*rigidDynamic);
	PxSerialization::complete(*collectionB, *gSerializationRegistry, collectionA);
	PX_ASSERT(PxSerialization::isSerializable(*collectionB, *gSerializationRegistry, collectionA));
	
	serializeCollection(*collection, NULL, "collection.xml", false);
	serializeCollection(*collectionA, NULL, "collectionA.xml", false);
	serializeCollection(*collectionB, collectionA, "collectionB.xml", false);
	serializeCollection(*collection, NULL, "collection.bin", true);
	serializeCollection(*collectionA, NULL, "collectionA.bin", true);
	serializeCollection(*collectionB, collectionA, "collectionB.bin", true);

	collection->release();
	collectionA->release();
	collectionB->release();
}

int snippetMain(int argc, const char *const* argv)
{
	if(!parseCommandLine(gParameters, argc, argv))
	   return 1;

	initPhysics();

	if(gParameters.generateExampleFiles)
		generateExampleFiles();
	
	// collection that may have shared objects
	PxCollection* firstCollection = NULL;

	for(PxU32 i=0; i<gParameters.nbFiles; i++)
	{
		const char*	filename = gParameters.inputFiles[i];

		bool isBinary;
		bool validFile = checkFile(isBinary, filename);

		if (!validFile)
			break;

		PxDefaultFileInputData inputStream(filename);
		PxCollection* collection = deserializeCollection(inputStream, isBinary, firstCollection, *gSerializationRegistry);
		if (!collection)
		{
			printf( "[ERROR] deserialization failure! filename: %s\n", filename);
			break;
		}
		else
		{
			printf( "Loaded: \"%s\"\n", filename);
		}

		gScene->addCollection(*collection);

		if (i == 0)
		{
			firstCollection = collection;
		}
		else
		{
			collection->release();
		}
	}

	if (firstCollection)
		firstCollection->release();

	gScene->simulate(1.0f/60.0f);
	gScene->fetchResults(true);

	cleanupPhysics();	

	return 0;
}

