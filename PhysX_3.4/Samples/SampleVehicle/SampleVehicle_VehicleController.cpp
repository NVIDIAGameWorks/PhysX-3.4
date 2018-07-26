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

#include "SampleVehicle_VehicleController.h"
#include "PhysXSample.h"
#include "vehicle/PxVehicleDrive4W.h"
#include "vehicle/PxVehicleDriveNW.h"
#include "vehicle/PxVehicleUtilControl.h"
#include "vehicle/PxVehicleUtil.h"

using namespace SampleRenderer;
using namespace SampleFramework;

PxVehicleKeySmoothingData gKeySmoothingData=
{
	{
		3.0f,	//rise rate eANALOG_INPUT_ACCEL		
		3.0f,	//rise rate eANALOG_INPUT_BRAKE		
		10.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT	
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT	
	},
	{
		5.0f,	//fall rate eANALOG_INPUT__ACCEL		
		5.0f,	//fall rate eANALOG_INPUT__BRAKE		
		10.0f,	//fall rate eANALOG_INPUT__HANDBRAKE	
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT	
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT	
	}
};

PxVehiclePadSmoothingData gCarPadSmoothingData=
{
	{
		6.0f,	//rise rate eANALOG_INPUT_ACCEL		
		6.0f,	//rise rate eANALOG_INPUT_BRAKE		
		12.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
		2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT	
		2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT	
	},
	{
		10.0f,	//fall rate eANALOG_INPUT_ACCEL		
		10.0f,	//fall rate eANALOG_INPUT_BRAKE		
		12.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
		5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT	
		5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT	
	}
};

PxF32 gSteerVsForwardSpeedData[2*8]=
{
	0.0f,		0.75f,
	5.0f,		0.75f,
	30.0f,		0.125f,
	120.0f,		0.1f,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32,
	PX_MAX_F32, PX_MAX_F32
};
PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData,4);

//Tank smoothing data.
PxVehiclePadSmoothingData gTankPadSmoothingData=
{
	{
		6.0f,		//rise rate eTANK_ANALOG_INPUT_ACCEL		
		6.0f,		//rise rate eTANK_ANALOG_INPUT_BRAKE_LEFT		
		6.0f,		//rise rate eTANK_ANALOG_INPUT_BRAKE_RIGHT	
		2.5f,		//rise rate eTANK_ANALOG_INPUT_THRUST_LEFT	
		2.5f,		//rise rate eTANK_ANALOG_INPUT_THRUST_RIGHT	
	},
	{
		10.0f,		//fall rate eTANK_ANALOG_INPUT_ACCEL		
		10.0f,		//fall rate eTANK_ANALOG_INPUT_BRAKE_LEFT		
		10.0f,		//fall rate eTANK_ANALOG_INPUT_BRAKE_RIGHT	
		5.0f,		//fall rate eTANK_ANALOG_INPUT_THRUST_LEFT	
		5.0f		//fall rate eTANK_ANALOG_INPUT_THRUST_RIGHT		
	}
};

			
///////////////////////////////////////////////////////////////////////////////

SampleVehicle_VehicleController::SampleVehicle_VehicleController()
{
	clear();
}

SampleVehicle_VehicleController::~SampleVehicle_VehicleController()
{
}

void SampleVehicle_VehicleController::clear()
{
	mKeyPressedAccel					 = false;
	mKeyPressedGearUp					 = false;
	mKeyPressedGearDown					 = false;

	mKeyPressedBrake					 = false;
	mKeyPressedHandbrake				 = false;
	mKeyPressedSteerLeft				 = false;
	mKeyPressedSteerRight				 = false;

	mKeyPressedThrustLeft				 = false;
	mKeyPressedThrustRight				 = false;
	mKeyPressedBrakeLeft				 = false;
	mKeyPressedBrakeRight				 = false;

	mGamepadAccel						 = 0.0f;
	mGamepadGearup						 = false;			
	mGamepadGeardown					 = false;

	mGamepadCarBrake					 = 0.0f;
	mGamepadCarSteer					 = 0.0f;
	mGamepadCarHandbrake				 = false;

	mTankThrustLeft						 = 0.0f;
	mTankThrustRight					 = 0.0f;
	mTankBrakeLeft						 = 0.0f;
	mTankBrakeRight						 = 0.0f;

	mRecord								 = false;
	mReplay								 = false;
	mNumSamples							 = 0;
	mNumRecordedSamples					 = 0;
	mUseKeyInputs						 = true;
	mToggleAutoGears					 = false;
	mIsMovingForwardSlowly				 = true;
	mInReverseMode						 = false;
}


