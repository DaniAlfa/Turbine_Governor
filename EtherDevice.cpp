#include "EtherDevice.h"
#include "CommonTypes.h"
#include <cstdint>
#include <chrono>
#include "EthercatDrv.h"

EtherDevice::EtherDevice(EthercatDrv & drv) : mtDrv(&drv) {

}

EtherDevice::~EtherDevice(){

}


bool EtherDevice::writeIOmap(IOAddr const& addr, std::int32_t val){
	return mtDrv->writeIOmap(addr, val);
}


bool EtherDevice::readIOmap(IOAddr const& addr, std::int32_t & val){
	return mtDrv->readIOmap(addr, val);
}

std::int64_t EtherDevice::getMsSinceEpoch(){
	using namespace std::chrono;
	return duration_cast<duration<std::int64_t,std::milli>>(system_clock::now().time_since_epoch()).count();
}