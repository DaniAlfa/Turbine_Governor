#include "ModbusSlaveDrv.h"

#include <chrono>
#include <cstdlib>
#include <errno.h>

//Especificos de linux
#include <unistd.h>

#define DIOFFSET 2 //Numero de DI en IOVar
#define DIADDRESS 10001
#define COILOFFSET 1
#define COILADDRESS 1
#define IRADDRESS 30001
#define IROFFSET 4
#define HRADDRESS 40001
#define HROFFSET 4

#define MAX_TRANSMISSION_TRYS 3
#define TRANSMISSION_TRYS_TIME 30

#define COM_ERROR_MIN_TIMEOUT 500
#define MAX_COM_ERROR_TRYS 3

#define MAX_NUM_MASTERS 5
#define WAITING_FOR_DATA_MILLIS 30

#define POOLING_DELAY_MILLIS 10

using namespace std;

ModbusSlaveDrv::ModbusSlaveDrv() : mpMBCtx(NULL), mpMBmapping(NULL), miServerSock(-1), mbDrvEnd(false), mtDrvState(UnInit) {
	std::uint32_t iNum = 1;
    std::uint8_t *pNum = (std::uint8_t*)&iNum;
    mbArchLittleEnd = (*pNum == 1);
	mtDrvThread = new thread(&ModbusSlaveDrv::driverLoop, this);
}

ModbusSlaveDrv::~ModbusSlaveDrv(){
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mbDrvEnd = true;
	mtWaitingStart.notify_one();
	mutexDrvState.unlock();
	mtDrvThread->join();
	delete mtDrvThread;
	closeSockets();
	eraseDrvConfig();
}

bool ModbusSlaveDrv::close(){
	if(mtDrvState == UnInit) return true;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = UnInit;
	closeSockets();
	eraseDrvConfig();
	return true;
}

void ModbusSlaveDrv::eraseDrvConfig(){
	if(mpMBCtx != NULL) modbus_free(mpMBCtx);
	mpMBCtx = NULL;
    if(mpMBmapping != NULL) modbus_mapping_free(mpMBmapping);
    mpMBmapping = NULL;
    mFieldVars.clear();
    muiNumIOVars = 0;
    muiNumROInts = 0;
    uiComErrors = 0;
}

bool ModbusSlaveDrv::start(){
	if(mtDrvState == UnInit) return false;
	if(mtDrvState == Stopped){
		unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
		mtDrvState = Running;
		mtWaitingStart.notify_one();
	}
	return true;
}

bool ModbusSlaveDrv::stop(){
	if(mtDrvState == UnInit) return false;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	closeSockets();
	mtDrvState = Stopped;
	uiComErrors = 0;
	return true;
}

void ModbusSlaveDrv::driverLoop(){
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
				if(miServerSock == -1){
					if(!createServerSocket()){
						mtDrvState = COMError;
					}
				}else{
					//this_thread::sleep_for(chrono::milliseconds(POOLING_DELAY_MILLIS));
					mtReadset = mtRefset;
					timeval tTimeOut;
					tTimeOut.tv_sec = 0;
					tTimeOut.tv_usec = 1000*WAITING_FOR_DATA_MILLIS;
					int iRes = select(miFdmax+1, &mtReadset, NULL, NULL, &tTimeOut);
        			if (iRes == -1) {
            			mstrLastError = "Error en espera de comunicaciones";
						mtDrvState = COMError;
        			}
        			else if(iRes > 0){
        				processPendingRequests();
        			}
				}	
			break;
			case COMError:
				if(miServerSock == -1){
					if(createServerSocket()){
						mtDrvState = Running;
					}
					else{
						if(uiComErrors == 0) mfComErrorCallB();
						if(++uiComErrors > MAX_COM_ERROR_TRYS) uiComErrors = MAX_COM_ERROR_TRYS;
						mutexDrvState.unlock();
						this_thread::sleep_for(chrono::milliseconds(COM_ERROR_MIN_TIMEOUT * uiComErrors));
						mutexDrvState.lock();
					}
				}
				else{
					closeSockets();
				}
			break;
			case VarError:
			default:
				mtDrvState = COMError;
		}
	}
}