/*
bool SampleVehicle_VehicleController::processAutoReverse
(const PxF32 timestep, const bool isInAir, const PxF32 forwardSpeed, const PxF32 sidewaysSpeed, const PxVehicleDriveTankRawInputData& rawInputData)
{
	//Keyboard controls for tank not implemented yet.
	bool brakeLeft,brakeRight,accelLeft,accelRight;
	if(mUseKeyInputs)
	{
		//Keyboard controls for tank not implemented yet.
		brakeLeft=false;
		brakeRight=false;
		accelLeft=false;
		accelRight=false;
	}
	else if(PxVehicleDriveTank::eDRIVE_MODEL_STANDARD==rawInputData.getDriveModel())
	{
		brakeLeft = mInReverseMode ? rawInputData.getAnalogLeftThrust()  > 0 : rawInputData.getAnalogLeftBrake() > 0.0f;
		brakeRight = mInReverseMode ? rawInputData.getAnalogRightThrust()  > 0 : rawInputData.getAnalogRightBrake() > 0.0f;
		accelLeft = mInReverseMode ? rawInputData.getAnalogLeftBrake()  > 0 : rawInputData.getAnalogLeftThrust() > 0.0f;
		accelRight = mInReverseMode ? rawInputData.getAnalogRightBrake()  > 0 : rawInputData.getAnalogRightThrust() > 0.0f;
	}
	else
	{
		//Not much point in auto-reverse for tanks that can just spin both wheels backwards.
		return false;
	}

	//If the car has been brought to rest by pressing the brake then raise a flag.
	bool justRaisedFlag=false;
	if(brakeLeft && brakeRight && !mAtRestUnderBraking)
	{
		if(!isInAir && forwardSpeed < THRESHOLD_FORWARD_SPEED && sidewaysSpeed < THRESHOLD_SIDEWAYS_SPEED)
		{
			justRaisedFlag=true;
			mAtRestUnderBraking = true;
			mTimeElapsedSinceAtRestUnderBraking = 0.0f;
		}
	}

	//If the flag is raised and the player pressed accelerate then lower the flag.
	if(mAtRestUnderBraking && (accelLeft || accelRight))
	{	
		mAtRestUnderBraking = false;
		mTimeElapsedSinceAtRestUnderBraking = 0.0f;
	}

	//If the flag is raised and the player doesn't press brake then increment the timer.
	if(!brakeLeft && !brakeRight && mAtRestUnderBraking && !justRaisedFlag)
	{
		mTimeElapsedSinceAtRestUnderBraking += timestep;
	}

	//If the flag is raised and the player pressed brake again then switch auto-reverse.
	if(brakeLeft && brakeRight && mAtRestUnderBraking && !justRaisedFlag && mTimeElapsedSinceAtRestUnderBraking > 0.0f)
	{
		mAtRestUnderBraking = false;
		mTimeElapsedSinceAtRestUnderBraking = 0.0f;
		return true;
	}

	return false;
}
*/

