#include "ModbusMasterDrv.h"

#include <chrono>
#include <cstdlib>
#include <utility>
#include <errno.h>

#define DIOFFSET 2
#define DIADDRESS 10001
#define COILOFFSET 1
#define COILADDRESS 1
#define IRADDRESS 30001
#define IROFFSET 4
#define HRADDRESS 40001
#define HROFFSET 4

#define MAX_TRANSMISSION_TRYS 3
#define TRANSMISSION_TRYS_TIME 15

#define COM_ERROR_MIN_TIMEOUT 500
#define MAX_COM_ERROR_TRYS 3

#define POOLING_DELAY_MILLIS 10

using namespace std;

ModbusMasterDrv::ModbusMasterDrv() : mpMBCtx(NULL), bConnected(false), bLastOpRead(false), mbDrvEnd(false), mtDrvState(UnInit){
	std::uint32_t iNum = 1;
    std::uint8_t *pNum = (std::uint8_t*)&iNum;
    mbArchLittleEnd = (*pNum == 1);
	mtDrvThread = new thread(&ModbusMasterDrv::driverLoop, this);
}

ModbusMasterDrv::~ModbusMasterDrv(){
	mbDrvEnd = true;
	mtWaitingStart.notify_one();
	mtDrvThread->join();
	delete mtDrvThread;
	cleanWriteRequests(true);
	closeConnection();
	eraseDrvConfig();
}

bool ModbusMasterDrv::close(){
	if(mtDrvState == UnInit) return true;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = UnInit;
	cleanWriteRequests(true);
	closeConnection();
	eraseDrvConfig();
	return true;
}

void ModbusMasterDrv::eraseDrvConfig(){
	if(mpMBCtx != NULL) modbus_free(mpMBCtx);
	mpMBCtx = NULL;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	eraseVars();
    mutexIOMap.unlock();
    mFieldIt = mFieldVars.begin();
    mModbusIt = mModbusVars.begin();
    muiNumIOVars = 0;
    muiNumROInts = 0;
    uiComErrors = 0;
}

bool ModbusMasterDrv::start(){
	if(mtDrvState == UnInit) return false;
	if(mtDrvState == Stopped){
		unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
		mtDrvState = Running;
		mtWaitingStart.notify_one();
	}
	return true;
}

bool ModbusMasterDrv::stop(){
	if(mtDrvState == UnInit) return false;
	if(mtDrvState == Stopped) return true;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	cleanWriteRequests(true);
	closeConnection();
	mtDrvState = Stopped;
	uiComErrors = 0;
	return true;
}

void ModbusMasterDrv::driverLoop(){
	while(!mbDrvEnd){
		unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
		switch(mtDrvState){
			case Stopped:
			case UnInit:
				while((mtDrvState == UnInit || mtDrvState == Stopped) && !mbDrvEnd){
					mtWaitingStart.wait(mutexDrvState);
				}
			break;
			case Running:
				if(!bConnected){
					if(!createConnection()){
						mtDrvState = COMError;
					}
				}else{
					mutexDrvState.unlock();
					this_thread::sleep_for(chrono::milliseconds(POOLING_DELAY_MILLIS));
					mutexDrvState.lock();
					if(mtDrvState != Running) break;
					bool bOk = true;
					if(bLastOpRead && !mPendingWrites.empty()){
						bOk = processNextWrite();
						bLastOpRead = false;
					}
        			else {
        				bOk = processNextRead();
        				bLastOpRead = true;
        			}
        			if(!bOk){
						mtDrvState = COMError;
        			}
				}	
			break;
			case COMError:
				if(!bConnected){
					if(createConnection()){
						mtDrvState = Running;
					}
					else{
						if(uiComErrors == 0) mfComErrorCallB();
						if(++uiComErrors > MAX_COM_ERROR_TRYS) uiComErrors = MAX_COM_ERROR_TRYS;
						cleanWriteRequests();
						mutexDrvState.unlock();
						this_thread::sleep_for(chrono::milliseconds(COM_ERROR_MIN_TIMEOUT * uiComErrors));
						mutexDrvState.lock();
					}
				}
				else{
					closeConnection();
				}
			break;
			case VarError:
			default:
				mtDrvState = COMError;
		}
	}
}

