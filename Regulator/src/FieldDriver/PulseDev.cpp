#include "PulseDev.h"
#define WRITE_TIMEOUT_MILLIS 2000

PulseDev::PulseDev(EthercatDrv & drv, ControlWord const& control) : EtherDevice(drv), mtControlB(control){
}


bool PulseDev::read(IOVar & var){
	std::uint32_t uiReadedVal;
	if(!readDevice(var.getAddr(), uiReadedVal)) return false;

	var.setCurrentVal(convertBytes(uiReadedVal, Unsigned, var.getAddr().uiNumBits)); //Ampliable a representacion con signo
	var.setTimeS(getMsSinceEpoch());
	var.setQState(mtLastQState);
	return true;
}

bool PulseDev::write(IOVar const& var){
	if(writeDevice(var.getAddr(), convertFloat(var.getCurrentVal(), var.getAddr().uiNumBits))){
		IOAddr tSetAddr(mtControlB.tAddr);
		tSetAddr.uiChannel += mtControlB.uiSetBit;
		tSetAddr.uiNumBits = 1;
		if(writeDeviceSync(tSetAddr, 1, WRITE_TIMEOUT_MILLIS)){
			return writeDeviceSync(tSetAddr, 0, WRITE_TIMEOUT_MILLIS);
		}
	}
	return false;
}


bool PulseDev::parametersOk(ControlWord const& control){
	return control.tAddr.uiNumBits > 0 && control.tAddr.uiNumBits <= 32 && control.tAddr.uiNumBits % 8 == 0 && control.tAddr.uiNumBits != 24 && control.uiSetBit >= 0 && control.uiSetBit <= control.tAddr.uiNumBits - 1;
}

void PulseDev::updateDevice(IOAddr const& addr){
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
}