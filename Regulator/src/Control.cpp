#include "Control.h"
#include "IOImage.h"
#include "AlarmManager.h"


Control::Control(IOImage & ioImg, AlarmManager & alMan) : io(&ioImg), mpAlMan(&alMan){

}

Control::~Control(){

}



void Control::updateControl(){
	
}

bool Control::init(std::string const& strConfigPath){
	return true;
}