void ModbusSlaveDrv::closeSockets(){
	if(miServerSock != -1){
		for(int iSocket = 0; iSocket <= miFdmax; ++iSocket){
			if (FD_ISSET(iSocket, &mtRefset) && iSocket != miServerSock) {
            	FD_CLR(iSocket, &mtRefset);
            	modbus_set_socket(mpMBCtx, iSocket);
            	modbus_close(mpMBCtx);
        	}
		}
		modbus_set_socket(mpMBCtx, miServerSock);
		modbus_close(mpMBCtx);
		FD_ZERO(&mtRefset);
		muiNumConnections = 0;
		miServerSock = -1;
	}
}

void ModbusSlaveDrv::processPendingRequests(){
	for (int iMasterSock = 0; iMasterSock <= miFdmax; ++iMasterSock) {
        if (!FD_ISSET(iMasterSock, &mtReadset)) {
            continue;
        }
        if (iMasterSock == miServerSock) {
            if(muiNumConnections < MAX_NUM_MASTERS){
            	int iNewSocket = modbus_tcp_accept(mpMBCtx, &miServerSock);
            	if(iNewSocket != -1){
            		FD_SET(iNewSocket, &mtRefset);
            		++muiNumConnections;
                    if (iNewSocket > miFdmax) {
                        miFdmax = iNewSocket;
                   	}
            	}
            }
        } else {
            modbus_set_socket(mpMBCtx, iMasterSock);
            int iRequest = modbus_receive(mpMBCtx, muiQuery);
            int iTransTrys = 0;
            if (iRequest > 0) {
                unique_lock<mutex> mutexIOMap(mtIOMapMutex);
                while(modbus_reply(mpMBCtx, muiQuery, iRequest, mpMBmapping) == -1 && iTransTrys++ < MAX_TRANSMISSION_TRYS){
                	mutexIOMap.unlock();
                	this_thread::sleep_for(chrono::milliseconds(TRANSMISSION_TRYS_TIME));
                	mutexIOMap.lock();
                }
            }
            if (iRequest == -1 || iTransTrys >= MAX_TRANSMISSION_TRYS) {
                ::close(iMasterSock);
                FD_CLR(iMasterSock, &mtRefset);
                --muiNumConnections;
                if (iMasterSock == miFdmax) {
                    miFdmax--;
                }
            }
        }
    }
}

bool ModbusSlaveDrv::createServerSocket(){
	if(miServerSock == -1){
		miServerSock = modbus_tcp_listen(mpMBCtx, MAX_NUM_MASTERS);
		if(miServerSock == -1){
			mstrLastError = std::string(modbus_strerror(errno));
			return false;
		}
		FD_ZERO(&mtRefset);
		FD_SET(miServerSock, &mtRefset);
		miFdmax = miServerSock;
		muiNumConnections = 0;
		uiComErrors = 0;
	}
	return true;
}


bool ModbusSlaveDrv::init(std::string const& strConfigPath, std::function<void()> const& comErrorCallB){
	if(mtDrvState != UnInit){
		mstrLastError = "Driver ya inicializado";
		return false;
	}

	if(!loadXMLConfig(strConfigPath)){
		mstrLastError = "Error en archivo de configuracion";
		return false;
	}
	
	if(miPort == -1) miPort = MODBUS_TCP_DEFAULT_PORT;
	if(muiNumIOVars < 1 && muiNumROInts < 1){
		mstrLastError = "El driver necesita al menos una variable en su mapa";
		return false;
	}

	/*Inicializacion de la biblioteca*/
	mpMBCtx = modbus_new_tcp(mstrServerIp.c_str(), miPort);
	if(mpMBCtx == NULL){
		mstrLastError = std::string(modbus_strerror(errno));
		mFieldVars.clear();
		return false;
	}

	mpMBmapping = modbus_mapping_new_start_address(COILADDRESS, muiNumIOVars* COILOFFSET, DIADDRESS, muiNumIOVars* DIOFFSET,
        HRADDRESS, muiNumIOVars * HROFFSET, IRADDRESS, muiNumIOVars * IROFFSET + muiNumROInts * 2);

	if(mpMBmapping == NULL) {
        mstrLastError = std::string(modbus_strerror(errno));
        mFieldVars.clear();
        modbus_free(mpMBCtx);
        mpMBCtx = NULL;
        return false;
    }

    initMapping();
    
    mfComErrorCallB = comErrorCallB;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = Stopped;
	uiComErrors = 0;
	mtWaitingStart.notify_one();
    return true;
}

