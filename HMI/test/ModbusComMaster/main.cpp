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
	if(!drv->init(strConfigPath, [](IOAddr addr){}, []{})){ 
		cout << "Error: " << drv->getLastErrorInfo() << endl;
		return -1;
	}
	cout << "Driver configurado" << endl;

	std::uint32_t uiOutID = 5;
	IOAddr addr1;
    addr1.uiHeader = 0;
	addr1.uiModule = 0;
	addr1.uiChannel = 5;
	addr1.uiNumBits = 1;

	RegVar digitalOut(uiOutID, addr1);
	digitalOut.setCurrentVal(1);

	std::uint32_t uiInID = 6;
	IOAddr addr2;
    addr2.uiHeader = 0;
	addr2.uiModule = 0;
	addr2.uiChannel = 6;
	addr2.uiNumBits = 1;

	RegVar digitalInput(uiInID, addr2);
	digitalInput.setCurrentVal(0);

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
		if(!drv->write(digitalOut,50000))
			cout << "Error de escritura digital en ID: " <<  digitalOut.getID() << endl;
		if(drv->read(digitalInput)){
			std::uint32_t uiVal = digitalInput.getCurrentVal();
			cout << "Entrada Digital--Valor leido: " << uiVal << " ,TimeStamp: ";
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
