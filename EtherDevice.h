#ifndef ETHERDEVICE_H
#define ETHERDEVICE_H
#include <cstdint>

//Declaraciones anticipadas
class EthercatDrv;
class IOVar;
class IOAddr;

class EtherDevice{
public:
	EtherDevice(EthercatDrv & drv);
	virtual bool read(IOVar & var) = 0;
	virtual bool write(IOVar const& var) = 0;
	virtual void updateDevice(IOAddr const& addr){}
	virtual ~EtherDevice();

protected:
	EthercatDrv* mtDrv;

	bool writeIOmap(IOAddr const& addr, std::int32_t val);
	bool readIOmap(IOAddr const& addr, std::int32_t & val);
	static std::int64_t getMsSinceEpoch();
};





#endif