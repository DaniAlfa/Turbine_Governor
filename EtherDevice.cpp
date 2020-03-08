#include "EtherDevice.h"
#include "CommonTypes.h"
#include <cstdint>
#include <chrono>
#include "EthercatDrv.h"

EtherDevice::EtherDevice(EthercatDrv & drv) : mtDrv(&drv) {

}

EtherDevice::~EtherDevice(){

}


bool EtherDevice::writeDevice(IOAddr const& addr, std::int32_t val){
	return mtDrv->writeDevice(addr, val);
}


bool EtherDevice::readDevice(IOAddr const& addr, std::int32_t & val){
	return mtDrv->readDevice(addr, val);
}

bool EtherDevice::writeDeviceSync(IOAddr const& addr, std::int32_t val, std::uint32_t tTimeOut){
	return mtDrv->writeDeviceSync(addr, val, tTimeOut);
}

std::int64_t EtherDevice::getMsSinceEpoch(){
	using namespace std::chrono;
	return duration_cast<duration<std::int64_t,std::milli>>(system_clock::now().time_since_epoch()).count();
}