void SampleVehicle_VehicleController::processRawInputs
(const PxF32 dtime, const bool useAutoGears, PxVehicleDrive4WRawInputData& rawInputData)
{
	// Keyboard
	{
		if(mRecord)
		{
			if(mNumSamples<MAX_NUM_RECORD_REPLAY_SAMPLES)
			{
				mKeyboardAccelValues[mNumSamples]		= mKeyPressedAccel;
				mKeyboardBrakeValues[mNumSamples]		= mKeyPressedBrake;
				mKeyboardHandbrakeValues[mNumSamples]	= mKeyPressedHandbrake;
				mKeyboardSteerLeftValues[mNumSamples]	= mKeyPressedSteerLeft;
				mKeyboardSteerRightValues[mNumSamples]	= mKeyPressedSteerRight;
				mKeyboardGearupValues[mNumSamples]		= mKeyPressedGearUp;
				mKeyboardGeardownValues[mNumSamples]	= mKeyPressedGearDown;
			}
		}
		else if(mReplay)
		{
			if(mNumSamples<mNumRecordedSamples)
			{
				mKeyPressedAccel		= mKeyboardAccelValues[mNumSamples];		
				mKeyPressedBrake		= mKeyboardBrakeValues[mNumSamples];		
				mKeyPressedHandbrake	= mKeyboardHandbrakeValues[mNumSamples];	
				mKeyPressedSteerLeft	= mKeyboardSteerLeftValues[mNumSamples];
				mKeyPressedSteerRight	= mKeyboardSteerRightValues[mNumSamples];	
				mKeyPressedGearUp		= mKeyboardGearupValues[mNumSamples];	
				mKeyPressedGearDown		= mKeyboardGeardownValues[mNumSamples];
			}
		}

		rawInputData.setDigitalAccel(mKeyPressedAccel);
		rawInputData.setDigitalBrake(mKeyPressedBrake);
		rawInputData.setDigitalHandbrake(mKeyPressedHandbrake);
		rawInputData.setDigitalSteerLeft(mKeyPressedSteerLeft);
		rawInputData.setDigitalSteerRight(mKeyPressedSteerRight);
		rawInputData.setGearUp(mKeyPressedGearUp);
		rawInputData.setGearDown(mKeyPressedGearDown);

		mUseKeyInputs=
			(mKeyPressedAccel || mKeyPressedBrake  || mKeyPressedHandbrake || 
			 mKeyPressedSteerLeft || mKeyPressedSteerRight || 
			 mKeyPressedGearUp || mKeyPressedGearDown);
	}

	// Gamepad
	{
		if(mRecord)
		{
			if(mNumSamples<MAX_NUM_RECORD_REPLAY_SAMPLES)
			{
				mGamepadAccelValues[mNumSamples]		= mGamepadAccel;
				mGamepadCarBrakeValues[mNumSamples]		= mGamepadCarBrake;
				mGamepadCarSteerValues[mNumSamples]		= mGamepadCarSteer;
				mGamepadGearupValues[mNumSamples]		= mGamepadGearup;
				mGamepadGeardownValues[mNumSamples]		= mGamepadGeardown;
				mGamepadCarHandbrakeValues[mNumSamples]	= mGamepadCarHandbrake;
			}
		}
		else if(mReplay)
		{
			if(mNumSamples<mNumRecordedSamples)
			{
				mGamepadAccel		= mGamepadAccelValues[mNumSamples];		
				mGamepadCarBrake		= mGamepadCarBrakeValues[mNumSamples];		
				mGamepadCarSteer		= mGamepadCarSteerValues[mNumSamples];		
				mGamepadGearup		= mGamepadGearupValues[mNumSamples];	
				mGamepadGeardown	= mGamepadGeardownValues[mNumSamples];	
				mGamepadCarHandbrake	= mGamepadCarHandbrakeValues[mNumSamples];	
			}
		}

		if(mGamepadAccel<0.0f || mGamepadAccel>1.01f)
			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal accel value from gamepad", __FILE__, __LINE__);

		if(mGamepadCarBrake<0.0f || mGamepadCarBrake>1.01f)
			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal brake value from gamepad", __FILE__, __LINE__);

		if(PxAbs(mGamepadCarSteer)>1.01f)
			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal steer value from gamepad", __FILE__, __LINE__);

		if(mUseKeyInputs && ((mGamepadAccel+mGamepadCarBrake+mGamepadCarSteer)!=0.0f ||  mGamepadGearup || mGamepadGeardown || mGamepadCarHandbrake))
		{
			mUseKeyInputs=false;
		}

		if(!mUseKeyInputs)
		{
			rawInputData.setAnalogAccel(mGamepadAccel);
			rawInputData.setAnalogBrake(mGamepadCarBrake);
			rawInputData.setAnalogHandbrake(mGamepadCarHandbrake ? 1.0f : 0.0f);
			rawInputData.setAnalogSteer(mGamepadCarSteer);
			rawInputData.setGearUp(mGamepadGearup);
			rawInputData.setGearDown(mGamepadGeardown);
		}
	}

	if(useAutoGears && (rawInputData.getGearDown() || rawInputData.getGearUp()))
	{
		rawInputData.setGearDown(false);
		rawInputData.setGearUp(false);
	}

	mNumSamples++;
}

