#include "Control.h"
#include "IOImage.h"
#include "AlarmManager.h"
#include "RegIDS.h"
#include <Alarms.h>
#include <iostream>

Control::Control(IOImage & ioImg, AlarmManager & alMan) : io(ioImg), mpAlMan(&alMan){
	iState = 0;
	button = false;
}

Control::~Control(){

}



void Control::updateControl(){
	//std::cout << "Arranque Valor: " << io[HMSM_REG_STR].getCurrentVal() << std::endl;
	//io[Y_REG_LOC].setCurrentVal(1);
	if(iState == 0 && io[HMSM_REG_STR].getCurrentVal() == 1){
		iState = 1;
	}
	
	if(io[HMSM_REG_TRIP].getCurrentVal() == 0){
		mpAlMan->setAlarm(AL_TRIPEXT);
	}else{
		if(io[HMSM_REG_ALREP].getCurrentVal() == 1){
			//std::cout << "Clear" << std::endl;
			mpAlMan->clearAlarms();
		}
	}
	
	if(iState == 1 || iState == 4){
		
		float currentSpeed = io[SE_REG_F1].getCurrentVal();
		currentSpeed += 0.009;
		if(currentSpeed > 80.0) currentSpeed = 0;
		io[SE_REG_F1].setCurrentVal(currentSpeed);
		io[SE_REG_F2].setCurrentVal(currentSpeed);
		io[SE_REG_F3].setCurrentVal(currentSpeed);
		io[SIS_TUR_N0].setCurrentVal(currentSpeed < 0.5);
		io[SIS_TUR_BRK].setCurrentVal(currentSpeed < 10);
		io[SIS_TUR_EXC].setCurrentVal(currentSpeed > 50);
		io[SIS_TUR_SYNC].setCurrentVal(currentSpeed > 59);
		if(currentSpeed > 59){
			io[ZC_REG_SSP].setCurrentVal(60);
			iState = 4;
		} 
		io[SAH_TUR_OVS].setCurrentVal(!(currentSpeed > 62));
		io[SAHH_TUR_OVS].setCurrentVal(!(currentSpeed > 70));
		if(currentSpeed > 65)
		{
			io[ZC_REG_PWSP].setCurrentVal(50);
			iState = 6;
		} 
	}
	else if(iState == 6){
		if(io[HMSM_REG_OPREG].getCurrentVal() == 1) {
			iState = 7;
		}
		else if(io[HMSM_REG_SPOK].getCurrentVal() == 1){
			io[ZC_REG_PWSP].setCurrentVal(io[ZR_REG_PWSP].getCurrentVal());
		}
	}
	else if(iState == 7){
		if(io[HMSM_REG_PWREG].getCurrentVal() == 1) {
			iState = 6;
		}
		else{
			io[ZC_REG_OPSP].setCurrentVal(io[ZR_REG_OPSP].getCurrentVal());
		}
	}
	io[Y_REG_STATE].setCurrentVal(iState);
}

bool Control::init(std::string const& strConfigPath){
	return true;
}