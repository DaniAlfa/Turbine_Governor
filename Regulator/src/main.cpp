#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include "IOSlaveDrv.h"
#include "IOFieldDrv.h"
#include <ModbusSlaveDrv.h>
#include <EthercatDrv.h>
#include "IOImage.h"
#include "AlarmManager.h"
#include "Control.h"

#include <signal.h>

#define SCAN_TIME 40

static IOSlaveDrv *pSlaveDrv = nullptr;
static IOFieldDrv *pFieldDrv = nullptr;

static void exitReg(int status){
	std::cout << "Saliendo" << std::endl;
	if(pFieldDrv != nullptr){
		std::cout << "Cerrando driver de campo" << std::endl;
		pFieldDrv->close();
		delete pFieldDrv;
	}
	if(pSlaveDrv != nullptr){
		std::cout << "Cerrando driver esclavo" << std::endl;
		pSlaveDrv->close();
		delete pSlaveDrv;
	}
	exit(status);
}

static void closeSignalHandle(int){
	exitReg(0);
}

int main(int argc, char* argv[]){
	if(argc != 4){
		std::cout << "Los argumentos son invalidos, " <<  argv[0] << " configuracionReg.xml configDriverCampo.xml configDriverEsclavo.xml." << std::endl; 
		return -1;
	}
	signal(SIGINT, closeSignalHandle);
	signal(SIGTSTP, closeSignalHandle);

	pFieldDrv = new EthercatDrv();
	pSlaveDrv = new ModbusSlaveDrv();
	IOImage ioImage(*pFieldDrv, *pSlaveDrv);
	AlarmManager alMan(ioImage);
	ioImage.setAlarmManager(alMan);
	Control ctrl(ioImage, alMan);
/*
	if(!pFieldDrv->init(argv[2], [&alMan](){alMan.driverFailed();}) || !pFieldDrv->start()){
		std::cout << "Error en la configuracion del driver de campo: " << pFieldDrv->getLastErrorInfo() << std::endl; 
		exitReg(-1);
	}
	*/
	if(!pSlaveDrv->init(argv[3], [&alMan](){alMan.driverFailed();}) || !pSlaveDrv->start()){
		std::cout << "Error en la configuracion del driver esclavo: " << pSlaveDrv->getLastErrorInfo() << std::endl; 
		exitReg(-1);
	}
	if(!ioImage.init(argv[1])){
		std::cout << "Error en la inicializacion de la imagen" << std::endl; 
		exitReg(-1);
	}
	if(!ctrl.init(argv[1])){
		std::cout << "Error en la inicializacion del control" << std::endl; 
		exitReg(-1);
	}
	std::cout << "Regulador arrancado" << std::endl;
	std::uint32_t uiTotalCycles = 0;
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(SCAN_TIME));
		ioImage.updateInputs();
		alMan.checkDriverErrors();

		ctrl.updateControl();

		ioImage.updateOutputs();
		++uiTotalCycles;
		std::cout << "Ciclo " << uiTotalCycles << std::endl;
	}
	
	exitReg(0);
}
