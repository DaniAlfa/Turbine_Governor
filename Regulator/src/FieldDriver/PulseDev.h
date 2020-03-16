#ifndef PULSEDEV_H
#define PULSEDEV_H
#include <cstdint>
#include "EtherDevice.h"
#include "CommonTypes.h"

//Declaraciones adelantadas
class EthercatDrv;


class PulseDev : public EtherDevice{
public:
	class ControlWord{
	public:
		IOAddr tAddr;
		std::uint8_t uiSetBit;
	};

	PulseDev(EthercatDrv & drv, ControlWord const& control);
	
	bool read(IOVar & var);
	bool write(IOVar const& var);

	static bool parametersOk(ControlWord const& control);

protected:
	ControlWord mtControlB;

};



#endif