void ModbusMasterDrv::closeConnection(){
	if(bConnected){
		modbus_flush(mpMBCtx);
		modbus_close(mpMBCtx);
		bConnected = false;
	}
}

bool ModbusMasterDrv::processNextRead(){
	if(mFieldIt == mFieldVars.end() && mModbusIt == mModbusVars.end()){
		mFieldIt = mFieldVars.begin();
		mModbusIt = mModbusVars.begin();
		if(mFieldIt == mFieldVars.end() && mModbusIt == mModbusVars.end()) return true; //Si el driver no tiene variables para leer
	}
	if(mFieldIt != mFieldVars.end()){
		if(processRead(mFieldIt->first, mFieldIt->second)){
			++mFieldIt;
			return true;
		}
	}
	else if (mModbusIt != mModbusVars.end()){
		if(processRead(mModbusIt->first, mModbusIt->second)){
			++mModbusIt;
			return true;
		}
	}
	return false;
}

bool ModbusMasterDrv::processRead(IOAddr tAddr, FieldData* pFieldVar){
	int uiNumVar = tAddr.uiChannel - muiNumROInts;
	bool bVals, bTS, bQstate, bforced;
	unsigned iNumTrys = 0; 
	bVals = bTS = bQstate = bforced = false;
	std::uint16_t uiVals[4];
	std::uint16_t uiTS[4];
	std::uint8_t uiForced;
	std::uint8_t uiQstate[2];
	while(iNumTrys++ < MAX_TRANSMISSION_TRYS && (!bVals || !bTS || !bQstate || !bforced)){
		if(!bVals){
			bVals = modbus_read_registers(mpMBCtx, (HRADDRESS + ((uiNumVar * HROFFSET) - HROFFSET)), 4, uiVals) == 4;
		}
		if(bVals && !bTS){
			bTS = modbus_read_input_registers(mpMBCtx, (IRADDRESS + ((uiNumVar * IROFFSET) - IROFFSET) + muiNumROInts*2), 4, uiTS) == 4;
		}
		if(bVals && bTS && !bQstate){
			bQstate = modbus_read_input_bits(mpMBCtx, (DIADDRESS + ((uiNumVar * DIOFFSET) - DIOFFSET)), 2, uiQstate) == 2;
		}
		if(bVals && bTS && bQstate && !bforced){
			bforced = modbus_read_bits(mpMBCtx, (COILADDRESS + ((uiNumVar * COILOFFSET) - COILOFFSET)), 1, &uiForced) == 1;
		}
		if((!bVals || !bTS || !bQstate || !bforced)){
			this_thread::sleep_for(chrono::milliseconds(TRANSMISSION_TRYS_TIME));
		}
	}
	if(iNumTrys >= MAX_TRANSMISSION_TRYS + 1){
		mstrLastError = std::string(modbus_strerror(errno));
		return false;
	}
	float fTrueVal = modbus_get_float_abcd(uiVals);
	float fForcedVal = modbus_get_float_abcd(uiVals + 2);
	QState tqstate = getQState(uiQstate);
	bool bForced = (uiForced >= 1);
	if(pFieldVar->mbForceChangeDetection || fTrueVal != pFieldVar->mfTrueVal || pFieldVar->mfForcedVal != fForcedVal || tqstate != pFieldVar->mtQState || pFieldVar->mbForced != bForced){
		unique_lock<mutex> mutexChagesSet(mtChagesSetMutex);
		musChangedVars.insert(tAddr);
		pFieldVar->mbForceChangeDetection = false;
	}
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	pFieldVar->mfTrueVal = fTrueVal;
	pFieldVar->mfForcedVal = fForcedVal;
	getTimeS(pFieldVar->miTimeS, uiTS);
	pFieldVar->mtQState = tqstate;
	pFieldVar->mbForced = bForced;
	return true;
}

