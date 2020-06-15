#ifndef MODBUSMASTERDRV_H
#define MODBUSMASTERDRV_H

#include <unordered_map>
#include <deque>
#include <queue>
#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <string>
#include <functional>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libmodbus/modbus.h>
#include <chrono>

#include "../CommonHMITypes.h"
#include "../IOMasterDrv.h"


class ModbusMasterDrv : public IOMasterDrv {
public:
	ModbusMasterDrv();
	~ModbusMasterDrv();

	//bool init(std::string const& strConfigPath, std::function<void(IOAddr)> const& writeTimeOutCallB, std::function<void()> const& comErrorCallB);
	bool init(std::string const& strConfigPath, std::unordered_set<IOAddr> const& slaveVarsToUpdate, std::unordered_set<IOAddr> const& fieldVarsToUpdate, std::function<void()> const& comErrorCallB, std::function<void()> const& recoveredFromErrorCallB);
	bool close();
	bool start();
	bool stop();
	DrvState getState() const {return mtDrvState;}
	std::string getLastErrorInfo() const {return mstrLastError;};

	bool read(VarImage & var, IOAddr const tAddr); //Para leer variables fisicas y modbus

	bool write(float const val, IOAddr const& tAddr, std::function<void(IOAddr)> const& timeOut, std::uint32_t tWriteTimeOut, std::function<void(IOAddr)> const& writeSuccess); //Para escribr en variables modbus o fisicas representadas como solo escritura
	bool force(float const fVal, IOAddr const& tAddr, bool bForceBitVal, std::function<void(IOAddr)> const& timeOut, std::uint32_t tWriteTimeOut, std::function<void(IOAddr)>const& writeSuccess);
	
	bool read(std::uint32_t & uiVal, IOAddr const tAddr); //Lectura de enteros con bitstrings

	void getChangedVars(std::unordered_set<IOAddr> & usChanges);

private:
	bool mbArchLittleEnd;

	class FieldData{
	public:
		std::int64_t miTimeS;
		float mfTrueVal;
		float mfForcedVal;
		QuState mtQState;
		bool mbForced;
		bool mbForceChangeDetection = true;
	};

	class ModbusData{
	public:
		std::int64_t miTimeS;
		float mfCurrentVal;
		std::uint32_t muiCurrentVal;
		QuState mtQState;
		bool mbForceChangeDetection = true;
	};

	class WriteReq{
	public:
		IOAddr mtAddr;
		bool mbForce;
		bool mbForcedVal;
		float mfWriteVal;
		std::uint32_t mtWriteTimeOut;
		std::function<void(IOAddr)> mfWriteSuccess;
		std::function<void(IOAddr)> mfWriteTimeOut;
		std::int64_t mTimeS;
		bool operator<(WriteReq const& other) const {
			return mtWriteTimeOut < other.mtWriteTimeOut;
		}
		class Comp{
		public:
			bool operator()(WriteReq const* wr1, WriteReq const* wr2) const{
				return (*wr1) < (*wr2);
			}
		};

	};

	std::unordered_map<IOAddr, FieldData*> mFieldVars;
	std::unordered_map<IOAddr, ModbusData*> mModbusVars;

	std::unordered_map<IOAddr, FieldData*>::iterator mFieldIt;
	std::unordered_map<IOAddr, ModbusData*>::iterator mModbusIt;

	std::priority_queue<WriteReq*, std::deque<WriteReq*>, WriteReq::Comp> mPendingWrites;

	std::unordered_set<IOAddr> musChangedVars;
	

	//Configuracion
	std::string mstrServerIp;
	int miPort;
	std::uint32_t muiNumIOVars;
	std::uint32_t muiNumROInts;

    
    //Control de cliente
    modbus_t * mpMBCtx;
    bool bConnected;
   	std::uint32_t uiComErrors;
   	std::function<void()> mfComErrorCallB;
   	std::function<void()> mfRecoveredFromErrorCallB;


	bool mbDrvEnd;
	DrvState mtDrvState;
	std::string mstrLastError;
	std::chrono::milliseconds mMillisToWriteCycle;
	std::chrono::milliseconds mMillisToReadCycle;
	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::milliseconds> mLastTimePoint;
	bool mbInWriteCycle;
	bool mbInReadCycle;

	std::thread* mtDrvThread;
	std::mutex mtDrvStateMutex, mtIOMapMutex, mtChagesSetMutex;
	std::condition_variable mtWaitingStart;

	bool createConnection();
	void closeConnection();
	bool processNextRead();
	bool processNextWrite();

	void cleanWriteRequests(bool bAll = false);
	bool processRead(IOAddr tAddr, FieldData* pFieldVar);
	bool processRead(IOAddr tAddr, ModbusData* pMBVar);

	void driverLoop();
	void eraseDrvConfig();
	void eraseVars();

	void readModbusVar(VarImage & var, std::unordered_map<IOAddr, ModbusData*>::const_iterator it);
	void readFieldVar(VarImage & var, std::unordered_map<IOAddr, FieldData*>::const_iterator it);
	void getTimeS(std::int64_t & iReadTs, std::uint16_t const uiTS[4]) const;
	std::uint32_t getInt(std::uint16_t const uiInt[2]) const;
	QuState getQState(std::uint8_t const uiQstate[2]) const;
	static std::int64_t getMsSinceEpoch();


	bool loadXMLConfig(std::string const& strConfigPath);
	bool readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot);
	bool readXMLVars(xmlDoc* pDocTree, xmlNode* pNode, bool bField);
	static bool parseXML8bitIntAttr(xmlNode* pNode, const char* attr, std::uint8_t & iVal);
	static bool parseXMLAddr(xmlNode* pNode, IOAddr & addr);

};



#endif