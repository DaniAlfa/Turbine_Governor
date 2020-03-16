#include "DeviceFactory.h"
#include "DigitalDev.h"
#include "AnalogDev.h"
#include "PulseDev.h"
#include "CommonTypes.h"
#include <string>

DeviceFactory::DeviceFactory(){
}


DeviceFactory::~DeviceFactory(){
}


EtherDevice* DeviceFactory::getNewXMLDevice(xmlDoc* doc, xmlNode* pNode, EthercatDrv & drv){
	if(!pNode) return nullptr;
	xmlChar *strcType;
	strcType = xmlGetProp(pNode, (const xmlChar *)"Type");
	if(!strcType) return nullptr;
	std::string strType((const char*) strcType);
	xmlFree(strcType);
	xmlNode* pParam = pNode->xmlChildrenNode;
	if(strType == "DigitalDev"){
		while(pParam){
			if (xmlStrcmp(pParam->name, (const xmlChar *)"PulseMeasure") == 0) {
				strcType = xmlNodeListGetString(doc, pParam->xmlChildrenNode, 1);
				if(strcType){
					int iPulseMeasure = atoi((const char*) strcType);
					xmlFree(strcType);
					return new DigitalDev(drv, iPulseMeasure);
				}
				else return nullptr;
			}
			pParam = pParam->next;
		}

	}
	else if(strType == "AnalogDev"){
		AnalogDev::StatusWord stByte;
		EtherDevice::BitRepr bitRepr;
		std::uint8_t uiNumBits;
		bool bStatusW, bBitRepr, bNumBits;
		bStatusW = bBitRepr = bNumBits = false;
		while(pParam){
			if (xmlStrcmp(pParam->name, (const xmlChar *)"StatusWord") == 0) {
				xmlNode* pStatusWord = pParam->xmlChildrenNode;
				bool bAddr, bUnderBit, bOverBit;
				bAddr = bUnderBit = bOverBit = false;
				while(pStatusWord){
					if (xmlStrcmp(pStatusWord->name, (const xmlChar *)"Addr") == 0) {
						if(!parseXMLAddr(pStatusWord, stByte.tAddr)) return nullptr;
						bAddr = true;
					}
					else if(xmlStrcmp(pStatusWord->name, (const xmlChar *)"UnderRangeBit") == 0){
						strcType = xmlNodeListGetString(doc, pStatusWord->xmlChildrenNode, 1);
						if(strcType){
							stByte.uiUnderRangeBit = atoi((const char*) strcType);
							xmlFree(strcType);
							bUnderBit = true;
						}
						else return nullptr;
					}
					else if(xmlStrcmp(pStatusWord->name, (const xmlChar *)"OverRangeBit") == 0){
						strcType = xmlNodeListGetString(doc, pStatusWord->xmlChildrenNode, 1);
						if(strcType){
							stByte.uiOverRangeBit = atoi((const char*) strcType);
							xmlFree(strcType);
							bOverBit = true;
						}
						else return nullptr;
					}
					pStatusWord = pStatusWord->next;
				}
				if(bAddr && bUnderBit && bOverBit) bStatusW = true;
				else return nullptr;
			}
			else if (xmlStrcmp(pParam->name, (const xmlChar *)"BitRepr") == 0){
				strcType = xmlGetProp(pParam, (const xmlChar *)"Type");
				if(!strcType) return nullptr;
				std::string strBitRepr((const char*) strcType);
				xmlFree(strcType);
				bitRepr = EtherDevice::getBitRepr(strBitRepr);
				bBitRepr = true;
				strcType = xmlGetProp(pParam, (const xmlChar *)"NumBits");
				if(!strcType) return nullptr;
				uiNumBits = atoi((const char*) strcType);
				xmlFree(strcType);
				bNumBits = true;
			}
			pParam = pParam->next;
		}
		if(bStatusW && bBitRepr && bNumBits && AnalogDev::parametersOk(stByte, bitRepr, uiNumBits)){
			return new AnalogDev(drv, stByte, bitRepr, uiNumBits);
		}
	}
	else if(strType == "PulseDev"){
		PulseDev::ControlWord ctrByte;
		while(pParam){
			if (xmlStrcmp(pParam->name, (const xmlChar *)"ControlWord") == 0) {
				xmlNode* pControlWord = pParam->xmlChildrenNode;
				bool bAddr, bSetBit;
				bAddr = bSetBit = false;
				while(pControlWord){
					if (xmlStrcmp(pControlWord->name, (const xmlChar *)"Addr") == 0) {
						if(!parseXMLAddr(pControlWord, ctrByte.tAddr)) return nullptr;
						bAddr = true;
					}
					else if(xmlStrcmp(pControlWord->name, (const xmlChar *)"SetBit") == 0){
						strcType = xmlNodeListGetString(doc, pControlWord->xmlChildrenNode, 1);
						if(strcType){
							ctrByte.uiSetBit = atoi((const char*) strcType);
							xmlFree(strcType);
							bSetBit = true;
						}
						else return nullptr;
					}
					pControlWord = pControlWord->next;
				}
				if(bAddr && bSetBit && PulseDev::parametersOk(ctrByte)) return new PulseDev(drv, ctrByte);
			}
			pParam = pParam->next;
		}
	}
	return nullptr;
}


bool DeviceFactory::parseXMLAddr(xmlNode* pNode, IOAddr & addr){
	return parseXML8bitIntAttr(pNode, "Head", addr.uiHeader) && parseXML8bitIntAttr(pNode, "Mod", addr.uiModule) && parseXML8bitIntAttr(pNode, "Ch", addr.uiChannel) && 
	parseXML8bitIntAttr(pNode, "Prec", addr.uiNumBits);
}

bool DeviceFactory::parseXML8bitIntAttr(xmlNode* pNode, const char* attr, std::uint8_t & iVal){
	if(!pNode) return false;
	xmlChar *strAttr;
	strAttr = xmlGetProp(pNode, (const xmlChar *) attr);
	if(strAttr){
		iVal = atoi((const char*) strAttr);
		xmlFree(strAttr);
		return true;
	}
	return false;
}