bool ModbusMasterDrv::processRead(IOAddr tAddr, ModbusData* pMBVar){
	if(tAddr.uiChannel >= 1 && tAddr.uiChannel <= muiNumROInts){
		int varAddr = (IRADDRESS + ((uiNumVar - 1) * 2));
		std::uint16_t uiROInt[2];
		unsigned iNumTrys = 0; 
		while(iNumTrys++ < MAX_TRANSMISSION_TRYS && modbus_read_input_registers(mpMBCtx, varAddr, 2, uiROInt) == -1) 
			this_thread::sleep_for(chrono::milliseconds(TRANSMISSION_TRYS_TIME));
		if(iNumTrys >= MAX_TRANSMISSION_TRYS + 1){
			mstrLastError = std::string(modbus_strerror(errno));
			return false;
		}
		std::uint32_t uiCurrentVal = getInt(uiROInt);
		if(pMBVar->mbForceChangeDetection || uiCurrentVal != pMBVar->muiCurrentVal || pMBVar->mtQState != OK){
			unique_lock<mutex> mutexChagesSet(mtChagesSetMutex);
			musChangedVars.insert(tAddr);
			pMBVar->mbForceChangeDetection = false;
		}
		unique_lock<mutex> mutexIOMap(mtIOMapMutex);
		pMBVar->muiCurrentVal = uiCurrentVal;
		pMBVar->miTimeS = getMsSinceEpoch();
		pMBVar->mtQState = OK;
	}
	else{
		int varAddr = HRADDRESS + (((tAddr.uiChannel- muiNumROInts) * HROFFSET) - HROFFSET);
		std::uint16_t uiCurrentVal[2];
		unsigned iNumTrys = 0; 
		while(iNumTrys++ < MAX_TRANSMISSION_TRYS && modbus_read_registers(mpMBCtx, varAddr, 2, uiCurrentVal) == -1) 
			this_thread::sleep_for(chrono::milliseconds(TRANSMISSION_TRYS_TIME));
		if(iNumTrys >= MAX_TRANSMISSION_TRYS + 1){
			mstrLastError = std::string(modbus_strerror(errno));
			return false;
		}
		float fCurrentVal = modbus_get_float_abcd(uiCurrentVal);
		if(pMBVar->mbForceChangeDetection || pMBVar->mfCurrentVal != fCurrentVal || pMBVar->mtQState != OK){
			unique_lock<mutex> mutexChagesSet(mtChagesSetMutex);
			musChangedVars.insert(tAddr);
			pMBVar->mbForceChangeDetection = false;
		}
		unique_lock<mutex> mutexIOMap(mtIOMapMutex);
		pMBVar->mfCurrentVal = fCurrentVal;
		pMBVar->miTimeS = getMsSinceEpoch();
		pMBVar->mtQState = OK;
	}
	return true;
}

void ModbusMasterDrv::getTimeS(std::int64_t & iReadTs, std::uint16_t const uiTS[4]) const{
	std::uint8_t* pMap = (std::uint8_t*) uiTS;
	std::uint8_t* pVal = (std::uint8_t*) &iReadTs;
	if(mbArchLittleEnd) pVal = pVal + 7;
	for(int i = 0; i < 8; ++i){
		*pVal = *pMap;
		if(mbArchLittleEnd) --pVal;
		else ++pVal;
		++pMap;
	}
}

std::uint32_t ModbusMasterDrv::getInt(std::uint16_t const uiInt[2]) const{
	std::uint32_t iVal = 0;
	std::uint8_t* pMap = (std::uint8_t*) uiInt;
	std::uint8_t* pVal = (std::uint8_t*) &iVal;
	if(mbArchLittleEnd) pVal = pVal + 3;
	for(int i = 0; i < 4; ++i){
		*pVal = *pMap;
		if(mbArchLittleEnd) --pVal;
		else ++pVal;
		++pMap;
	}
	return iVal;
}