void ModbusSlaveDrv::initMapping(){
	std::uint8_t uiNumVar;
	for(auto & var : mFieldVars){
		uiNumVar = var.second.uiChannel;
		setForced(uiNumVar, false);
	}
}


bool ModbusSlaveDrv::read(IOVar & var){
	std::uint8_t uiNumVar = var.getAddr().uiChannel - muiNumROInts;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	if(uiNumVar < 1 || uiNumVar > muiNumIOVars || mtDrvState == UnInit) return false;
	var.setCurrentVal(getCurrentVal(uiNumVar));
	if(mtDrvState == COMError) var.setQState(QuState::ComError);
	else var.setQState(QuState::OK);
	mutexIOMap.unlock();
	var.setTimeS(getMsSinceEpoch());
	return true;
}

bool ModbusSlaveDrv::write(IOVar const& var){
	std::uint8_t uiNumVar = var.getAddr().uiChannel - muiNumROInts;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	if(uiNumVar < 1 || uiNumVar > muiNumIOVars || mtDrvState == UnInit) return false;
	setCurrentVal(var.getTrueVal(), uiNumVar);
	return true;
}

bool ModbusSlaveDrv::updateFieldVar(IOVar & var){
	auto it = mFieldVars.find(var.getID());
	if(it == mFieldVars.end()) return false;
	std::uint8_t uiNumVar = it->second.uiChannel-muiNumROInts;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	if(uiNumVar < 1 || uiNumVar > muiNumIOVars || mtDrvState == UnInit) return false;
	var.setForcedVal(getForcedVal(uiNumVar));
	var.setForced(getForced(uiNumVar));
	setCurrentVal(var.getTrueVal(), uiNumVar);
	setQState(var.getQState(), uiNumVar);
	setTimeS(var.getTimeS(), uiNumVar);
	return true;
}

bool ModbusSlaveDrv::writeRO(std::uint32_t iVal, IOAddr addr){
	std::uint8_t uiNumVar = addr.uiChannel;
	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
	if(uiNumVar < 1 || uiNumVar > muiNumROInts || mtDrvState == UnInit) return false;
	std::uint8_t* pMap = (std::uint8_t*) (mpMBmapping->tab_input_registers + ((uiNumVar - 1) * 2));
	std::uint8_t* pVal = (std::uint8_t*) &iVal;
	if(mbArchLittleEnd) pVal = pVal + 3;
	for(int i = 0; i < 4; ++i){
		*pMap = *pVal;
		if(mbArchLittleEnd) --pVal;
		else ++pVal;
		++pMap;
	}
	return true;
}

void ModbusSlaveDrv::setQState(QuState tState, std::uint8_t uiVar){
	std::uint8_t* pDi = (mpMBmapping->tab_input_bits + ((uiVar * DIOFFSET) - DIOFFSET));
	std::uint8_t uiVal = tState;
	std::uint8_t uiOffset = DIOFFSET - 1;
	std::uint8_t uiMask = 1 << uiOffset;
	for(int i = 0; i < DIOFFSET; ++i, --uiOffset, ++pDi){
		*pDi = ((uiVal & uiMask) >> uiOffset);
		uiMask = uiMask >> 1;
	}
}

void ModbusSlaveDrv::setTimeS(std::int64_t const& iTimeS, std::uint8_t uiVar){
	std::uint8_t* pMap = (std::uint8_t*) (mpMBmapping->tab_input_registers + ((uiVar * IROFFSET) - IROFFSET) + muiNumROInts*2);
	std::uint8_t* pVal = (std::uint8_t*) &iTimeS;
	if(mbArchLittleEnd) pVal = pVal + 7;
	for(int i = 0; i < 8; ++i){
		*pMap = *pVal;
		if(mbArchLittleEnd) --pVal;
		else ++pVal;
		++pMap;
	}
}


bool ModbusSlaveDrv::getForced(std::uint8_t uiVar){
	std::uint16_t valAddr = (uiVar * COILOFFSET) - COILOFFSET;
	return (*(mpMBmapping->tab_bits + valAddr) >= 1);
}

void ModbusSlaveDrv::setForced(std::uint8_t uiVar, bool bState){
	std::uint16_t valAddr = (uiVar * COILOFFSET) - COILOFFSET;
	if(bState) *(mpMBmapping->tab_bits + valAddr) = 1;
	else *(mpMBmapping->tab_bits + valAddr) = 0;
}

