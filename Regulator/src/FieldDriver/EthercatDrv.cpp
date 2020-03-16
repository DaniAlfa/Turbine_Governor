#include "EthercatDrv.h"
#include "EtherDevice.h"
#include "DeviceFactory.h"
#include <libsoem/ethercat.h>
#include <chrono>
#include <cstdlib>

#define RECIEVE_TIMEOUT 2000
#define POOLING_DELAY_MILLIS 20


using namespace std;

EthercatDrv::EthercatDrv() : mbDrvEnd(false), mtDrvState(UnInit) {
	for(int i = 0; i < IOMAP_DEFAULT_SIZE; ++i)
		mcIOmap[i] = 0;
	mtDrvThread = new thread(&EthercatDrv::driverLoop, this);
}

EthercatDrv::~EthercatDrv(){
	mbDrvEnd = true;
	mtWaitingStart.notify_one();
	mtDrvThread->join();
	delete mtDrvThread;
	eraseDrvConfig();
}

bool EthercatDrv::close(){
	if(mtDrvState == UnInit) return true;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = UnInit;
	eraseDrvConfig();
	return true;
}

void EthercatDrv::eraseDrvConfig(){
	for(auto & var : mDevices){
		delete var.second;
	}
	mDevices.clear();
	ec_close();
}

bool EthercatDrv::start(){
	if(mtDrvState == UnInit) return false;
	if(mtDrvState == Stopped){
		unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
		mtDrvState = Running;
		mtWaitingStart.notify_one();
	}
	return true;
}

bool EthercatDrv::stop(){
	if(mtDrvState == UnInit) return false;
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = Stopped;
	return true;
}

void EthercatDrv::driverLoop(){
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
				mutexDrvState.unlock();
				this_thread::sleep_for(chrono::milliseconds(POOLING_DELAY_MILLIS));
				mutexDrvState.lock();
				if(mtDrvState != Running) break;
				updateDevices();
				unique_lock<mutex> mutexIOMap(mtIOMapMutex);
				ec_send_processdata();
				ec_receive_processdata(RECIEVE_TIMEOUT);
				//recepcion y envio ok
				mtWaitingSend.notify_all();
			break;
		}
	}
}
	

bool EthercatDrv::read(IOVar & var){
	auto it = mDevices.find(var.getAddr());
	if(it == mDevices.end()) return false;
	return it->second->read(var);
}
	
bool EthercatDrv::write(IOVar const& var){
	auto it = mDevices.find(var.getAddr());
	if(it == mDevices.end()) return false;
	return it->second->write(var);
}

void EthercatDrv::updateDevices(){
	for(auto & var : mDevices){
		var.second->updateDevice(var.first);
	}
}

bool EthercatDrv::writeIO(IOAddr const& addr, std::uint32_t uiVal, bool sync, std::uint32_t tTimeOut){
	if(addr.uiModule < 1 || addr.uiModule > ec_slavecount) return false;
	if(addr.uiNumBits < 8 && addr.uiNumBits > 0){ // Escritura de bits en byte
		int iOffset = 7 - (addr.uiChannel % 8) - (addr.uiNumBits - 1); 
		uint8_t cleanMask = 0; // Mascara para limpiar los bit a escribir
		uint8_t maskVal; // Mascara con los bit a escribir
		std::uint8_t* pIOByte = ((std::uint8_t*) ec_slave[addr.uiModule].outputs) + (int) (addr.uiChannel / 8);

    	for(int i = 0; i < addr.uiNumBits; ++i)
    		cleanMask += (1 << i);
 
    	maskVal = ((uint8_t) uiVal) & cleanMask;
    	cleanMask = cleanMask << iOffset;
    	cleanMask = ~cleanMask; 
    	maskVal = maskVal << iOffset;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	*pIOByte = (*pIOByte & cleanMask) | maskVal;
    	if(sync){
    		if(mtWaitingSend.wait_for(mutexIOMap, std::chrono::duration<std::uint32_t,std::milli>(tTimeOut)) == std::cv_status::timeout) return false;
    	}
    }
    else if(addr.uiNumBits >= 8 && addr.uiNumBits <= 32 && addr.uiNumBits % 8 == 0 && addr.uiChannel % 8 == 0){ // Escritura de bytes
    	union {
        	uint32_t* p32;
        	uint16_t* p16;
        	uint8_t* p8;
    	} pIOByte, pValByte;
    	int iNumBytes = (int) addr.uiNumBits / 8;
    	pIOByte.p8 = ((std::uint8_t*) ec_slave[addr.uiModule].outputs) + (int) (addr.uiChannel / 8);
    	pValByte.p32 = &uiVal;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	if(iNumBytes == 1){ //Soem compila a little o big por lo que no es necesario invertir bytes
    		*pIOByte.p8 = *pValByte.p8;
    	}
    	else if(iNumBytes == 2){
    		*pIOByte.p16 = *pValByte.p16;
    	}
    	else if(iNumBytes == 4){
    		*pIOByte.p32 = *pValByte.p32;
    	}
    	if(sync){
    		if(mtWaitingSend.wait_for(mutexIOMap, std::chrono::duration<std::uint32_t,std::milli>(tTimeOut)) == std::cv_status::timeout) return false;
    	}
    }
    else return false;
    return true;
}

bool EthercatDrv::writeDeviceSync(IOAddr const& addr, std::uint32_t uiVal, std::uint32_t tTimeOut){
	return writeIO(addr, uiVal, true, tTimeOut);
}

bool EthercatDrv::writeDevice(IOAddr const& addr, std::uint32_t uiVal){
	return writeIO(addr, uiVal, false, 0);
}

