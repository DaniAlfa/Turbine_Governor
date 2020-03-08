#include "DigitalDev.h"
#include <chrono>
#include "CommonTypes.h"

DigitalDev::DigitalDev(EthercatDrv & drv, int iPulseMeasureOpt) : EtherDevice(drv), miPulseMeasureOpt(iPulseMeasureOpt){
	if(miPulseMeasureOpt < -1) miPulseMeasureOpt = -1;
	else if(miPulseMeasureOpt > 1) miPulseMeasureOpt = 1;
}


bool DigitalDev::read(IOVar & var){
	std::int32_t readedVal;
	if(!readDevice(var.getAddr(), readedVal)) return false;
	var.setCurrentVal((readedVal > 0) ? 1 : 0);
	var.setTimeS(getMsSinceEpoch());
	var.setPulseInfo(mdPulseDur, miPulseTimeS);
	return true;
}


bool DigitalDev::write(IOVar const& var){
	return writeDevice(var.getAddr(), (var.getCurrentVal() > 0) ? 1 : 0);
}


void DigitalDev::updateDevice(IOAddr const& addr){
	using namespace std::chrono;
	if(miPulseMeasureOpt == -1) return;
	std::int32_t readedVal;
	if(!readDevice(addr, readedVal)) return;
	if(mtPulseEndTime >= mtPulseStartTime && miPulseMeasureOpt == readedVal){ //Comienzo pulso
		mtPulseStartTime = steady_clock::now();
	}
	else if(mtPulseStartTime > mtPulseEndTime && miPulseMeasureOpt != readedVal){ //Fin pulso
		mtPulseEndTime = steady_clock::now();
		mdPulseDur = duration_cast<duration<double, std::milli>>(mtPulseEndTime - mtPulseStartTime).count();
		miPulseTimeS = getMsSinceEpoch();
	}
}