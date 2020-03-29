#include <iostream>
#include <IOSlaveDrv.h>
#include <IOFieldDrv.h>
#include <ModbusSlaveDrv.h>
#include <EthercatDrv.h>
#include <CommonTypes.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <signal.h>

using namespace std;

static IOSlaveDrv *slaveDrv = nullptr;
static IOFieldDrv *fieldDrv = nullptr;

static void interruptTest(int){
	if(slaveDrv != nullptr){
		slaveDrv->close();
		delete slaveDrv;
	}
	if(fieldDrv != nullptr){
		fieldDrv->close();
		delete fieldDrv;
	}
	cout << endl << "Test interrumpido" << endl;
	exit(0);
}


int main(int argc, char* argv[]){
	const char* strConfigPathModbus = (argc == 3) ? argv[1] : "";
	const char* strConfigPathField = (argc == 3) ? argv[2] : "";

	slaveDrv = new ModbusSlaveDrv();
	fieldDrv = new EthercatDrv();
	signal(SIGINT, interruptTest);
	signal(SIGTSTP, interruptTest);
	if(!slaveDrv->init(strConfigPathModbus, []{})){ 
		cout << "Error: " << slaveDrv->getLastErrorInfo() << endl;
		return -1;
	}
	if(!fieldDrv->init(strConfigPathField)){ 
		cout << "Error: " << fieldDrv->getLastErrorInfo() << endl;
		return -1;
	}
	cout << "Drivers configurados" << endl;

	IOAddr addr2;
    addr2.uiHeader = 0;
	addr2.uiModule = 3;
	addr2.uiChannel = 7;
	addr2.uiNumBits = 1;

	IOVar digitalOut(0, addr2);
	digitalOut.setCurrentVal(0);
	digitalOut.setQState(OK);

	IOAddr addr3;
    addr3.uiHeader = 0;
	addr3.uiModule = 4;
	addr3.uiChannel = 32;
	addr3.uiNumBits = 16;

	IOVar AnalogInput(1, addr3);
	AnalogInput.setEguMin(0);
	AnalogInput.setEguMax(100);	
	AnalogInput.setHwMin(0);
	AnalogInput.setHwMax(32767);
	AnalogInput.setCurrentVal(0);
	AnalogInput.setQState(OK);

	//Inicializacion de los valores modbus
	//slaveDrv->write(digitalInput);
	//slaveDrv->write(digitalOut);

	if(!fieldDrv->start()){
		cout << "El driver ethercat no pudo arrancar" << endl;
		return -1;
	}
	cout << "Driver ethercat arrancado con exito" << endl;
	if(!slaveDrv->start()){
		cout << "El driver modbus no pudo arrancar" << endl;
		return -1;
	}
	cout << "Driver modbus arrancado con exito" << endl;
	sleep(2);

	bool state = false;
	auto startTime = chrono::high_resolution_clock::now();
	auto endTime = chrono::high_resolution_clock::now();
	double elapsedTime = 0;
	int scanTime = 40;
	while(elapsedTime < 100000){
		this_thread::sleep_for(chrono::milliseconds(scanTime));
		if(elapsedTime < 50000){
			if(fieldDrv->read(AnalogInput)){
				if(slaveDrv->updateFieldVar(AnalogInput)){
					cout << "Entrada Analogica--Valor leido: " << AnalogInput.getCurrentVal() << " ,TimeStamp: ";
					cout << AnalogInput.getTimeS() << " OverRange: " << ((AnalogInput.getQState() == OverRange) ? "SI" : "NO");
					cout << " UnderRange: " << ((AnalogInput.getQState() == UnderRange) ? "SI" : "NO") << endl;
				}
				else cout << "Error de lectura analogica modbus en ID: " << AnalogInput.getID() << endl;
			}
			else cout << "Error de lectura analogica ethercat en ID: " << AnalogInput.getID() << endl;
		}
		else{
			state = !state;
			if(!state){
				digitalOut.setCurrentVal(0);
			}
			else digitalOut.setCurrentVal(1);
			if(slaveDrv->updateFieldVar(digitalOut)){
				if(fieldDrv->write(digitalOut)){
					cout << "Salida digital--Valor verdadero: " << digitalOut.getTrueVal() << " ,Valor forzado: " << digitalOut.getForcedVal();
					cout << " Variable forzada: " << ((digitalOut.getForced()) ? "SI" : "NO") << endl;
				}
				else cout << "Error de escritura digital ethercat en ID: " <<  digitalOut.getID() << endl;
			}
			else cout << "Error de escritura digital modbus en ID: " << AnalogInput.getID() << endl;
		}
		endTime = chrono::high_resolution_clock::now();
		elapsedTime = chrono::duration<double, std::milli>(endTime - startTime).count();
	}
	cout << "Fin de la prueba con exito" << endl;
	//drv->stop();
	//drv->close();
	delete fieldDrv;
	delete slaveDrv;
	return 0;
} 
