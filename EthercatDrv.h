#ifndef ETHERCATDRV_H
#define ETHERCATDRV_H
#define IOMAP_DEFAULT_SIZE 255
#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "CommonTypes.h"
#include "IOFieldDrv.h"

//Declaraciones adelantadas
class EtherDevice;
class IOVar;


class EthercatDrv : public IOFieldDrv {
public:
	EthercatDrv();
	~EthercatDrv();

	bool init(std::string const& strConfigPath);
	bool close();
	bool start();
	bool stop();
	DrvState getState() const {return mtDrvState;}
	std::string getLastErrorInfo() const {return mstrLastError;};

	bool read(IOVar & var);
	bool write(IOVar const& var);

protected:
	friend class EtherDevice;

	bool writeDeviceSync(IOAddr const& addr, std::int32_t val, std::uint32_t tTimeOut);
	bool writeDevice(IOAddr const& addr, std::int32_t val);
	bool readDevice(IOAddr const& addr, std::int32_t & val);

private:
	char mcIOmap[IOMAP_DEFAULT_SIZE];
	std::unordered_map<IOAddr, EtherDevice*> mDevices;

	bool mbDrvEnd;
	DrvState mtDrvState;
	std::string mstrLastError;

	std::thread* mtDrvThread;
	std::mutex mtDrvStateMutex, mtIOMapMutex;
	std::condition_variable mtWaitingStart;
	std::condition_variable mtWaitingSend;

	std::string mstrIfname;


	bool writeIO(IOAddr const& addr, std::int32_t val, bool sync, std::uint32_t tTimeOut);
	void driverLoop();
	void eraseDrvConfig();
	void updateDevices();
	bool loadXMLConfig(std::string const& strConfigPath);
	bool readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot);
	bool readXMLSlaveCnf(xmlNode* pNode);
	bool readXMLVars(xmlDoc* pDocTree, xmlNode* pNode);
	
};



#endif
