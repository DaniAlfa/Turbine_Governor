#ifndef DIGITALDEV_H
#define DIGITALDEV_H
#include <cstdint>
#include <chrono>
#include "EtherDevice.h"

//Declaraciones adelantadas
class EthercatDrv;

class DigitalDev : public EtherDevice{
public:

	DigitalDev(EthercatDrv & drv, int iPulseMeasureOpt = -1);
	bool read(IOVar & var);
	bool write(IOVar const& var);
	void updateDevice(IOAddr const& addr);

private:
	int miPulseMeasureOpt;
	std::chrono::steady_clock::time_point mtPulseStartTime;
	std::chrono::steady_clock::time_point mtPulseEndTime;
	double mdPulseDur;
	std::int64_t miPulseTimeS;
};



#endif