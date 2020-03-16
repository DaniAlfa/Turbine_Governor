#include <iostream>
#include <IOSlaveDrv.h>
#include <ModbusSlaveDrv.h>
#include <CommonTypes.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <signal.h>

using namespace std;

static IOSlaveDrv *drv = nullptr;

static void interruptTest(int){
	if(drv != nullptr){
		drv->close();
		delete drv;
	}
	cout << endl << "Test interrumpido" << endl;
	exit(0);
}


int main(int argc, char* argv[]){
	const char* strConfigPath = (argc == 2) ? argv[1] : "";

	IOSlaveDrv* drv = new ModbusSlaveDrv();
	signal(SIGINT, interruptTest);
	signal(SIGTSTP, interruptTest);
	if(!drv->init(strConfigPath, []{})){ 
		cout << "Error: " << drv->getLastErrorInfo() << endl;
		return -1;
	}
	cout << "Driver configurado" << endl;

	std::uint32_t uiInputID = 5;
	IOAddr addr1;
    addr1.uiHeader = 0;
	addr1.uiModule = 0;
	addr1.uiChannel = 5;
	addr1.uiNumBits = 1;

	IOVar digitalInput(uiInputID, addr1);
	digitalInput.setCurrentVal(0);
	digitalInput.setQState(OK);

	std::uint32_t uiOutID = 6;
	IOAddr addr2;
    addr2.uiHeader = 0;
	addr2.uiModule = 0;
	addr2.uiChannel = 6;
	addr2.uiNumBits = 1;

	IOVar digitalOut(uiOutID, addr2);
	digitalOut.setCurrentVal(1);
	digitalOut.setQState(OK);

	//Inicializacion de los valores modbus
	drv->write(digitalInput);
	drv->write(digitalOut);

	if(!drv->start()){
		cout << "El driver no pudo arrancar" << endl;
		return -1;
	}
	cout << "Driver arrancado con exito" << endl;
	sleep(2);

	bool state = false;
	auto startTime = chrono::high_resolution_clock::now();
	auto endTime = chrono::high_resolution_clock::now();
	double elapsedTime = 0;
	int scanTime = 40;
	while(elapsedTime < 120000){
		this_thread::sleep_for(chrono::milliseconds(scanTime));
		state = !state;
		if(!state){
			digitalOut.setCurrentVal(0);
		}
		else digitalOut.setCurrentVal(1);
		if(!drv->write(digitalOut))
			cout << "Error de escritura digital en ID: " <<  digitalOut.getID() << endl;
		if(drv->read(digitalInput)){
			cout << "Entrada Digital--Valor leido: " << digitalInput.getCurrentVal() << " ,TimeStamp: ";
			cout << digitalInput.getTimeS() << " Estado de calidad: " << digitalInput.getQState() << endl;
		}
		else cout << "Error de lectura digital en ID: " << digitalInput.getID() << endl;
		endTime = chrono::high_resolution_clock::now();
		elapsedTime = chrono::duration<double, std::milli>(endTime - startTime).count();
	}
	cout << "Fin de la prueba con exito" << endl;
	drv->stop();
	drv->close();
	delete drv;
	return 0;
} 
