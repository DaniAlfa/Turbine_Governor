#include "ModbusMasterDrv.h"

#include <chrono>
#include <cstdlib>
#include <errno.h>
#include <iostream>

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
    muiNumVars = 0;
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
	int uiNumVar = tAddr.uiChannel;
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
			bTS = modbus_read_input_registers(mpMBCtx, (IRADDRESS + ((uiNumVar * IROFFSET) - IROFFSET)), 4, uiTS) == 4;
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
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	pFieldVar->mfTrueVal = modbus_get_float_abcd(uiVals);
	pFieldVar->mfForcedVal = modbus_get_float_abcd(uiVals + 2);
	getTimeS(pFieldVar->miTimeS, uiTS);
	pFieldVar->mtQState = getQState(uiQstate);
	pFieldVar->mbForced = (uiForced >= 1);
	return true;
}

bool ModbusMasterDrv::processRead(IOAddr tAddr, ModbusData* pMBVar){
	int varAddr = HRADDRESS + ((tAddr.uiChannel * HROFFSET) - HROFFSET);
	std::uint16_t uiCurrentVal[2];
	unsigned iNumTrys = 0; 
	while(iNumTrys++ < MAX_TRANSMISSION_TRYS && modbus_read_registers(mpMBCtx, varAddr, 2, uiCurrentVal) == -1) 
		this_thread::sleep_for(chrono::milliseconds(TRANSMISSION_TRYS_TIME));
	if(iNumTrys >= MAX_TRANSMISSION_TRYS + 1){
		mstrLastError = std::string(modbus_strerror(errno));
		return false;
	}
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	pMBVar->mfCurrentVal = modbus_get_float_abcd(uiCurrentVal);
	pMBVar->miTimeS = getMsSinceEpoch();
	pMBVar->mtQState = OK;
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
	std::uint16_t uiAddr = HRADDRESS + ((writeReq->mtAddr.uiChannel * HROFFSET) - HROFFSET);
	if(writeReq->mbForce) uiAddr += 2;
	std::uint16_t uiWriteVal[2];
	modbus_set_float_abcd(writeReq->mfWriteVal, uiWriteVal);
	unsigned iNumTrys = 0;
	bool bForced = !writeReq->mbForce;
	bool bWriteVal = writeReq->mbForce && !writeReq->mbForcedVal;
	while(iNumTrys++ < MAX_TRANSMISSION_TRYS && (!bForced || !bWriteVal)){
		if(!bForced){
			bForced = modbus_write_bit(mpMBCtx, (COILADDRESS + ((writeReq->mtAddr.uiChannel * COILOFFSET) - COILOFFSET)), writeReq->mbForcedVal) == 1;
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
			mfWriteTimeOutCallB(writeReq->mtAddr);
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


bool ModbusMasterDrv::init(std::string const& strConfigPath, std::function<void(IOAddr)> const& writeTimeOutCallB, std::function<void()> const& comErrorCallB){
	if(mtDrvState != UnInit){
		mstrLastError = "Driver ya inicializado";
		return false;
	}

	if(!loadXMLConfig(strConfigPath)){
		return false;
	}
	
	if(miPort == -1) miPort = MODBUS_TCP_DEFAULT_PORT;
	if(muiNumVars < 1){
		mstrLastError = "El driver necesita al menos una variable en su mapa";
		return false;
	}

	/*Inicializacion de la biblioteca*/
	mpMBCtx = modbus_new_tcp(mstrServerIp.c_str(), miPort);
	if(mpMBCtx == NULL){
		mstrLastError = std::string(modbus_strerror(errno));
		eraseVars();
		return false;
	}

    
    mfComErrorCallB = comErrorCallB;
    mfWriteTimeOutCallB = writeTimeOutCallB;
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
	var.setQState(tVarData->mtQState);
}

bool ModbusMasterDrv::write(RegVar const& var, std::uint32_t tWriteTimeOut){
	std::uint8_t uiNumVar = var.getAddr().uiChannel;
	if(uiNumVar < 1 || uiNumVar > muiNumVars) return false;
	WriteReq* writeReq = new WriteReq();
	writeReq->mtAddr = var.getAddr();
	writeReq->mbForce = false;
	writeReq->mfWriteVal = var.getTrueVal();
	writeReq->mtWriteTimeOut = tWriteTimeOut;
	writeReq->mTimeS = getMsSinceEpoch();
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	if(mtDrvState != Stopped && mtDrvState != UnInit){
		mPendingWrites.push(writeReq);
		return true;
	}
	delete writeReq;
	return false;
}


bool ModbusMasterDrv::force(RegVar const& var, std::uint32_t tWriteTimeOut){
	std::uint8_t uiNumVar = var.getAddr().uiChannel;
	if(uiNumVar < 1 || uiNumVar > muiNumVars) return false;
	WriteReq* writeReq = new WriteReq();
	writeReq->mtAddr = var.getAddr();
	writeReq->mbForce = true;
	writeReq->mfWriteVal = var.getForcedVal();
	writeReq->mbForcedVal = var.getForced();
	writeReq->mtWriteTimeOut = tWriteTimeOut;
	writeReq->mTimeS = getMsSinceEpoch();
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	if(mtDrvState != Stopped && mtDrvState != UnInit){
		mPendingWrites.push(writeReq);
		return true;
	}
	delete writeReq;
	return false;
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
		strAttr = xmlGetProp(pRoot, (const xmlChar *)"NumVars");
		if(!strAttr) return false;
		muiNumVars = atoi((const char*) strAttr);
		xmlFree(strAttr);
		xmlNode* pSibling = pRoot->xmlChildrenNode;
		bool bOk = true;
		while(pSibling && bOk){
 	    	if(xmlStrcmp(pSibling->name, (const xmlChar *)"FieldUpdates") == 0){
 	    		bOk = readXMLVars(pDocTree, pSibling, true);
 	    	}
 	    	else if(xmlStrcmp(pSibling->name, (const xmlChar *)"SlaveUpdates") == 0){
 	    		bOk = readXMLVars(pDocTree, pSibling, false);
 	    	}
 	    	pSibling = pSibling->next;
		}
		return bOk;
	}
	return false; 
}



bool ModbusMasterDrv::readXMLVars(xmlDoc* pDocTree, xmlNode* pNode, bool bField){
	if(pNode && xmlStrcmp(pNode->name, (const xmlChar *)((bField) ? "FieldUpdates" : "SlaveUpdates")) == 0){
		xmlNode* pSibling = pNode->xmlChildrenNode;
		bool bOk = true;
		IOAddr newAddr;
		while(pSibling && bOk){
			if (xmlStrcmp(pSibling->name, (const xmlChar *)"Addr") == 0) {
		    	bOk = parseXMLAddr(pSibling, newAddr);
		    	if(bOk){
		    		if(bField){
		    			mFieldVars.insert({newAddr, new FieldData()});
		    		}
		    		else mModbusVars.insert({newAddr, new ModbusData()});
		    	}
 	    	}
 	    	pSibling = pSibling->next;
		}
		if(!bOk){
			eraseVars();
		}
		return bOk;
	}
	return false; 
}

bool ModbusMasterDrv::parseXMLAddr(xmlNode* pNode, IOAddr & addr){
	return parseXML8bitIntAttr(pNode, "Head", addr.uiHeader) && parseXML8bitIntAttr(pNode, "Mod", addr.uiModule) && parseXML8bitIntAttr(pNode, "Ch", addr.uiChannel) && 
	parseXML8bitIntAttr(pNode, "Prec", addr.uiNumBits);
}

bool ModbusMasterDrv::parseXML8bitIntAttr(xmlNode* pNode, const char* attr, std::uint8_t & iVal){
	if(!pNode) return false;
	xmlChar *strAttr;
	strAttr = xmlGetProp(pNode, (const xmlChar *) attr);
	if(strAttr){
		iVal = atoi((const char*) strAttr);
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
