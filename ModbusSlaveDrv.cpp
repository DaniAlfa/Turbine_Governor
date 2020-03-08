#include "ModbusSlaveDrv.h"

#include <chrono>
#include <cstdlib>
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
#define TRANSMISSION_TRYS_TIME 30

#define COM_ERROR_MIN_TIMEOUT 500
#define MAX_COM_ERROR_TRYS 3

#define MAX_NUM_MASTERS 5
#define WAITING_FOR_DATA_MILLIS 30


using namespace std;

ModbusSlaveDrv::ModbusSlaveDrv() : miServerSock(-1), mbDrvEnd(false), mtDrvState(UnInit) {
	mtDrvThread = new thread(&ModbusSlaveDrv::driverLoop, this);
}

ModbusSlaveDrv::~ModbusSlaveDrv(){
	mbDrvEnd = true;
	mtWaitingStart.notify_one();
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
	modbus_free(mpMBCtx);
    modbus_mapping_free(mpMBmapping);
    mFieldVars.clear();
    muiNumVars = 0;
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
					mtReadset = mtRefset;
					timeval tTimeOut;
					tTimeOut.tv_sec = 0;
					tTimeOut.tv_usec = 1000*WAITING_FOR_DATA_MILLIS;
					int iRes = select(miFdmax+1, &mtReadset, NULL, NULL, tTimeOut);
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
            	close(iSocket);
        	}
		}
		close(miServerSock);
		FD_ZERO(&mtRefset);
		muiNumConnections = 0;
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
                close(iMasterSock);
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


bool ModbusSlaveDrv::init(std::string const& strConfigPath){
	if(mtDrvState != UnInit){
		mstrLastError = "Driver ya inicializado";
		return false;
	}

	//Leer Configuracion
	mstrServerIp = "127.0.0.1";
	miPort = MODBUS_TCP_DEFAULT_PORT;
	muiNumVars = 50;

	/*
	if(!loadXMLConfig(strConfigPath)){
		mstrLastError = "Error en archivo de configuracion";
		return false;
	}
	*/

	/*Inicializacion de la biblioteca*/
	mpMBCtx = modbus_new_tcp(mstrServerIp.c_str(), miPort);
	if(mpMBCtx == NULL){
		mstrLastError = std::string(modbus_strerror(errno));
		return false;
	}

	mpMBmapping = modbus_mapping_new_start_address(COILADDRESS, muiNumVars* COILOFFSET, DIADDRESS, muiNumVars* DIOFFSET,
        HRADDRESS, muiNumVars * HROFFSET, IRADDRESS, muiNumVars * IROFFSET);

	if(mpMBmapping == NULL) {
        mstrLastError = std::string(modbus_strerror(errno));
        modbus_free(mpMBCtx);
        return false;
    }
    
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = Stopped;
	uiComErrors = 0;
	mtWaitingStart.notify_one();
    return true;
}


bool ModbusSlaveDrv::read(IOVar & var){
	std::uint8_t uiNumVar = var.getAddr().uiChannel;
	if(uiNumVar < 1 || uiNumVar > muiNumVars) return false;
	var.setCurrentVal(getCurrentVal(uiNumVar));

}

bool ModbusSlaveDrv::write(IOVar const& var){

}

bool ModbusSlaveDrv::readFieldVar(IOVar & var){

}

bool ModbusSlaveDrv::writeFieldVar(IOVar & var){

}

float ModbusSlaveDrv::getCurrentVal(std::uint8_t uiVar){

}