QState ModbusMasterDrv::getQState(std::uint8_t const uiQstate[2]) const{
	std::uint8_t const* pDi = uiQstate;
	std::uint8_t uiVal = 0;
	std::uint8_t uiOffset = DIOFFSET - 1;
	for(int i = 0; i < DIOFFSET; ++i, --uiOffset, ++pDi){
		uiVal = uiVal | ((*pDi) << uiOffset);
	}
	return (QState) uiVal;
}

bool ModbusMasterDrv::processNextWrite(){
	cleanWriteRequests();
	if(mPendingWrites.empty()) return true;
	WriteReq* writeReq = mPendingWrites.top();
	std::uint16_t uiAddr = HRADDRESS + (((writeReq->mtAddr.uiChannel - muiNumROInts) * HROFFSET) - HROFFSET);
	if(writeReq->mbForce) uiAddr += 2;
	std::uint16_t uiWriteVal[2];
	modbus_set_float_abcd(writeReq->mfWriteVal, uiWriteVal);
	unsigned iNumTrys = 0;
	bool bForced = !writeReq->mbForce;
	bool bWriteVal = writeReq->mbForce && !writeReq->mbForcedVal;
	while(iNumTrys++ < MAX_TRANSMISSION_TRYS && (!bForced || !bWriteVal)){
		if(!bForced){
			bForced = modbus_write_bit(mpMBCtx, (COILADDRESS + (((writeReq->mtAddr.uiChannel - muiNumROInts) * COILOFFSET) - COILOFFSET)), writeReq->mbForcedVal) == 1;
		}
		if(bForced && !bWriteVal){
			bWriteVal = modbus_write_registers(mpMBCtx, uiAddr, 2, uiWriteVal) == 2;
		}
		if((!bForced || !bWriteVal)){
			this_thread::sleep_for(chrono::milliseconds(TRANSMISSION_TRYS_TIME));
		}
	}
	if(iNumTrys >= MAX_TRANSMISSION_TRYS + 1){
		mstrLastError = std::string(modbus_strerror(errno));
		return false;
	}
	mPendingWrites.pop();
	if(writeReq->mpfWriteSuccess != nullptr){
		*(writeReq->mpfWriteSuccess)(writeReq->mtAddr);
	}
	delete writeReq;
	return true;
}

void ModbusMasterDrv::cleanWriteRequests(bool bAll){
	std::int64_t iTimeS = getMsSinceEpoch();
	WriteReq* writeReq;
	while(!mPendingWrites.empty()){
		writeReq = mPendingWrites.top();
		if(iTimeS - writeReq->mTimeS > writeReq->mtWriteTimeOut || bAll){
			mPendingWrites.pop();
			if(writeReq->mpfWriteTimeOut != nullptr){
				*(writeReq->mpfWriteTimeOut)(writeReq->mtAddr);
			}
			delete writeReq;
		} 
		else break;
	}
}


bool ModbusMasterDrv::createConnection(){
	if(!bConnected){
		if(modbus_connect(mpMBCtx) == -1){
			mstrLastError = std::string(modbus_strerror(errno));
			return false;
		}
		bConnected = true;
		uiComErrors = 0;
	}
	return true;
}

bool ModbusMasterDrv::init(std::string const& strConfigPath, std::unordered_set<IOAddr> const& slaveVarsToUpdate, std::unordered_set<IOAddr> const& fieldVarsToUpdate, std::function<void()> const& comErrorCallB){
	if(mtDrvState != UnInit){
		mstrLastError = "Driver ya inicializado";
		return false;
	}

	if(!loadXMLConfig(strConfigPath)){
		return false;
	}

	if(miPort == -1) miPort = MODBUS_TCP_DEFAULT_PORT;
	if(muiNumIOVars + muiNumROInts < 1){
		mstrLastError = "El driver necesita al menos una variable en su mapa";
		return false;
	}

	for(IOAddr addr : slaveVarsToUpdate){
		if(addr.uiChannel < 1 || addr.uiChannel > muiNumIOVars + muiNumROInts) continue;
		mModbusVars.insert({newAddr, new ModbusData()});
	}

	for(IOAddr addr : fieldVarsToUpdate){
		if(addr.uiChannel <= muiNumROInts || addr.uiChannel > muiNumIOVars + muiNumROInts) continue;
		mFieldVars.insert({newAddr, new FieldData()});
	}


	/*Inicializacion de la biblioteca*/
	mpMBCtx = modbus_new_tcp(mstrServerIp.c_str(), miPort);
	if(mpMBCtx == NULL){
		mstrLastError = std::string(modbus_strerror(errno));
		eraseVars();
		return false;
	}

	mfComErrorCallB = comErrorCallB;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = Stopped;
	uiComErrors = 0;
	mFieldIt = mFieldVars.begin();
	mModbusIt = mModbusVars.begin();
	mtWaitingStart.notify_one();
    return true;
}