void SampleVehicle_VehicleController::processRawInputs
(const PxF32 dtime, const bool useAutoGears, PxVehicleDriveTankRawInputData& rawInputData)
{
	// Keyboard
	//Keyboard controls for tank not implemented yet.
	{
		/*
		if(mRecord)
		{
			if(mNumSamples<MAX_NUM_RECORD_REPLAY_SAMPLES)
			{
				mKeyboardAccelValues[mNumSamples]		= mAccelKeyPressed;
				mKeyboardBrakeValues[mNumSamples]		= mBrakeKeyPressed;
				mKeyboardHandbrakeValues[mNumSamples]	= mHandbrakeKeyPressed;
				mKeyboardSteerLeftValues[mNumSamples]	= mSteerLeftKeyPressed;
				mKeyboardSteerRightValues[mNumSamples]	= mSteerRightKeyPressed;
				mKeyboardGearupValues[mNumSamples]		= mGearUpKeyPressed;
				mKeyboardGeardownValues[mNumSamples]	= mGearDownKeyPressed;
			}
		}
		else if(mReplay)
		{
			if(mNumSamples<mNumRecordedSamples)
			{
				mAccelKeyPressed		= mKeyboardAccelValues[mNumSamples];		
				mBrakeKeyPressed		= mKeyboardBrakeValues[mNumSamples];		
				mHandbrakeKeyPressed	= mKeyboardHandbrakeValues[mNumSamples];	
				mSteerLeftKeyPressed	= mKeyboardSteerLeftValues[mNumSamples];
				mSteerRightKeyPressed	= mKeyboardSteerRightValues[mNumSamples];	
				mGearUpKeyPressed		= mKeyboardGearupValues[mNumSamples];	
				mGearDownKeyPressed		= mKeyboardGeardownValues[mNumSamples];
			}
		}
		*/

		rawInputData.setDigitalAccel(mKeyPressedAccel);
		rawInputData.setDigitalLeftThrust(mKeyPressedThrustLeft);
		rawInputData.setDigitalRightThrust(mKeyPressedThrustRight);
		rawInputData.setDigitalLeftBrake(mKeyPressedBrakeLeft);
		rawInputData.setDigitalRightBrake(mKeyPressedBrakeRight);
		rawInputData.setGearUp(mKeyPressedGearUp);
		rawInputData.setGearDown(mKeyPressedGearDown);

		mUseKeyInputs=
			(mKeyPressedAccel || mKeyPressedThrustLeft  || mKeyPressedThrustRight || 
			mKeyPressedBrakeLeft || mKeyPressedBrakeRight || 
			mKeyPressedGearUp || mKeyPressedGearDown);
	}


	// Gamepad
	{
		if(mRecord)
		{
			if(mNumSamples<MAX_NUM_RECORD_REPLAY_SAMPLES)
			{
				mGamepadAccelValues[mNumSamples]			= mGamepadAccel;
				mGamepadTankThrustLeftValues[mNumSamples]	= mTankThrustLeft;
				mGamepadTankThrustRightValues[mNumSamples]	= mTankThrustRight;
				mGamepadGearupValues[mNumSamples]			= mGamepadGearup;
				mGamepadGeardownValues[mNumSamples]			= mGamepadGeardown;
			}
		}
		else if(mReplay)
		{
			if(mNumSamples<mNumRecordedSamples)
			{
				mGamepadAccel		= mGamepadAccelValues[mNumSamples];		
				mTankThrustLeft	= mGamepadTankThrustLeftValues[mNumSamples];		
				mTankThrustRight= mGamepadTankThrustRightValues[mNumSamples];		
				mGamepadGearup		= mGamepadGearupValues[mNumSamples];	
				mGamepadGeardown	= mGamepadGeardownValues[mNumSamples];	
			}
		}

		if(mGamepadAccel<0.0f || mGamepadAccel>1.01f)
			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal accel value from gamepad", __FILE__, __LINE__);

		if(mTankThrustLeft<-1.01f || mTankThrustLeft>1.01f)
			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal brake value from gamepad", __FILE__, __LINE__);

		if(mTankThrustRight<-1.01f || mTankThrustRight>1.01f)
			getSampleErrorCallback().reportError(PxErrorCode::eINTERNAL_ERROR, "Illegal steer value from gamepad", __FILE__, __LINE__);

		if(mUseKeyInputs && ((mGamepadAccel+mTankThrustLeft+mTankThrustRight)!=0.0f ||  mGamepadGearup || mGamepadGeardown))
		{
			mUseKeyInputs=false;
		}

		if(!mUseKeyInputs)
		{
			rawInputData.setAnalogAccel(mGamepadAccel);
			rawInputData.setAnalogLeftThrust(mTankThrustLeft);
			rawInputData.setAnalogRightThrust(mTankThrustRight);
			rawInputData.setAnalogLeftBrake(mTankBrakeLeft);
			rawInputData.setAnalogRightBrake(mTankBrakeRight);
			rawInputData.setGearUp(mGamepadGearup);
			rawInputData.setGearDown(mGamepadGeardown);
		}
	}

	if(useAutoGears && (rawInputData.getGearDown() || rawInputData.getGearUp()))
	{
		rawInputData.setGearDown(false);
		rawInputData.setGearUp(false);
	}

	mNumSamples++;
}

