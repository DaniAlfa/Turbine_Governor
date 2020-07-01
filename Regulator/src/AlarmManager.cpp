#include "AlarmManager.h"
#include <Alarms.h>
#include <cmath>
#include <IDVarList.h>
#include "IOImage.h"


AlarmManager::AlarmManager(IOImage & ioImg) : mtIOImg(ioImg), mbWithErrors(false){
	muiNumLogicErrorInts = (std::uint32_t) ceil((NUM_ALARMS) / (float) 32); 
	muiNumFieldQStatesInts = (std::uint32_t) ceil(((FLD_IN_VARS + FLD_OUT_VARS) * 2) / (float) 32);
	mpuiLogicErrors = new std::uint32_t[muiNumLogicErrorInts];
	mpuiFieldQStates = new std::uint32_t[muiNumFieldQStatesInts];
	mpuiAlarmLevels = new std::uint8_t[NUM_ALARMS];
	std::uint8_t puiAlarmLevels[NUM_ALARMS] = ALARM_LEVELS;
	for(int i = 0; i < NUM_ALARMS; ++i){
		mpuiAlarmLevels[i] = puiAlarmLevels[i];
	}
}
	

AlarmManager::~AlarmManager(){
	delete[] mpuiLogicErrors;
	delete[] mpuiFieldQStates;
	delete[] mpuiAlarmLevels;
}

void AlarmManager::driverFailed(){
	mbWithErrors = true;
}


void AlarmManager::checkDriverErrors(){
	if(mbWithErrors){
		mbWithErrors = false;
		DrvState fldState = mtIOImg.getFieldDrvState();
		DrvState slvState = mtIOImg.getSlaveDrvState();
		if(slvState == COMError){
			setAlarm(AL_MASTERCOM);
		}
		if(fldState == COMError){
			setAlarm(AL_FIELDCOM);
		}
		else if(fldState == VarError){
			mbWithErrors = true;
			setAlarm(AL_FIELDVARS);
			mtIOImg.getFldErrors(mMLastFldErrors);
			if(!mMLastFldErrors.empty()){
				clearFldStates();
				setFldStates();
			}

		}
		else {
			if(!mMLastFldErrors.empty()){
				mMLastFldErrors.clear(); 
				clearFldStates();
			}
		}
		
	}
}

void AlarmManager::clearFldStates(){
	for(std::uint32_t i = 0; i < muiNumFieldQStatesInts; ++i){
		mpuiFieldQStates[i] = 0;
	}
}

void AlarmManager::setFldStates(){
	for(auto it : mMLastFldErrors){
		std::uint32_t uiFldInt, uiStartBit;
		if(it.first >= FLD_IN_LOW_RANGE && it.first <= FLD_IN_HIGH_RANGE){
			uiStartBit = (it.first - FLD_IN_LOW_RANGE) * 2;
		}
		else if(it.first >= FLD_OUT_LOW_RANGE && it.first <= FLD_OUT_HIGH_RANGE){
			uiStartBit = (it.first - FLD_OUT_LOW_RANGE + FLD_IN_VARS) * 2;
		}
		else continue;
		uiFldInt = uiStartBit / 32;
		mpuiFieldQStates[uiFldInt] = mpuiFieldQStates[uiFldInt] & (~(0x3 << (31 - uiStartBit)));
		mpuiFieldQStates[uiFldInt] = mpuiFieldQStates[uiFldInt] | (((std::uint32_t) (it.second)) << (31 - uiStartBit));
	}
}

void AlarmManager::clearAlarms(){
	for(std::uint32_t i = 0; i < muiNumLogicErrorInts; ++i){
		mpuiLogicErrors[i] = 0;
	}
}

bool AlarmManager::isAlarmSet(std::uint32_t uiError) const{
	if(uiError >= NUM_ALARMS) return false;
	std::uint32_t uiErrorInt = (std::uint32_t) (uiError / 32);
	std::uint32_t uiMask = 0;
	uiMask = 0x1 << (31 - (std::uint32_t) (uiError % 32));
	return ((mpuiLogicErrors[uiErrorInt] & uiMask) > 0);
}

void AlarmManager::setAlarm(std::uint32_t uiError){
	if(uiError >= NUM_ALARMS) return;
	setAlarmBit(uiError);

	if(mpuiAlarmLevels[uiError] == 1){
		setAlarmBit(AL_MAYOR);
		if(AL_TRIPEXT != uiError) setAlarmBit(AL_TRIPINT);
		//mtIOImg[YA_REG_MA].setCurrentVal(0);
		//mtIOImg[YA_REG_TRIP].setCurrentVal(0);
	} 
	else{
		setAlarmBit(AL_MINOR);
		//mtIOImg[YA_REG_ME].setCurrentVal(0);
	}
}

void AlarmManager::setAlarmBit(std::uint32_t uiError){
	std::uint32_t uiErrorInt = (std::uint32_t) (uiError / 32);
	std::uint32_t uiMask = 0;
	uiMask = 0x1 << (31 - (std::uint32_t) (uiError % 32));
	mpuiLogicErrors[uiErrorInt] = mpuiLogicErrors[uiErrorInt] | uiMask;
}