bool ModbusMasterDrv::read(RegVar & var){
	auto it = mModbusVars.find(var.getAddr());
	if(it == mModbusVars.cend()){
		auto it2 = mFieldVars.find(var.getAddr());
		if(it2 == mFieldVars.cend()) return false;
		readFieldVar(var, it2);
	}
	else readModbusVar(var, it);
	return true;
}

void ModbusMasterDrv::readModbusVar(RegVar & var, std::unordered_map<IOAddr, ModbusData*>::const_iterator it){
	ModbusData const * tVarData = it->second;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	var.setCurrentVal(tVarData->mfCurrentVal);
	var.setTimeS(tVarData->miTimeS);
	var.setQState(((mtDrvState == COMError) ? ComError : tVarData->mtQState));
}
	
void ModbusMasterDrv::readFieldVar(RegVar & var, std::unordered_map<IOAddr, FieldData*>::const_iterator it){
	FieldData const * tVarData = it->second;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	var.setCurrentVal(tVarData->mfTrueVal);
	var.setForcedVal(tVarData->mfForcedVal);
	var.setForced(tVarData->mbForced);
	var.setTimeS(tVarData->miTimeS);
	var.setQState(((mtDrvState == COMError) ? ComError : tVarData->mtQState));
}

bool ModbusMasterDrv::write(RegVar const& var, std::function<void(IOAddr)>* writeSuccess, std::function<void(IOAddr)>* timeOut, std::uint32_t tWriteTimeOut){
	return write(var.getTrueVal(), var.getAddr(), writeSuccess, timeOut, tWriteTimeOut);
}


bool ModbusMasterDrv::force(RegVar const& var, std::function<void(IOAddr)>* writeSuccess, std::function<void(IOAddr)>* timeOut, std::uint32_t tWriteTimeOut){
	return force(var.getForcedVal(), var.getAddr(), var.getForced(), writeSuccess, timeOut, tWriteTimeOut);
}

bool ModbusMasterDrv::write(float const fVal, IOAddr const& tAddr, std::function<void(IOAddr)>* writeSuccess, std::function<void(IOAddr)>* timeOut, std::uint32_t tWriteTimeOut){
	std::uint8_t uiNumVar = tAddr.uiChannel;
	if(uiNumVar <= muiNumROInts || uiNumVar > muiNumIOVars + muiNumROInts) return false;
	WriteReq* writeReq = new WriteReq();
	writeReq->mtAddr = tAddr;
	writeReq->mbForce = false;
	writeReq->mfWriteVal = fVal;
	writeReq->mtWriteTimeOut = tWriteTimeOut;
	writeReq->mpfWriteSuccess = writeSuccess;
	writeReq->mpfWriteTimeOut = timeOut;
	writeReq->mTimeS = getMsSinceEpoch();
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	if(mtDrvState != Stopped && mtDrvState != UnInit){
		mPendingWrites.push(writeReq);
		return true;
	}
	delete writeReq;
	return false;
}