#define THRESHOLD_FORWARD_SPEED (0.1f) 
#define THRESHOLD_SIDEWAYS_SPEED (0.2f)
#define THRESHOLD_ROLLING_BACKWARDS_SPEED (0.1f)

void SampleVehicle_VehicleController::processAutoReverse
(const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
 const PxVehicleDrive4WRawInputData& carRawInputs,
 bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const
{
	newIsMovingForwardSlowly = false;
	toggleAutoReverse = false;

	if(driveDynData.getUseAutoGears())
	{
		//If the car is travelling very slowly in forward gear without player input and the player subsequently presses the brake then we want the car to go into reverse gear
		//If the car is travelling very slowly in reverse gear without player input and the player subsequently presses the accel then we want the car to go into forward gear
		//If the car is in forward gear and is travelling backwards then we want to automatically put the car into reverse gear.
		//If the car is in reverse gear and is travelling forwards then we want to automatically put the car into forward gear.
		//(If the player brings the car to rest with the brake the player needs to release the brake then reapply it 
		//to indicate they want to toggle between forward and reverse.)

		const bool prevIsMovingForwardSlowly=mIsMovingForwardSlowly;
		bool isMovingForwardSlowly=false;
		bool isMovingBackwards=false;
		const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
		if(!isInAir)
		{
			bool accelRaw,brakeRaw,handbrakeRaw;
			if(mUseKeyInputs)
			{
				accelRaw=carRawInputs.getDigitalAccel();
				brakeRaw=carRawInputs.getDigitalBrake();
				handbrakeRaw=carRawInputs.getDigitalHandbrake();
			}
			else
			{
				accelRaw=carRawInputs.getAnalogAccel() > 0 ? true : false;
				brakeRaw=carRawInputs.getAnalogBrake() > 0 ? true : false;
				handbrakeRaw=carRawInputs.getAnalogHandbrake() > 0 ? true : false;
			}

			const PxF32 forwardSpeed = focusVehicle.computeForwardSpeed();
			const PxF32 forwardSpeedAbs = PxAbs(forwardSpeed);
			const PxF32 sidewaysSpeedAbs = PxAbs(focusVehicle.computeSidewaysSpeed());
			const PxU32 currentGear = driveDynData.getCurrentGear();
			const PxU32 targetGear = driveDynData.getTargetGear();

			//Check if the car is rolling against the gear (backwards in forward gear or forwards in reverse gear).
			if(PxVehicleGearsData::eFIRST == currentGear  && forwardSpeed < -THRESHOLD_ROLLING_BACKWARDS_SPEED)
			{
				isMovingBackwards = true;
			}
			else if(PxVehicleGearsData::eREVERSE == currentGear && forwardSpeed > THRESHOLD_ROLLING_BACKWARDS_SPEED)
			{
				isMovingBackwards = true;
			}

			//Check if the car is moving slowly.
			if(forwardSpeedAbs < THRESHOLD_FORWARD_SPEED && sidewaysSpeedAbs < THRESHOLD_SIDEWAYS_SPEED)
			{
				isMovingForwardSlowly=true;
			}

			//Now work if we need to toggle from forwards gear to reverse gear or vice versa.
			if(isMovingBackwards)
			{
				if(!accelRaw && !brakeRaw && !handbrakeRaw && (currentGear == targetGear))			
				{
					//The car is rolling against the gear and the player is doing nothing to stop this.
					toggleAutoReverse = true;
				}
			}
			else if(prevIsMovingForwardSlowly && isMovingForwardSlowly)
			{
				if((currentGear > PxVehicleGearsData::eNEUTRAL) && brakeRaw && !accelRaw && (currentGear == targetGear))
				{
					//The car was moving slowly in forward gear without player input and is now moving slowly with player input that indicates the 
					//player wants to switch to reverse gear.
					toggleAutoReverse = true;
				}
				else if(currentGear == PxVehicleGearsData::eREVERSE && accelRaw && !brakeRaw && (currentGear == targetGear))
				{
					//The car was moving slowly in reverse gear without player input and is now moving slowly with player input that indicates the 
					//player wants to switch to forward gear.
					toggleAutoReverse = true;
				}
			}

			//If the car was brought to rest through braking then the player needs to release the brake then reapply
			//to indicate that the gears should toggle between reverse and forward.
			if(isMovingForwardSlowly && !brakeRaw && !accelRaw && !handbrakeRaw)
			{
				newIsMovingForwardSlowly = true;
			}
		}
	}
}

void SampleVehicle_VehicleController::processAutoReverse
(const PxVehicleWheels& focusVehicle, const PxVehicleDriveDynData& driveDynData, const PxVehicleWheelQueryResult& vehicleWheelQueryResults,
 const PxVehicleDriveTankRawInputData& tankRawInputs,
 bool& toggleAutoReverse, bool& newIsMovingForwardSlowly) const
{
	newIsMovingForwardSlowly = false;
	toggleAutoReverse = false;

	if(driveDynData.getUseAutoGears())
	{
		//If the car is travelling very slowly in forward gear without player input and the player subsequently presses the brake then we want the car to go into reverse gear
		//If the car is travelling very slowly in reverse gear without player input and the player subsequently presses the accel then we want the car to go into forward gear
		//If the car is in forward gear and is travelling backwards then we want to automatically put the car into reverse gear.
		//If the car is in reverse gear and is travelling forwards then we want to automatically put the car into forward gear.
		//(If the player brings the car to rest with the brake the player needs to release the brake then reapply it 
		//to indicate they want to toggle between forward and reverse.)

		const bool prevIsMovingForwardSlowly=mIsMovingForwardSlowly;
		bool isMovingForwardSlowly=false;
		bool isMovingBackwards=false;
		const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
		if(!isInAir)
		{
			bool accelLeft,accelRight,brakeLeft,brakeRight;
			if(mUseKeyInputs)
			{
				accelLeft=tankRawInputs.getDigitalLeftThrust();
				accelRight=tankRawInputs.getDigitalRightThrust();
				brakeLeft=tankRawInputs.getDigitalLeftBrake();
				brakeRight=tankRawInputs.getDigitalRightBrake();
			}
			else
			{
				accelLeft = tankRawInputs.getAnalogLeftThrust() > 0 ? true : false;
				accelRight = tankRawInputs.getAnalogRightThrust() > 0 ? true : false; 
				brakeLeft = tankRawInputs.getAnalogLeftBrake() > 0 ? true : false;  
				brakeRight = tankRawInputs.getAnalogRightBrake() > 0 ? true : false;

				/*
				if(accelLeft && accelLeft==accelRight && !brakeLeft && !brakeRight)
				{
					shdfnd::printFormatted("aligned accel\n");
				}

				if(brakeLeft && brakeLeft==brakeRight && !accelLeft && !accelRight)
				{
					shdfnd::printFormatted("aligned brake\n");
				}
				*/

			}

			const PxF32 forwardSpeed = focusVehicle.computeForwardSpeed();
			const PxF32 forwardSpeedAbs = PxAbs(forwardSpeed);
			const PxF32 sidewaysSpeedAbs = PxAbs(focusVehicle.computeSidewaysSpeed());
			const PxU32 currentGear = driveDynData.getCurrentGear();
			const PxU32 targetGear = driveDynData.getTargetGear();

			//Check if the car is rolling against the gear (backwards in forward gear or forwards in reverse gear).
			if(PxVehicleGearsData::eFIRST == currentGear  && forwardSpeed < -THRESHOLD_ROLLING_BACKWARDS_SPEED)
			{
				isMovingBackwards = true;
			}
			else if(PxVehicleGearsData::eREVERSE == currentGear && forwardSpeed > THRESHOLD_ROLLING_BACKWARDS_SPEED)
			{
				isMovingBackwards = true;
			}

			//Check if the car is moving slowly.
			if(forwardSpeedAbs < THRESHOLD_FORWARD_SPEED && sidewaysSpeedAbs < THRESHOLD_SIDEWAYS_SPEED)
			{
				isMovingForwardSlowly=true;
			}

			//Now work if we need to toggle from forwards gear to reverse gear or vice versa.
			if(isMovingBackwards)
			{
				if(!accelLeft && !accelRight && !brakeLeft && !brakeRight && (currentGear == targetGear))			
				{
					//The car is rolling against the gear and the player is doing nothing to stop this.
					toggleAutoReverse = true;
				}
			}
			else if(prevIsMovingForwardSlowly && isMovingForwardSlowly)
			{
				if((currentGear > PxVehicleGearsData::eNEUTRAL) && brakeLeft && brakeRight && !accelLeft && !accelRight && (currentGear == targetGear))
				{
					//The car was moving slowly in forward gear without player input and is now moving slowly with player input that indicates the 
					//player wants to switch to reverse gear.
					toggleAutoReverse = true;
				}
				else if(currentGear == PxVehicleGearsData::eREVERSE && accelLeft && accelRight && !brakeLeft && !brakeRight && (currentGear == targetGear))
				{
					//The car was moving slowly in reverse gear without player input and is now moving slowly with player input that indicates the 
					//player wants to switch to forward gear.
					toggleAutoReverse = true;
				}
			}

			//If the car was brought to rest through braking then the player needs to release the brake then reapply
			//to indicate that the gears should toggle between reverse and forward.
			if(isMovingForwardSlowly && (!brakeLeft || !brakeRight) && (!accelLeft || !accelRight))
			{
				newIsMovingForwardSlowly = true;
			}
		}
	}
}

void SampleVehicle_VehicleController::update(const PxF32 timestep, const PxVehicleWheelQueryResult& vehicleWheelQueryResults, PxVehicleWheels& focusVehicle)
{
	PxVehicleDriveDynData* driveDynData=NULL;
	bool isTank=false;
	PxVehicleDriveTankControlModel::Enum tankDriveModel=PxVehicleDriveTankControlModel::eSTANDARD;
	switch(focusVehicle.getVehicleType())
	{
	case PxVehicleTypes::eDRIVE4W:
		{
			PxVehicleDrive4W& vehDrive4W=(PxVehicleDrive4W&)focusVehicle;
			driveDynData=&vehDrive4W.mDriveDynData;
			isTank=false;
		}
		break;
	case PxVehicleTypes::eDRIVENW:
		{
			PxVehicleDriveNW& vehDriveNW=(PxVehicleDriveNW&)focusVehicle;
			driveDynData=&vehDriveNW.mDriveDynData;
			isTank=false;
		}
		break;
	case PxVehicleTypes::eDRIVETANK:
		{
			PxVehicleDriveTank& vehDriveTank=(PxVehicleDriveTank&)focusVehicle;
			driveDynData=&vehDriveTank.mDriveDynData;
			isTank=true;
			tankDriveModel=vehDriveTank.getDriveModel();
		}
		break;
	default:
		PX_ASSERT(false);
		break;
	}

	//Toggle autogear flag
	if(mToggleAutoGears)
	{
		driveDynData->toggleAutoGears();
		mToggleAutoGears = false;
	}

	//Store raw inputs in replay stream if in recording mode.
	//Set raw inputs from replay stream if in replay mode.
	//Store raw inputs from active stream in handy arrays so we don't need to worry
	//about which stream (live input or replay) is active.
	//Work out if we are using keys or gamepad controls depending on which is being used
	//(gamepad selected if both are being used).
	PxVehicleDrive4WRawInputData carRawInputs;
	PxVehicleDriveTankRawInputData tankRawInputs(tankDriveModel);
	if(!isTank)
	{
		processRawInputs(timestep,driveDynData->getUseAutoGears(),carRawInputs);
	}
	else
	{
		processRawInputs(timestep,driveDynData->getUseAutoGears(),tankRawInputs);
	}

	//Work out if the car is to flip from reverse to forward gear or from forward gear to reverse.
	bool toggleAutoReverse = false;
	bool newIsMovingForwardSlowly = false;
	if(!isTank)
	{
		processAutoReverse(focusVehicle, *driveDynData, vehicleWheelQueryResults, carRawInputs, toggleAutoReverse, newIsMovingForwardSlowly);
	}
	else
	{
		processAutoReverse(focusVehicle, *driveDynData, vehicleWheelQueryResults, tankRawInputs, toggleAutoReverse, newIsMovingForwardSlowly);
	}
	mIsMovingForwardSlowly = newIsMovingForwardSlowly;


	//If the car is to flip gear direction then switch gear as appropriate.
	if(toggleAutoReverse)
	{
		mInReverseMode = !mInReverseMode;
		
		if(mInReverseMode)
		{
			driveDynData->forceGearChange(PxVehicleGearsData::eREVERSE);
		}
		else
		{
			driveDynData->forceGearChange(PxVehicleGearsData::eFIRST);
		}
	}

	//If in reverse mode then swap the accel and brake.
	if(mInReverseMode)
	{
		if(mUseKeyInputs)
		{
			if(!isTank)
			{
				const bool accel=carRawInputs.getDigitalAccel();
				const bool brake=carRawInputs.getDigitalBrake();
				carRawInputs.setDigitalAccel(brake);
				carRawInputs.setDigitalBrake(accel);
			}
			else
			{
				//Keyboard controls for tank not implemented yet.
				const bool accelLeft=tankRawInputs.getDigitalLeftThrust();
				const bool accelRight=tankRawInputs.getDigitalRightThrust();
				const bool brakeLeft=tankRawInputs.getDigitalLeftBrake();
				const bool brakeRight=tankRawInputs.getDigitalRightBrake();
				tankRawInputs.setDigitalLeftThrust(brakeLeft);
				tankRawInputs.setDigitalRightThrust(brakeRight);
				tankRawInputs.setDigitalLeftBrake(accelLeft);
				tankRawInputs.setDigitalRightBrake(accelRight);
			}
		}
		else
		{
			if(!isTank)
			{
				const PxF32 accel=carRawInputs.getAnalogAccel();
				const PxF32 brake=carRawInputs.getAnalogBrake();
				carRawInputs.setAnalogAccel(brake);
				carRawInputs.setAnalogBrake(accel);
			}
			else if(PxVehicleDriveTankControlModel::eSPECIAL==tankDriveModel)
			{
				const PxF32 thrustLeft=tankRawInputs.getAnalogLeftThrust();
				const PxF32 thrustRight=tankRawInputs.getAnalogRightThrust();
				tankRawInputs.setAnalogLeftThrust(-thrustLeft);
				tankRawInputs.setAnalogRightThrust(-thrustRight);
			}
			else
			{
				const PxF32 thrustLeft=tankRawInputs.getAnalogLeftThrust();
				const PxF32 thrustRight=tankRawInputs.getAnalogRightThrust();
				const PxF32 brakeLeft=tankRawInputs.getAnalogLeftBrake();
				const PxF32 brakeRight=tankRawInputs.getAnalogRightBrake();
				tankRawInputs.setAnalogLeftThrust(brakeLeft);
				tankRawInputs.setAnalogLeftBrake(thrustLeft);
				tankRawInputs.setAnalogRightThrust(brakeRight);
				tankRawInputs.setAnalogRightBrake(thrustRight);
			}
		}
	}

	// Now filter the raw input values and apply them to focus vehicle
	// as floats for brake,accel,handbrake,steer and bools for gearup,geardown.
	if(mUseKeyInputs)
	{
		if(!isTank)
		{
			const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
			PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs
				(gKeySmoothingData,gSteerVsForwardSpeedTable,carRawInputs,timestep,isInAir,(PxVehicleDrive4W&)focusVehicle);
		}
		else
		{
			PxVehicleDriveTankSmoothDigitalRawInputsAndSetAnalogInputs
				(gKeySmoothingData,tankRawInputs,timestep,(PxVehicleDriveTank&)focusVehicle);
		}
	}
	else
	{
		if(!isTank)
		{
			const bool isInAir = PxVehicleIsInAir(vehicleWheelQueryResults);
			PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs
				(gCarPadSmoothingData,gSteerVsForwardSpeedTable,carRawInputs,timestep,isInAir,(PxVehicleDrive4W&)focusVehicle);
		}
		else
		{
			PxVehicleDriveTankSmoothAnalogRawInputsAndSetAnalogInputs
				(gTankPadSmoothingData,tankRawInputs,timestep,(PxVehicleDriveTank&)focusVehicle);
		}
	}
}

void SampleVehicle_VehicleController::restart()
{
	const bool record=mRecord;
	const bool replay=mReplay;
	const PxU32 numSamples=mNumSamples;
	const PxU32 numRecordedSamples=mNumRecordedSamples;
	clear();
	mRecord=record;
	mReplay=replay;
	mNumRecordedSamples=numRecordedSamples;

	if(record)
	{
		PX_ASSERT(!replay);
		mNumRecordedSamples	= numSamples;
		mRecord				= false;
		mReplay				= true;
	}
}