bool ModbusSlaveDrv::loadXMLConfig(std::string const& strConfigPath){
	xmlParserCtxt* pParser; /* the parser context */
    xmlDoc* pDocTree; /* the resulting document tree */

    /* create a parser context */
    pParser = xmlNewParserCtxt();
    if (pParser == NULL) {
       	mstrLastError = "No se pudo crear el parser xml.";
		return false;
    }
    /* parse the file, activating the DTD validation option */
    bool bOk = true;
    pDocTree = xmlCtxtReadFile(pParser, strConfigPath.c_str(), NULL, 0);
    /* check if parsing suceeded */
    if (pDocTree == NULL) {
        mstrLastError = std::string("Fallo al parsear el archivo ") + strConfigPath + ".";
        bOk = false;
    } else {
		/* check if validation suceeded */
        if (!pParser->valid){
        	mstrLastError = std::string("Archivo ") + strConfigPath + " con formato invalido.";
        	bOk = false;
        }
        else{
        	xmlNode* root = xmlDocGetRootElement(pDocTree);
        	bOk = readXMLConfig(pDocTree, root);
        }
	    
		/* free up the resulting document */
		xmlFreeDoc(pDocTree);
    }
    /* free up the parser context */
    xmlFreeParserCtxt(pParser);

    xmlCleanupParser();
    return bOk;
}

bool ModbusSlaveDrv::readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot){
	xmlChar *strAttr;
	if(pRoot && xmlStrcmp(pRoot->name, (const xmlChar *)"FieldDriver") == 0){
		strAttr = xmlGetProp(pRoot, (const xmlChar *)"ifname");
		mstrIfname = std::string((const char*) strAttr);
		xmlFree(strAttr);
		xmlNode* pSibling = pRoot->xmlChildrenNode;
		bool bOk = true;
		bool bSlaveCnf = false;
		bool bVars = false;
		while(pSibling && bOk && (!bSlaveCnf || !bVars)){
			if (!bSlaveCnf && xmlStrcmp(pSibling->name, (const xmlChar *)"SlaveConfig") == 0) {
		    	bSlaveCnf = true;
		    	bOk = readXMLSlaveCnf(pSibling);
 	    	}
 	    	else if(!bVars && xmlStrcmp(pSibling->name, (const xmlChar *)"Vars") == 0){
 	    		bVars = true;
 	    		bOk = readXMLVars(pDocTree, pSibling);
 	    	}
 	    	pSibling = pSibling->next;
		}
		return bOk;
	}
	return false; 
}

bool ModbusSlaveDrv::readXMLSlaveCnf(xmlNode* pNode){
	return true;
	
}

bool ModbusSlaveDrv::readXMLVars(xmlDoc* pDocTree, xmlNode* pNode){
	if(pNode && xmlStrcmp(pNode->name, (const xmlChar *)"Vars") == 0){
		xmlNode* pSibling = pNode->xmlChildrenNode;
		xmlNode* pVarData;
		bool bOk = true;
		DeviceFactory* devFactory = new DeviceFactory();
		while(pSibling && bOk){
			if (xmlStrcmp(pSibling->name, (const xmlChar *)"Var") == 0) {
		    	pVarData = pSibling->xmlChildrenNode;
		    	IOAddr newAddr;
		    	EtherDevice* dev = nullptr;
		    	bool bAddr = false;
		    	bool bVarType = false;
		    	while(pVarData && bOk && (!bAddr || !bVarType)){
		    		if (!bAddr && xmlStrcmp(pVarData->name, (const xmlChar *)"Addr") == 0) {
		    			bAddr = true;
		    			bOk = DeviceFactory::parseXMLAddr(pVarData, newAddr);
 	    			}
		    		else if (!bVarType && xmlStrcmp(pVarData->name, (const xmlChar *)"VarType") == 0) {
		    			bVarType = true;
		    			dev = devFactory->getNewXMLDevice(pDocTree, pVarData, *this);
		    			bOk = dev != nullptr;
 	    			}
 	    			pVarData = pVarData->next;
		    	}
		    	if(bOk && bAddr && bVarType){
		    		mDevices.insert({newAddr, dev});
		    	}
		    	else{
		    		if(dev != nullptr) delete dev;
		    		bOk = false;
		    	} 
 	    	}
 	    	pSibling = pSibling->next;
		}
		delete devFactory;
		if(!bOk){
			for(auto & var : mDevices){
				delete var.second;
			}
			mDevices.clear();
		}
		return bOk;
	}
	return false; 
}