bool EthercatDrv::readDevice(IOAddr const& addr, std::uint32_t & uiVal){
	uiVal = 0;
	if(addr.uiModule < 1 || addr.uiModule > ec_slavecount) return false;
	if(addr.uiNumBits < 8 && addr.uiNumBits > 0){ // Lectura de bits
		int iOffset = 7 - (addr.uiChannel % 8) - (addr.uiNumBits - 1); 
		uint8_t readMask = 0; // Mascara seleccionar los bits a leer
		uint8_t readedVal = 0; // Valor leido
		std::uint8_t* pIOByte = ((std::uint8_t*) ec_slave[addr.uiModule].inputs) + addr.uiChannel / 8;

    	for(int i = 0; i < addr.uiNumBits; ++i)
    		readMask += (1 << i);
 
    	readMask = readMask << iOffset;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	readedVal = ((*pIOByte & readMask) >> iOffset);
    	uiVal = readedVal;
    }
    else if(addr.uiNumBits >= 8 && addr.uiNumBits <= 32 && addr.uiNumBits % 8 == 0 && addr.uiChannel % 8 == 0){ // Lectura de bytes
    	union {
        	uint32_t* p32;
        	uint16_t* p16;
        	uint8_t* p8;
    	} pIOByte, pValByte;
    	int iNumBytes = (int) addr.uiNumBits / 8;
    	pIOByte.p8 = ((std::uint8_t*) ec_slave[addr.uiModule].inputs) + (addr.uiChannel / 8);
    	pValByte.p32 = &uiVal;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	if(iNumBytes == 1){ //Soem compila a little o big por lo que no es necesario invertir bytes
    		*pValByte.p8 = *pIOByte.p8;
    	}
    	else if(iNumBytes == 2){
    		*pValByte.p16 = *pIOByte.p16;
    	}
    	else if(iNumBytes == 4){
    		*pValByte.p32 = *pIOByte.p32;
    	}
    }
    else return false;
    return true;

}

bool EthercatDrv::init(std::string const& strConfigPath){
	if(mtDrvState != UnInit){
		mstrLastError = "Driver ya inicializado";
		return false;
	}

	if(!loadXMLConfig(strConfigPath)){
		mstrLastError = "Error en archivo de configuracion";
		return false;
	}

	/*Inicializacion de la biblioteca SOEM*/
	bool bOk = true;
	if(!ec_init(mstrIfname.c_str())){
		mstrLastError = "Interfaz de red invalida";
		bOk = false;
	}
	//cout << "Interfaz de red inicializada con exito." << endl;
	
	if(bOk && !ec_config_init(FALSE)){
		mstrLastError =  "No se pudieron configurar los esclavos, ec_config_init";
		bOk = false;
	}

	//Configuracion de esclavos, escritura de funcion de configuracion en array de esclavos

	if(bOk){
		//Configuracion de mapa de memoria
		//int usedmem = ec_config_map(&mcIOmap);
		ec_config_map(&mcIOmap);
		//Configuracion de relojes distribuidos
		ec_configdc();
		/*
		cout << "Memoria mapeada " << usedmem << " bytes." << endl;
		cout << ec_slavecount << " esclavos configurados." << endl;
		//printf("%p\n", IOmap);
		for(int i = 1; i < ec_slavecount + 1; ++i){
			cout << "Esclavo " << i << ": " << ec_slave[i].name << ", ";
			cout << "Bits entrada: " << ec_slave[i].Ibits << ", ";
			//if(ec_slave[i].Ibits > 0) printf("Direccion de comienzo %p ", ec_slave[i].inputs);
			cout << "Bits salida: " << ec_slave[i].Obits << endl;
			//if(ec_slave[i].Obits > 0) printf("Direccion de comienzo %p\n", ec_slave[i].outputs);
		}
		
		cout << "Esclavos mapeados, cambio a modo seguro operacional." << endl;
	    */

	    //Cambio a modo seguro operacional 
	    if(EC_STATE_SAFE_OP != ec_statecheck(0, EC_STATE_SAFE_OP,  8000000)){
	    	mstrLastError = "Los esclavos no pudieron cambiar a modo seguro operacional.";
	    	bOk = false;
	    }
	    else{
	    	//Cambio a modo operacional
			ec_slave[0].state = EC_STATE_OPERATIONAL;
			 //Se envia una trama para cambiar de modo seguro a operacional
			ec_send_processdata();
			ec_receive_processdata(2000);
			//Se solicita el cambio a todos los esclavos a OP
		    ec_writestate(0);
		    int trys = 200;
		    //Se espera que lleguen todos los esclavos a modo operacional
		    do
		    {
		        ec_send_processdata();
		        ec_receive_processdata(EC_TIMEOUTRET);
		        ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
		    }
		    while (trys-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));
		    if(ec_slave[0].state != EC_STATE_OPERATIONAL){
		    	mstrLastError = "Los esclavos no pudieron cambiar a modo operacional.";
		    	bOk = false;
		    }
	    }	
	}
	if(!bOk){
		eraseDrvConfig();
		return false;
	}
    //cout << "Esclavos inicializados con exito a modo operacional." << endl;
    
    /*Carga de archivo e inicializacion de dispositivos*/
    //En fallo ec_close();
    
	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = Stopped;
	mtWaitingStart.notify_one();
    return true;
}

bool EthercatDrv::loadXMLConfig(std::string const& strConfigPath){
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
    pDocTree = xmlCtxtReadFile(pParser, strConfigPath.c_str(), NULL, XML_PARSE_DTDVALID);
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

bool EthercatDrv::readXMLConfig(xmlDoc* pDocTree, xmlNode* pRoot){
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

bool EthercatDrv::readXMLSlaveCnf(xmlNode* pNode){
	return true;
	
}

bool EthercatDrv::readXMLVars(xmlDoc* pDocTree, xmlNode* pNode){
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