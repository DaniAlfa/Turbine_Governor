#ifndef DEVICE_FACTORY_H
#define DEVICE_FACTORY_H
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cstdint>

//Declaraciones adelantadas
class EtherDevice;
class EthercatDrv;
class IOAddr;

class DeviceFactory{
public:
	DeviceFactory();
	~DeviceFactory();

	EtherDevice* getNewXMLDevice(xmlDoc* doc, xmlNode* pNode, EthercatDrv & drv);


	static bool parseXMLAddr(xmlNode* pNode, IOAddr & addr);
	static bool parseXML8bitIntAttr(xmlNode* pNode, const char* attr, std::uint8_t & iVal);


private:

};









#endif