float ModbusSlaveDrv::getCurrentVal(std::uint8_t uiVar){
	std::uint16_t valAddr = (uiVar * HROFFSET) - HROFFSET;
	return modbus_get_float_abcd(mpMBmapping->tab_registers + valAddr);
}

float ModbusSlaveDrv::getForcedVal(std::uint8_t uiVar){
	std::uint16_t valAddr = (uiVar * HROFFSET) - HROFFSET + 2;
	return modbus_get_float_abcd(mpMBmapping->tab_registers + valAddr);
}

void ModbusSlaveDrv::setCurrentVal(float fVal, std::uint8_t uiVar){
	std::uint16_t valAddr = (uiVar * HROFFSET) - HROFFSET;
	modbus_set_float_abcd(fVal, mpMBmapping->tab_registers + valAddr);
}

std::int64_t ModbusSlaveDrv::getMsSinceEpoch(){
	using namespace std::chrono;
	return duration_cast<duration<std::int64_t,std::milli>>(system_clock::now().time_since_epoch()).count();
}

bool ModbusSlaveDrv::loadXMLConfig(std::string const& strConfigPath){
	xmlParserCtxt* pParser; 
    xmlDoc* pDocTree; 

    
    pParser = xmlNewParserCtxt();
    if (pParser == NULL) {
       	mstrLastError = "No se pudo crear el parser xml.";
		return false;
    }

    bool bOk = true;
    pDocTree = xmlCtxtReadFile(pParser, strConfigPath.c_str(), NULL, XML_PARSE_DTDVALID);
    
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


bool ModbusSlaveDrv::readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot){
	xmlChar *strAttr;
	if(pRoot && xmlStrcmp(pRoot->name, (const xmlChar *)"SlaveDriver") == 0){
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
		xmlNode* pSibling = pRoot->xmlChildrenNode;
		bool bOk = true;
		while(pSibling && bOk){
 	    	if(xmlStrcmp(pSibling->name, (const xmlChar *)"FieldVars") == 0){
 	    		bOk = readXMLVars(pDocTree, pSibling);
 	    	}
 	    	pSibling = pSibling->next;
		}
		return bOk;
	}
	return false; 
}



bool ModbusSlaveDrv::readXMLVars(xmlDoc* pDocTree, xmlNode* pNode){
	if(pNode && xmlStrcmp(pNode->name, (const xmlChar *)"FieldVars") == 0){
		xmlNode* pSibling = pNode->xmlChildrenNode;
		xmlNode* pVarData;
		bool bOk = true;
		while(pSibling && bOk){
			if (xmlStrcmp(pSibling->name, (const xmlChar *)"FieldVar") == 0) {
				bool bAddr = false;
		    	pVarData = pSibling->xmlChildrenNode;
		    	IOAddr newAddr;
		  		xmlChar* strAttr = xmlGetProp(pSibling, (const xmlChar *)"ID");
				if(!strAttr) return false;
				std::uint32_t uiVarID = atoi((const char*) strAttr);
				xmlFree(strAttr);
		    	while(pVarData && bOk){
		    		if (xmlStrcmp(pVarData->name, (const xmlChar *)"Addr") == 0) {
		    			bAddr = true;
		    			bOk = parseXMLAddr(pVarData, newAddr);
 	    			}
 	    			pVarData = pVarData->next;
		    	}
		    	if(bOk && bAddr){
		    		mFieldVars.insert({uiVarID, newAddr});
		    	}
		    	else bOk = false; 
 	    	}
 	    	pSibling = pSibling->next;
		}
		if(!bOk){
			mFieldVars.clear();
		}
		return bOk;
	}
	return false; 
}

bool ModbusSlaveDrv::parseXMLAddr(xmlNode* pNode, IOAddr & addr){
	return parseXML8bitIntAttr(pNode, "Head", addr.uiHeader) && parseXML8bitIntAttr(pNode, "Mod", addr.uiModule) && parseXML8bitIntAttr(pNode, "Ch", addr.uiChannel) && 
	parseXML8bitIntAttr(pNode, "Prec", addr.uiNumBits);
}

bool ModbusSlaveDrv::parseXML8bitIntAttr(xmlNode* pNode, const char* attr, std::uint8_t & iVal){
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


