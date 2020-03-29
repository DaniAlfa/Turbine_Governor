#include <iostream>
#include <IOMasterDrv.h>
#include <ModbusMasterDrv.h>
#include <CommonTypes.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <signal.h>

using namespace std;

static IOMasterDrv *drv = nullptr;

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

	IOMasterDrv* drv = new ModbusMasterDrv();
	signal(SIGINT, interruptTest);
	signal(SIGTSTP, interruptTest);
	if(!drv->init(strConfigPath, [](IOAddr addr){cout << "TimeOut en direccion " << (unsigned) addr.uiChannel << endl;}, []{})){ 
		cout << "Error: " << drv->getLastErrorInfo() << endl;
		return -1;
	}
	cout << "Driver configurado" << endl;

	std::uint32_t uiOutID = 0;
	IOAddr addr1;
    addr1.uiHeader = 0;
	addr1.uiModule = 0;
	addr1.uiChannel = 1;
	addr1.uiNumBits = 1;

	RegVar digitalOut(uiOutID, addr1);

	std::uint32_t uiInID = 1;
	IOAddr addr2;
    addr2.uiHeader = 0;
	addr2.uiModule = 0;
	addr2.uiChannel = 2;
	addr2.uiNumBits = 16;

	RegVar AnalogInput(uiInID, addr2);
	AnalogInput.setEguMin(0);
	AnalogInput.setEguMax(100);	
	AnalogInput.setCurrentVal(0);

	if(!drv->start()){
		cout << "El driver no pudo arrancar" << endl;
		return -1;
	}
	cout << "Driver arrancado con exito" << endl;
	sleep(2);

	float fForcedVal = 0;
	bool state = false;
	auto startTime = chrono::high_resolution_clock::now();
	auto endTime = chrono::high_resolution_clock::now();
	double elapsedTime = 0;
	double lastForcedInterval = 0;
	int scanTime = 40;
	int forceInterval = 1000;
	while(elapsedTime < 100000){
		this_thread::sleep_for(chrono::milliseconds(scanTime));
		if(elapsedTime < 50000){
			if(elapsedTime > 10000 && elapsedTime - lastForcedInterval >= forceInterval){
				lastForcedInterval = elapsedTime;
				AnalogInput.setForced(true);
				fForcedVal += 5;
				fForcedVal = ((int) fForcedVal % 100);
				AnalogInput.setForcedVal(fForcedVal);
				drv->force(AnalogInput, 1000);
			}
			if(drv->read(AnalogInput)){
				cout << "Entrada Analogica Ethercat--Valor verdadero: " << AnalogInput.getTrueVal() << " ,Valor forzado: " << AnalogInput.getForcedVal();
				cout << " Variable forzada: " << ((AnalogInput.getForced()) ? "SI" : "NO") <<" ,TimeStamp: ";
				cout << AnalogInput.getTimeS() << " OverRange: " << ((AnalogInput.getQState() == OverRange) ? "SI" : "NO");
				cout << " UnderRange: " << ((AnalogInput.getQState() == UnderRange) ? "SI" : "NO") << endl;
			}
			else cout << "Error de lectura analogica en ID: " << AnalogInput.getID() << endl;
		}
		else{
			if(elapsedTime > 60000 && elapsedTime - lastForcedInterval >= forceInterval){
				lastForcedInterval = elapsedTime;
				digitalOut.setForced(true);
				state = !state;
				if(!state){
					digitalOut.setForcedVal(0);
				}
				else digitalOut.setForcedVal(1);
				drv->force(digitalOut, 1000);
			}
			if(drv->read(digitalOut)){
				cout << "Salida Digital Ethercat--Valor verdadero: " << digitalOut.getTrueVal() << " ,Valor forzado: " << digitalOut.getForcedVal();
				cout << " Variable forzada: " << ((digitalOut.getForced()) ? "SI" : "NO") << endl;
			}
			else cout << "Error de lectura digital ethercat en ID: " << digitalOut.getID() << endl;
		}
		endTime = chrono::high_resolution_clock::now();
		elapsedTime = chrono::duration<double, std::milli>(endTime - startTime).count();
	}
	cout << "Fin de la prueba con exito" << endl;
	delete drv;
	return 0;
} 
