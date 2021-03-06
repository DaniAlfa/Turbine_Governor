#include <iostream>
#include <IOFieldDrv.h>
#include <CommonTypes.h>
#include <unistd.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <EthercatDrv.h>

#include <signal.h>

using namespace std;

static IOFieldDrv *drv = nullptr;

static void interruptTest(int){
	if(drv != nullptr){
		drv->close();
		delete drv;
	}
	cout << endl << "Test interrumpido" << endl;
	exit(0);
}

static void errorCallBack(){
	DrvState st = drv->getState();
	cout << "Error en driver: ";
	if(st == COMError){
		cout << "COM Error" << endl;
	}
	else if(st == VarError){
		cout << "Var Error" << endl;
		std::unordered_map<IOAddr, QState> mErrors;
		drv->getVarErrors(mErrors);
		for(auto it : mErrors){
			cout << "Addr: Esclavo " << (unsigned) (it.first.uiModule) << " Canal " << (unsigned) (it.first.uiChannel) << endl;
			cout << "QState: ";
			if(it.second == ComError){
				cout << "ComError" << endl;
			}
			else if(it.second == OverRange){
				cout << "OverRange" << endl;
			}
			else if(it.second == UnderRange){
				cout << "UnderRange" << endl;
			}
		}
	}
}

int main(int argc, char* argv[]){
	const char* strConfigPath = (argc == 2) ? argv[1] : "";

	drv = new EthercatDrv();
	signal(SIGINT, interruptTest);
	signal(SIGTSTP, interruptTest);
	if(!drv->init(strConfigPath, errorCallBack)){ 
		cout << "Error: " << drv->getLastErrorInfo() << endl;
		delete drv;
		return -1;
	}
	cout << "Driver configurado" << endl;

	std::uint32_t digInID = 0;
	IOAddr addr1;
    addr1.uiHeader = 0;
	addr1.uiModule = 2;
	addr1.uiChannel = 0;
	addr1.uiNumBits = 1;

	IOVar digitalInput(digInID, addr1);

	std::uint32_t digOutID = 1;
	IOAddr addr2;
    addr2.uiHeader = 0;
	addr2.uiModule = 3;
	addr2.uiChannel = 7;
	addr2.uiNumBits = 1;

	IOVar digitalOut(digOutID, addr2);
	//digitalOut.setCurrentVal(0);


	std::uint32_t AngInID = 2;
	IOAddr addr3;
    addr3.uiHeader = 0;
	addr3.uiModule = 4;
	addr3.uiChannel = 32;
	addr3.uiNumBits = 16;

	IOVar AnalogInput(AngInID, addr3);
	AnalogInput.setEguMin(0);
	AnalogInput.setEguMax(100);	
	AnalogInput.setHwMin(0);
	AnalogInput.setHwMax(32767);

	std::uint32_t writePulseID = 3;
	std::uint32_t readPulseID = 3;
	IOAddr addr4;
    addr4.uiHeader = 0;
	addr4.uiModule = 5;
	addr4.uiChannel = 32;
	addr4.uiNumBits = 16;

	IOVar pulseOut(writePulseID, addr4);
	IOVar pulseIn(readPulseID, addr4);

	if(!drv->start()){
		cout << "El driver no pudo arrancar" << endl;
		delete drv;
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

		if(elapsedTime < 30000){
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
				cout << digitalInput.getTimeS() << " Duracion pulso: " << digitalInput.getPulseDur() << " TimeStamp Pulso: " << digitalInput.getPulseTimeS() << endl;
			}
			else cout << "Error de lectura digital en ID: " << digitalInput.getID() << endl;
		}
		else if(elapsedTime < 80000){
			this_thread::sleep_for(chrono::milliseconds(scanTime));
			if(drv->read(AnalogInput)){
				cout << "Entrada Analogica--Valor leido: " << AnalogInput.getCurrentVal() << " ,TimeStamp: ";
				cout << AnalogInput.getTimeS() << " OverRange: " << ((AnalogInput.getQState() == OverRange) ? "SI" : "NO");
				cout << " UnderRange: " << ((AnalogInput.getQState() == UnderRange) ? "SI" : "NO") << endl;
			}
			else cout << "Error de lectura analogica en ID: " << AnalogInput.getID() << endl;
		}
		else if(elapsedTime < 120000){
			this_thread::sleep_for(chrono::milliseconds(scanTime));
			if(drv->read(pulseIn)){
				cout << "Entrada Pulsos--Valor leido: " << pulseIn.getCurrentVal() << " ,TimeStamp: ";
				cout << pulseIn.getTimeS() << endl;
				if(pulseIn.getCurrentVal() > 60){
					pulseOut.setCurrentVal(50);
					//drv.write(pulseOut);
					if(!drv->write(pulseOut)) cout << "Error de escritura de pulsos en ID: " << pulseOut.getID() << endl;
				}
			}
			else cout << "Error de lectura de pulsos en ID: " << pulseIn.getID() << endl;
		}
		endTime = chrono::high_resolution_clock::now();
		elapsedTime = chrono::duration<double, std::milli>(endTime - startTime).count();
	}
	cout << "Fin de la prueba con exito" << endl;
	drv->stop();
	drv->close();
	delete drv;
	return 0;
} 
