#ifndef ANALOGDEV_H
#define ANALOGDEV_H
#include <cstdint>
#include "EtherDevice.h"
#include "CommonTypes.h"

//Declaraciones adelantadas
class EthercatDrv;


class AnalogDev : public EtherDevice{
public:
	class StatusByte{
		IOAddr tAddr;
		std::uint8_t uiUnderRangeBit;
		std::uint8_t uiOverRangeBit;
	};

	AnalogDev(EthercatDrv const& drv, StatusByte const& status);
	bool read(IOVar & var);
	bool write(IOVar const& var);

protected:
	StatusByte mtStatusB;

};



#endif