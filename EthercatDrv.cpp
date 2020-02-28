#include "EthercatDrv.h"
#include "EtherDevice.h"
#include <ethercat.h>
#include <cmath>
#include <chrono>


#define RECIEVE_TIMEOUT 2000

#include "DigitalDev.h"
#include <iostream>


using namespace std;

EthercatDrv::EthercatDrv() : mbDrvEnd(false), mtDrvState(UnInit) {
	
	mtDrvThread = new thread(&EthercatDrv::driverLoop, this);
}

EthercatDrv::~EthercatDrv(){
	mbDrvEnd = true;
	mtWaitingStart.notify_one();
	mtDrvThread->join();
	delete mtDrvThread;
	eraseDrvConfig();
}

bool EthercatDrv::close(){//////////
	if(mtDrvState == Stopped){
		unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
		mtDrvState = UnInit;
		eraseDrvConfig();
		mtWaitingStart.notify_one();
		mutexDrvState.unlock();
		return true;
	}
	return false;
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
				this_thread::sleep_for(chrono::milliseconds(5));
				mutexDrvState.lock();
				updateDevices();
				unique_lock<mutex> mutexIOMap(mtIOMapMutex);
				ec_send_processdata();
				ec_receive_processdata(RECIEVE_TIMEOUT);
			break;
		}
	}
}


bool EthercatDrv::init(std::string const& strIfname, std::string & strError){
	if(mtDrvState != UnInit){
		strError = "Driver ya inicializado";
		return false;
	}
	/*Inicializacion de la biblioteca SOEM*/

	if(!ec_init(strIfname.c_str())){
		strError = "Interfaz de red invalida";
		return false;
	}
	//cout << "Interfaz de red inicializada con exito." << endl;
	
	if(!ec_config_init(FALSE)){
		strError =  "No se pudieron configurar los esclavos, ec_config_init";
		return false;
	}

	//Configuracion de esclavos, escritura de funcion de configuracion en array de esclavos


	//Configuracion de mapa de memoria
	int usedmem = ec_config_map(&mcIOmap);
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
    	strError = "Los esclavos no pudieron cambiar a modo seguro operacional.";
    	return false;
    }

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
    	strError = "Los esclavos no pudieron cambiar a modo operacional.";
    	return false;
    }

    //cout << "Esclavos inicializados con exito a modo operacional." << endl;
    
    /*Carga de archivo e inicializacion de dispositivos*/
    //En fallo ec_close();

    IOAddr addr1;
    addr1.uiHeader = 0;
	addr1.uiModule = 2;
	addr1.uiChannel = 0;
	addr1.uiNumBits = 1;

	EtherDevice* dev1 = new DigitalDev(*this, 1);
	mDevices.insert({addr1, dev1});

	IOAddr addr2;
    addr2.uiHeader = 0;
	addr2.uiModule = 3;
	addr2.uiChannel = 7;
	addr2.uiNumBits = 1;

	EtherDevice* dev2 = new DigitalDev(*this);
	mDevices.insert({addr2, dev2});

	unique_lock<mutex> mutexDrvState(mtDrvStateMutex);
	mtDrvState = Stopped;
	mtWaitingStart.notify_one();
    return true;
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



bool EthercatDrv::writeIOmap(IOAddr const& addr, std::int32_t val){
	std::uint8_t* pIOByte;
	if(addr.uiModule < 1 || addr.uiModule > ec_slavecount) return false;
	if(addr.uiNumBits < 8 && addr.uiNumBits > 0){ // Escritura de bits en byte
		int iOffset = 7 - (addr.uiChannel % 8) - (addr.uiNumBits - 1); 
		uint8_t cleanMask = 0; // Mascara para limpiar los bit a escribir
		uint8_t maskVal; // Mascara con los bit a escribir
		pIOByte = ((std::uint8_t*) ec_slave[addr.uiModule].outputs) + addr.uiChannel / 8;

    	for(int i = 0; i < addr.uiNumBits; ++i)
    		cleanMask += pow(2, i);
 
    	maskVal = ((uint8_t) val) & cleanMask;
    	cleanMask = cleanMask << iOffset;
    	cleanMask = ~cleanMask; 
    	maskVal = maskVal << iOffset;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	*pIOByte = (*pIOByte & cleanMask) | maskVal;
    }
    else if(addr.uiNumBits >= 8 && addr.uiNumBits <= 32 && addr.uiNumBits % 8 == 0 && addr.uiChannel % 8 == 0){ // Escritura de bytes
    	int iNumBytes = (int) addr.uiNumBits / 8;
    	pIOByte = ((std::uint8_t*) ec_slave[addr.uiModule].outputs) + addr.uiChannel / 8;
    	std::uint8_t* pValByte = (std::uint8_t*) &val;
    	pValByte += 4 - iNumBytes;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	for(;iNumBytes > 0; --iNumBytes, ++pIOByte, ++pValByte)
    		*pIOByte = *pValByte;
    }
    else return false;
    return true;
}

bool EthercatDrv::readIOmap(IOAddr const& addr, std::int32_t & val){
	std::uint8_t* pIOByte;
	if(addr.uiModule < 1 || addr.uiModule > ec_slavecount) return false;
	if(addr.uiNumBits < 8 && addr.uiNumBits > 0){ // Lectura de bits
		int iOffset = 7 - (addr.uiChannel % 8) - (addr.uiNumBits - 1); 
		uint8_t readMask = 0; // Mascara seleccionar los bits a leer
		uint8_t readedVal = 0; // Valor leido
		pIOByte = ((std::uint8_t*) ec_slave[addr.uiModule].inputs) + addr.uiChannel / 8;

    	for(int i = 0; i < addr.uiNumBits; ++i)
    		readMask += pow(2, i);
 
    	readMask = readMask << iOffset;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	readedVal = ((*pIOByte & readMask) >> iOffset);
    	val = readedVal;
    }
    else if(addr.uiNumBits >= 8 && addr.uiNumBits <= 32 && addr.uiNumBits % 8 == 0 && addr.uiChannel % 8 == 0){ // Lectura de bytes
    	int iNumBytes = (int) addr.uiNumBits / 8;
    	pIOByte = ((std::uint8_t*) ec_slave[addr.uiModule].inputs) + addr.uiChannel / 8;
    	std::uint8_t* pValByte = (std::uint8_t*) &val;
    	pValByte += 4 - iNumBytes;
    	unique_lock<mutex> mutexIOMap(mtIOMapMutex);
    	for(;iNumBytes > 0; --iNumBytes, ++pIOByte, ++pValByte)
    		*pValByte = *pIOByte;
    }
    else return false;
    return true;

}