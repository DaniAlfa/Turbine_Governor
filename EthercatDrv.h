#ifndef ETHERCATDRV_H
#define ETHERCATDRV_H
#define IOMAP_DEFAULT_SIZE 255
#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include "CommonTypes.h"

#include "IOFieldDrv.h"

//Declaraciones adelantadas
class EtherDevice;
class IOVar;
class IOAddr;


class EthercatDrv : public IOFieldDrv {
public:
	EthercatDrv();
	~EthercatDrv();

	bool init(std::string const& strIfname, std::string & strError);
	bool close();
	bool start();
	bool stop();
	DrvState getState() const {return mtDrvState;}

	bool read(IOVar & var);
	bool write(IOVar const& var);


private:
	friend class EtherDevice;

	char mcIOmap[IOMAP_DEFAULT_SIZE];
	std::unordered_map<IOAddr, EtherDevice*> mDevices;

	bool mbDrvEnd;
	DrvState mtDrvState;

	std::thread* mtDrvThread;
	std::mutex mtDrvStateMutex, mtIOMapMutex;
	std::condition_variable mtWaitingStart;


	bool writeIOmap(IOAddr const& addr, std::int32_t val);
	bool readIOmap(IOAddr const& addr, std::int32_t & val);

	void driverLoop();
	void eraseDrvConfig();
	void updateDevices();
};



#endif