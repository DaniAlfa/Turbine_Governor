#ifndef MODBUSSLAVEDRV_H
#define MODBUSSLAVEDRV_H

#include <unordered_map>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <functional>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libmodbus/modbus.h>

#include <CommonTypes.h>
#include <IOSlaveDrv.h>

//Especificos de linux
#include <sys/select.h>


class ModbusSlaveDrv : public IOSlaveDrv {
public:
	ModbusSlaveDrv();
	~ModbusSlaveDrv();

	bool init(std::string const& strConfigPath, std::function<void()> const& comErrorCallB = []{});
	bool close();
	bool start();
	bool stop();
	DrvState getState() const {return mtDrvState;}
	std::string getLastErrorInfo() const {return mstrLastError;};

	bool read(IOVar & var);
	bool write(IOVar const& var);
	bool updateFieldVar(IOVar & var);

private:
	bool mbArchLittleEnd;

	std::unordered_map<std::uint32_t, IOAddr> mFieldVars;

	//Configuracion
	std::string mstrServerIp;
	int miPort;
	std::uint32_t muiNumVars;

    modbus_t * mpMBCtx;
    modbus_mapping_t * mpMBmapping;
    std::uint8_t muiQuery[MODBUS_TCP_MAX_ADU_LENGTH];
    
    //Control de servidor
    int miServerSock;
    fd_set mtRefset;
    fd_set mtReadset;
    int miFdmax;
    std::uint32_t muiNumConnections;
   	std::uint32_t uiComErrors;
   	std::function<void()> mfComErrorCallB;


	bool mbDrvEnd;
	DrvState mtDrvState;
	std::string mstrLastError;

	std::thread* mtDrvThread;
	std::mutex mtDrvStateMutex, mtIOMapMutex;
	std::condition_variable mtWaitingStart;

	bool createServerSocket();
	void processPendingRequests();
	void closeSockets();

	void driverLoop();
	void eraseDrvConfig();

	float getCurrentVal(std::uint8_t uiVar);
	float getForcedVal(std::uint8_t uiVar);
	bool getForced(std::uint8_t uiVar);
	void setCurrentVal(float fVal, std::uint8_t uiVar);
	void setQState(QState tState, std::uint8_t uiVar);
	void setTimeS(std::int64_t const& iTimeS, std::uint8_t uiVar);
	static std::int64_t getMsSinceEpoch();


	bool loadXMLConfig(std::string const& strConfigPath);
	bool readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot);
	bool readXMLVars(xmlDoc* pDocTree, xmlNode* pNode);
	static bool parseXML8bitIntAttr(xmlNode* pNode, const char* attr, std::uint8_t & iVal);
	static bool parseXMLAddr(xmlNode* pNode, IOAddr & addr);

};



#endif
