#include <iostream>
#include "EthercatDrv.h"
#include "CommonTypes.h"
#include <unistd.h>
#include <cstdint>
#include <string>

#define RECIEVE_TIMEOUT 2000
#define STATE_CHANGE_TIMEOUT 8000000  

using namespace std;


int main(int argc, char* argv[]){
	const char* ifname = (argc == 2) ? argv[1] : "";

	EthercatDrv drv;
	string strError;
	if(!drv.init(ifname, strError)){ 
		cout << "Error: " << strError << endl;
		cout << "Ayuda: sudo " << argv[0] << " ethernet_interface_name" << endl;
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
	digitalOut.setCurrentVal(0);

	bool state = false;
	int counter = 0;
	drv.start();
	while(true){

		if(++counter > 10000){
			counter = 0;
			state = !state;
			if(!state){
				digitalOut.setCurrentVal(0);
			}
			else digitalOut.setCurrentVal(1);
			if(!drv.write(digitalOut))
				cout << "Error de escritura" << endl;
			if(drv.read(digitalInput)){
				cout << "Valor leido: " << digitalInput.getCurrentVal() << " ,TimeStamp: ";
				cout << digitalInput.getTimeS() << " Duracion pulso: " << digitalInput.getPulseDur() << " TimeStamp Pulso: " << digitalInput.getPulseTimeS() << endl;
			}
			else cout << "Error de lectura" << endl;
		}
			

	}
	drv.stop();
	drv.close();
	return 0;
} 