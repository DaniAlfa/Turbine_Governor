#include "PulseDev.h"
#define WRITE_TIMEOUT_MILLIS 2000

PulseDev::PulseDev(EthercatDrv & drv, ControlWord const& control) : EtherDevice(drv), mtControlB(control){
}



bool PulseDev::read(IOVar & var){
	std::int32_t readedVal;
	if(!readDevice(var.getAddr(), readedVal)) return false;

	var.setCurrentVal(readedVal);
	var.setTimeS(getMsSinceEpoch());
	return true;
}

bool PulseDev::write(IOVar const& var){
	if(writeDevice(var.getAddr(), var.getCurrentVal())){
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
	return control.tAddr.uiNumBits > 0 && control.tAddr.uiNumBits <= 32 && control.tAddr.uiNumBits % 8 == 0 && control.tAddr.uiNumBits != 24 && control.uiSetBit >= 0 && uiSetBit <= control.tAddr.uiNumBits - 1;
}