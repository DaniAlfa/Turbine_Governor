#include "DigitalDev.h"
#include <chrono>
#include "../CommonRegTypes.h"

DigitalDev::DigitalDev(EthercatDrv & drv, int iPulseMeasureOpt) : EtherDevice(drv), miPulseMeasureOpt(iPulseMeasureOpt){
	if(miPulseMeasureOpt < -1) miPulseMeasureOpt = -1;
	else if(miPulseMeasureOpt > 1) miPulseMeasureOpt = 1;
}


bool DigitalDev::read(IOVar & var){
	std::uint32_t readedVal;
	if(!readDevice(var.getAddr(), readedVal)) return false;
	var.setCurrentVal((readedVal > 0) ? 1 : 0);
	var.setTimeS(getMsSinceEpoch());
	var.setPulseInfo(mdPulseDur, miPulseTimeS);
	var.setQState(mtLastQState);
	return true;
}


bool DigitalDev::write(IOVar const& var){
	return writeDevice(var.getAddr(), (var.getCurrentVal() > 0) ? 1 : 0);
}


void DigitalDev::updateDevice(IOAddr const& addr){
	if(!isModuleOk(addr)){
		if(!mbInError) newVarError(addr, QuState::ComError);
		mbInError = true;
		mtLastQState = QuState::ComError;
		return;
	}
	else if(mbInError){
		mbInError = false;
		mtLastQState = OK;
		clearVarError(addr);
	} 
	
	using namespace std::chrono;
	if(miPulseMeasureOpt == -1) return;
	std::uint32_t readedVal;
	if(!readDevice(addr, readedVal)) return;
	if(mtPulseEndTime >= mtPulseStartTime && miPulseMeasureOpt == (std::int8_t) readedVal){ //Comienzo pulso
		mtPulseStartTime = steady_clock::now();
	}
	else if(mtPulseStartTime > mtPulseEndTime && miPulseMeasureOpt != (std::int8_t) readedVal){ //Fin pulso
		mtPulseEndTime = steady_clock::now();
		mdPulseDur = duration_cast<duration<double, std::milli>>(mtPulseEndTime - mtPulseStartTime).count();
		miPulseTimeS = getMsSinceEpoch();
	}
}