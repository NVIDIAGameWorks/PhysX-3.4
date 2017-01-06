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


#ifndef SAMPLE_VEHICLE_VEHICLE_CONTROLLER_H
#define SAMPLE_VEHICLE_VEHICLE_CONTROLLER_H

#include "common/PxPhysXCommonConfig.h"
#include "foundation/PxVec3.h"
#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleUpdate.h"
#include "vehicle/PxVehicleUtilControl.h"

using namespace physx;

class SampleVehicle_VehicleController
{
public:

	SampleVehicle_VehicleController();
	~SampleVehicle_VehicleController();

	void setCarKeyboardInputs
		(const bool accel, const bool brake, const bool handbrake, 
		 const bool steerleft, const bool steerright, 
		 const bool gearup, const bool geardown)
	{
		mKeyPressedAccel=accel;
		mKeyPressedBrake=brake;
		mKeyPressedHandbrake=handbrake;
		mKeyPressedSteerLeft=steerleft;
		mKeyPressedSteerRight=steerright;
		mKeyPressedGearUp=gearup;
		mKeyPressedGearDown=geardown;
	}

	void setCarGamepadInputs
		(const PxF32 accel, const PxF32 brake, 
		 const PxF32 steer, 
		 const bool gearup, const bool geardown, 
		 const bool handbrake)
	{
		mGamepadAccel=accel;
		mGamepadCarBrake=brake;
		mGamepadCarSteer=steer;
		mGamepadGearup=gearup;
		mGamepadGeardown=geardown;
		mGamepadCarHandbrake=handbrake;
	}

	void setTankKeyboardInputs
		(const bool accel, const bool thrustLeft, const bool thrustRight, const bool brakeLeft, const bool brakeRight, const bool gearUp, const bool gearDown)
	{
		mKeyPressedAccel=accel;
		mKeyPressedThrustLeft=thrustLeft;
		mKeyPressedThrustRight=thrustRight;
		mKeyPressedBrakeLeft=brakeLeft;
		mKeyPressedBrakeRight=brakeRight;
		mKeyPressedGearUp=gearUp;
		mKeyPressedGearDown=gearDown;
	}

	void setTankGamepadInputs
		(const PxF32 accel, const PxF32 thrustLeft, const PxF32 thrustRight, const PxF32 brakeLeft, const PxF32 brakeRight, const bool gearUp, const bool gearDown)
	{
		mGamepadAccel=accel;
		mTankThrustLeft=thrustLeft;
		mTankThrustRight=thrustRight;
		mTankBrakeLeft=brakeLeft;
		mTankBrakeRight=brakeRight;
		mGamepadGearup=gearUp;
		mGamepadGeardown=gearDown;
	}

	void toggleAutoGearFlag() 
	{
		mToggleAutoGears = true;
	}

	void update(const PxF32 dtime, const PxVehicleWheelQueryResult& vehicleWheelQueryResults, PxVehicleWheels& focusVehicle);

	void clear();

private:

	//Raw driving inputs - keys (car + tank)
	bool			mKeyPressedAccel;
	bool			mKeyPressedGearUp;
	bool			mKeyPressedGearDown;

	//Raw driving inputs - keys (car only)
	bool			mKeyPressedBrake;
	bool			mKeyPressedHandbrake;
	bool			mKeyPressedSteerLeft;
	bool			mKeyPressedSteerRight;

	//Raw driving inputs - keys (tank only)
	bool			mKeyPressedThrustLeft;
	bool			mKeyPressedThrustRight;
	bool			mKeyPressedBrakeLeft;
	bool			mKeyPressedBrakeRight;

	//Raw driving inputs - gamepad (car + tank)
	PxF32			mGamepadAccel;
	bool			mGamepadGearup;
	bool			mGamepadGeardown;

	//Raw driving inputs - gamepad (car only)
	PxF32			mGamepadCarBrake;
	PxF32			mGamepadCarSteer;
	bool			mGamepadCarHandbrake;

	//Raw driving inputs - (tank only)
	PxF32			mTankThrustLeft;
	PxF32			mTankThrustRight;
	PxF32			mTankBrakeLeft;
	PxF32			mTankBrakeRight;

	//Record and replay using raw driving inputs.
	bool			mRecord;
	bool			mReplay;
	enum
	{
		MAX_NUM_RECORD_REPLAY_SAMPLES=8192
	};
	// Keyboard
	bool			mKeyboardAccelValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mKeyboardBrakeValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mKeyboardHandbrakeValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mKeyboardSteerLeftValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mKeyboardSteerRightValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mKeyboardGearupValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mKeyboardGeardownValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	// Gamepad - (tank + car)
	PxF32			mGamepadAccelValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mGamepadGearupValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mGamepadGeardownValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	// Gamepad - car only
	PxF32			mGamepadCarBrakeValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	PxF32			mGamepadCarSteerValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	bool			mGamepadCarHandbrakeValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	//Gamepad - tank only.
	PxF32			mGamepadTankThrustLeftValues[MAX_NUM_RECORD_REPLAY_SAMPLES];
	PxF32			mGamepadTankThrustRightValues[MAX_NUM_RECORD_REPLAY_SAMPLES];

	PxU32			mNumSamples;
	PxU32			mNumRecordedSamples;

	// Raw data taken from the correct stream (live input stream or replay stream)
	bool			mUseKeyInputs;

	// Toggle autogears flag on focus vehicle
	bool			mToggleAutoGears;

	//Auto-reverse mode.
	bool			mIsMovingForwardSlowly;
	bool			mInReverseMode;

	//Update 
	void processRawInputs(const PxF32 timestep, const bool useAutoGears, PxVehicleDrive4WRawInputData& rawInputData);
	void processRawInputs(const PxF32 timestep, const bool useAutoGears, PxVehicleDriveTankRawInputData& rawInputData);
	void processAutoReverse(
		const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
		const PxVehicleDrive4WRawInputData& rawInputData, 
		bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const;
	void processAutoReverse(
		const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
		const PxVehicleDriveTankRawInputData& rawInputData, 
		bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const;

	////////////////////////////////
	//Record and replay deprecated at the moment.
	//Setting functions as private to avoid them being used.
	///////////////////////////////
	bool getIsInRecordReplayMode() const {return (mRecord || mReplay);}
	bool getIsRecording() const {return mRecord;}
	bool getIsReplaying() const {return mReplay;}

	void enableRecordReplayMode()
	{
		PX_ASSERT(!getIsInRecordReplayMode());
		mRecord=true;
		mReplay=false;
		mNumRecordedSamples=0;
	}

	void disableRecordReplayMode()
	{
		PX_ASSERT(getIsInRecordReplayMode());
		mRecord=false;
		mReplay=false;
		mNumRecordedSamples=0;
	}

	void restart();
	////////////////////////////

};

#endif //SAMPLE_VEHICLE_VEHICLE_CONTROLLER_H