#include "EtherDevice.h"
#include <cstdint>
#include <chrono>
#include "EthercatDrv.h"

EtherDevice::EtherDevice(EthercatDrv & drv) : mtDrv(&drv), mbInError(false), mtLastQState(OK) {}

EtherDevice::~EtherDevice(){}


bool EtherDevice::writeDevice(IOAddr const& addr, std::uint32_t uiVal){
	return mtDrv->writeDevice(addr, uiVal);
}


bool EtherDevice::readDevice(IOAddr const& addr, std::uint32_t & uiVal){
	return mtDrv->readDevice(addr, uiVal);
}

bool EtherDevice::writeDeviceSync(IOAddr const& addr, std::uint32_t uiVal, std::uint32_t tTimeOut){
	return mtDrv->writeDeviceSync(addr, uiVal, tTimeOut);
}

std::int64_t EtherDevice::getMsSinceEpoch(){
	using namespace std::chrono;
	return duration_cast<duration<std::int64_t,std::milli>>(system_clock::now().time_since_epoch()).count();
}


float EtherDevice::convertBytes(std::uint32_t uiVal, EtherDevice::BitRepr repr, std::uint8_t uiNumBits){
	float fRetVal = 0;
	union{
		std::uint32_t* ui32;
		std::int32_t* i32;
		std::uint16_t* ui16;
		std::int16_t* i16;
		std::uint8_t* ui8;
		std::int8_t* i8;
	}reprType;
	reprType.ui32 = &uiVal;
	if(uiNumBits == 8){
		if(repr == Unsigned) fRetVal = *(reprType.ui8);
		else fRetVal = *(reprType.i8);
	}
	else if(uiNumBits == 16){
		if(repr == Unsigned) fRetVal = *(reprType.ui16);
		else fRetVal = *(reprType.i16);
	}
	else if(uiNumBits == 32){
		if(repr == Unsigned) fRetVal = *(reprType.ui32);
		else fRetVal = *(reprType.i32);
	}
	return fRetVal;
}

std::uint32_t EtherDevice::convertFloat(float fVal, std::uint8_t uiNumBits){
	std::uint32_t retVal = 0;
	union{
		std::uint32_t* ui32;
		std::uint16_t* ui16;
		std::uint8_t* ui8;
	}reprType;
	reprType.ui32 = &retVal;
	if(uiNumBits == 8){
		*(reprType.ui8) = fVal;
	}
	else if(uiNumBits == 16){
		*(reprType.ui16) = fVal;
	}
	else if(uiNumBits == 32){
		*(reprType.ui32) = fVal;
	}
	return retVal;
}

EtherDevice::BitRepr EtherDevice::getBitRepr(std::string strRepr){
	if(strRepr == "Unsigned") return Unsigned;
	else if(strRepr == "HalfSigned") return HalfSigned;
	else if(strRepr == "Signed") return Signed;
	return Invalid;
}


bool EtherDevice::isModuleOk(IOAddr const& addr) const{
	return mtDrv->isModuleOk(addr);
}

void EtherDevice::newVarError(IOAddr const& addr, QuState eState){
	mtDrv->newVarError(addr, eState);
}

void EtherDevice::clearVarError(IOAddr const& addr){
	mtDrv->clearVarError(addr);
}