bool ModbusMasterDrv::force(float const fVal, IOAddr const& tAddr, bool bForceBitVal, std::function<void(IOAddr)>* writeSuccess, std::function<void(IOAddr)>* timeOut, std::uint32_t tWriteTimeOut){
	std::uint8_t uiNumVar = tAddr.uiChannel;
	if(uiNumVar <= muiNumROInts || uiNumVar > muiNumIOVars + muiNumROInts) return false;
	WriteReq* writeReq = new WriteReq();
	writeReq->mtAddr = var.getAddr();
	writeReq->mbForce = true;
	writeReq->mfWriteVal = fVal;
	writeReq->mbForcedVal = bForceBitVal;
	writeReq->mtWriteTimeOut = tWriteTimeOut;
	writeReq->mpfWriteSuccess = writeSuccess;
	writeReq->mpfWriteTimeOut = timeOut;
	writeReq->mTimeS = getMsSinceEpoch();
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	if(mtDrvState != Stopped && mtDrvState != UnInit){
		mPendingWrites.push(writeReq);
		return true;
	}
	delete writeReq;
	return false;
}

bool ModbusMasterDrv::read(std::uint32_t & uiVal, IOAddr tAddt){
	auto it = mModbusVars.find(var.getAddr());
	if(it == mModbusVars.cend()){
		return false;
	}
	ModbusData const * tVarData = it->second;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	uiVal = tVarData->muiCurrentVal;
	return true;
}

void ModbusMasterDrv::getChangedVars(std::unordered_set<IOAddr> & usChanges){
	unique_lock<mutex> mutexChagesSet(mtChagesSetMutex);
	usChanges = std::move(musChangedVars);
}


std::int64_t ModbusMasterDrv::getMsSinceEpoch(){
	using namespace std::chrono;
	return duration_cast<duration<std::int64_t,std::milli>>(system_clock::now().time_since_epoch()).count();
}


bool ModbusMasterDrv::loadXMLConfig(std::string const& strConfigPath){
	xmlParserCtxt* pParser; 
    xmlDoc* pDocTree; 

    
    pParser = xmlNewParserCtxt();
    if (pParser == NULL) {
       	mstrLastError = "No se pudo crear el parser xml.";
		return false;
    }

    bool bOk = true;
    pDocTree = xmlCtxtReadFile(pParser, strConfigPath.c_str(), NULL, 0);
    
    if (pDocTree == NULL) {
        mstrLastError = std::string("Fallo al parsear el archivo ") + strConfigPath + ".";
        bOk = false;
    } else {
		
        if (!pParser->valid){
        	mstrLastError = std::string("Archivo ") + strConfigPath + " con formato invalido.";
        	bOk = false;
        }
        else{
        	xmlNode* root = xmlDocGetRootElement(pDocTree);
        	bOk = readXMLConfig(pDocTree, root);
        }
	    
		
		xmlFreeDoc(pDocTree);
    }
   
    xmlFreeParserCtxt(pParser);

    xmlCleanupParser();
    return bOk;
}


bool ModbusMasterDrv::readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot){
	xmlChar *strAttr;
	if(pRoot && xmlStrcmp(pRoot->name, (const xmlChar *)"MasterDriver") == 0){
		strAttr = xmlGetProp(pRoot, (const xmlChar *)"IP");
		if(!strAttr) return false;
		mstrServerIp = std::string((const char*) strAttr);
		xmlFree(strAttr);
		strAttr = xmlGetProp(pRoot, (const xmlChar *)"Port");
		if(!strAttr) return false;
		miPort = atoi((const char*) strAttr);
		xmlFree(strAttr);
		strAttr = xmlGetProp(pRoot, (const xmlChar *)"NumROInts");
		if(!strAttr) return false;
		muiNumROInts = atoi((const char*) strAttr);
		xmlFree(strAttr);
		strAttr = xmlGetProp(pRoot, (const xmlChar *)"NumIOVars");
		if(!strAttr) return false;
		muiNumIOVars = atoi((const char*) strAttr);
		xmlFree(strAttr);
		return true;
	}
	return false; 
}


void ModbusMasterDrv::eraseVars(){
	for(auto & var : mFieldVars){
		delete var.second;
	}
    mFieldVars.clear();
    for(auto & var : mModbusVars){
		delete var.second;
	}
    mModbusVars.clear();
}
