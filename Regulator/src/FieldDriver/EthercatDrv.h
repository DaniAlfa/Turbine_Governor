#ifndef ETHERCATDRV_H
#define ETHERCATDRV_H
#define IOMAP_DEFAULT_SIZE 255
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <CommonTypes.h>
#include <IOFieldDrv.h>

//Declaraciones adelantadas
class EtherDevice;


class EthercatDrv : public IOFieldDrv {
public:
	EthercatDrv();
	~EthercatDrv();

	bool init(std::string const& strConfigPath, std::function<void()> const& errorCallB);
	bool close();
	bool start();
	bool stop();
	DrvState getState() const {return mtDrvState;}
	std::string getLastErrorInfo() const {return mstrLastError;};

	bool read(IOVar & var);
	bool write(IOVar const& var);
	void getVarErrors(std::unordered_map<IOAddr, QState> & mErrors);

protected:
	friend class EtherDevice;

	bool writeDeviceSync(IOAddr const& addr, std::uint32_t uiVal, std::uint32_t tTimeOut);
	bool writeDevice(IOAddr const& addr, std::uint32_t uiVal);
	bool readDevice(IOAddr const& addr, std::uint32_t & uiVal);

	bool isModuleOk(IOAddr const& addr) const;
	void newVarError(IOAddr const& addr, QState eState);
	void clearVarError(IOAddr const& addr);

private:
	char mcIOmap[IOMAP_DEFAULT_SIZE];
	std::unordered_map<IOAddr, EtherDevice*> mDevices;
	std::unordered_map<IOAddr, QState> mVarErrors;
	std::unordered_set<std::uint8_t> mModuleErrors;
	std::vector<std::uint8_t> mSlavesComTrys; //Intentos realizados para restablecer la conexion por fallo en sincronizacion 

	bool mbDrvEnd;
	DrvState mtDrvState;
	std::string mstrLastError;
	std::function<void()> mferrorCallB;

	std::thread* mtDrvThread;
	std::mutex mtDrvStateMutex, mtIOMapMutex, mtVarErrorsMutex;
	std::condition_variable mtWaitingStart;
	std::condition_variable mtWaitingSend;

	std::string mstrIfname;

	bool writeIO(IOAddr const& addr, std::uint32_t uiVal, bool sync, std::uint32_t tTimeOut);
	void updateDevices();

	void driverLoop();
	void eraseDrvConfig();
	void checkSlavesWKC(int iWorkCounter);


	bool loadXMLConfig(std::string const& strConfigPath);
	bool readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot);
	bool readXMLSlaveCnf(xmlNode* pNode);
	bool readXMLVars(xmlDoc* pDocTree, xmlNode* pNode);
	
};



#endif
