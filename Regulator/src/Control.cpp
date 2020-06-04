#include "Control.h"
#include "IOImage.h"
#include "AlarmManager.h"
#include "RegIDS.h"
#include <iostream>

Control::Control(IOImage & ioImg, AlarmManager & alMan) : io(ioImg), mpAlMan(&alMan){
	state = false;
	button = false;
}

Control::~Control(){

}



void Control::updateControl(){
	//std::cout << "Arranque Valor: " << io[HMSM_REG_STR].getCurrentVal() << std::endl;
	if(io[HMSM_REG_STR].getCurrentVal() == 1){
		if(!button){
			float currentSpeed = io[SE_REG_F1].getCurrentVal();
			currentSpeed += 10;
			if(currentSpeed > 60.0) currentSpeed = 0;
			io[SE_REG_F1].setCurrentVal(currentSpeed);
			if(!state){
				io[SIS_TUR_SYNC].setCurrentVal(1);
				state = true;
			} 
			else{
				io[SIS_TUR_SYNC].setCurrentVal(0);
				state = false;
			}
			button = true;
		}
		//std::cout << "Arranque activado, Valor: " << io[HMSM_REG_STR].getCurrentVal() << std::endl;
		
	}
	else button = false;
}

bool Control::init(std::string const& strConfigPath){
	return true;
}