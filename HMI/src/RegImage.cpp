#include "RegImage.h"
#include "ModbusMasterDrv.h"
#include <unordered_set>
#include <QtXml>
#include <QFile>
#include <Alarms.h>
#include <RegIDS.h>
#include <cmath>
#include <utility>

#define DEFAULT_WRITE_TIMEOUT 1000


RegImage::RegImage(){
	mMasterDrv = new ModbusMasterDrv();
	usLastVarChanges = nullptr;
}

RegImage::~RegImage(){
	delete mMasterDrv;
	deleteRegVars();
	if(usLastVarChanges != nullptr) delete usLastVarChanges;
}

bool RegImage::init(QString const& regIOInfo, QString const& regConfigFile, QString const& masterDrvConfig, QString* strErrorInfo){
	std::unordered_set<IOAddr> usSlaveVarsToUpdate, usFieldVarsToUpdate;

	if(!parseRegIOInfo(regIOInfo, usSlaveVarsToUpdate, usFieldVarsToUpdate)){
		if(strErrorInfo != nullptr) *strErrorInfo = "Error en el archivo de informacion de IO";
		return false;
	}

	if(!parseRegConfigFile(regConfigFile)){
		if(strErrorInfo != nullptr) *strErrorInfo = "Error en el archivo de configuracion del regulador";
		deleteRegVars();
		return false;
	}

	muiNumLogicErrorInts = (std::uint32_t) ceil((NUM_ALARMS) / 32); 
	muiNumFieldQStatesInts = (std::uint32_t) ceil(((FLD_IN_VARS + FLD_OUT_VARS) * 2) / 32);

	for(int i = 1; i <= muiNumFieldQStatesInts + muiNumLogicErrorInts; ++i){ //Se añaden los enteros de informacion del regulador para actualizar
		IOAddr tAddr;
		tAddr.uiChannel = i;
		usSlaveVarsToUpdate.insert(tAddr);
	}

	if(!mMasterDrv->init(masterDrvConfig.toStdString(), usSlaveVarsToUpdate, usFieldVarsToUpdate, RegImage::driverComError, RegImage::driverRecovered)){
		if(strErrorInfo != nullptr) *strErrorInfo = "Error en la inicializacion del driver";
		deleteRegVars();
		return false;
	}
	usLastVarChanges = new std::unordered_set<IOAddr>();

	return true;
}

bool RegImage::start(){
	return mMasterDrv->start();
}

bool RegImage::stop(){
	return mMasterDrv->stop();
}

void RegImage::updateImage(){
	std::unordered_set<IOAddr> usChanges;
	if(mMasterDrv->getState() != Running) return;
	processUncompletedWrites();
	mMasterDrv->getChangedVars(usChanges);
	std::uint32_t ioInts = muiNumFieldQStatesInts + muiNumLogicErrorInts;
	for(IOAddr tAddr : usChanges){
		if(ioInts > 0 && tAddr.uiChannel >= 1 && tAddr.uiChannel <= ioInts){
			std::uint32_t uiVal;
			if(mMasterDrv->read(uiVal, tAddr)){
				emit roIntChanged(uiVal, tAddr.uiChannel);
				newVarUpdated(tAddr);
			}
		}
		else{
			RegVar* regVar;
			auto it = mumVars.find(tAddr);
			if(it == mumVars.end()) continue;
			else regVar = it->second;
			VarImage vImage(regVar->getID());
			vImage.setEguMin(regVar->getEguMin());
			vImage.setEguMax(regVar->getEguMax());
			vImage.setUnits(regVar->getUnits());
			if(mMasterDrv->read(vImage, tAddr)){
				emit varChanged(vImage);
				newVarUpdated(tAddr);
			}
		}
	}
}

void RegImage::processUncompletedWrites(){
	unique_lock<mutex> mutexButtonWrt(mtButtonWritesMtx);
	while(!mqUncompletedButtonWrites.empty()){ //Las escrituras de boton no completadas en segunda fase se procesan ahora
		std::pair<IOAddr, bool> butWrite = mqUncompletedButtonWrites.front();
		if(mMasterDrv->write((butWrite.second ? 1 : 0), butWrite.first, RegImage::driverWriteSuccess, RegImage::driverWriteTimeOut, DEFAULT_WRITE_TIMEOUT)){
			mqUncompletedButtonWrites.pop();
		}
		else break;
	}
}

void RegImage::newVarUpdated(IOAddr tAddr);{
	if(usLastVarChanges != nullptr){
		usLastVarChanges->insert(tAddr);
		if(usLastVarChanges->size() == mumVars.size() + muiNumFieldQStatesInts + muiNumLogicErrorInts){
			emit allVarsUpdated();
			delete usLastVarChanges;
			usLastVarChanges = nullptr;
		}
	}
}

float RegImage::getVarMinVal(std::uint32_t const varID) const{
	float fMin = 0;
	auto it = mumSlaveVars.find(varID);
	if(it == mumSlaveVars.end()){
		auto it2 = mumFieldVars.find(varID);
		if(it2 != mumFieldVars.end()) fMin = it2->second->getEguMin();
	}
	else fMin = it->second->getEguMin();
	return fMin;
}

float RegImage::getVarMaxVal(std::uint32_t const varID) const{
	float fMax = 0;
	auto it = mumSlaveVars.find(varID);
	if(it == mumSlaveVars.end()){
		auto it2 = mumFieldVars.find(varID);
		if(it2 != mumFieldVars.end()) fMax = it2->second->getEguMax();
	}
	else fMax = it->second->getEguMax();
	return fMax;
}

QString RegImage::getVarUnits(std::uint32_t const varID) const{
	QString strUnits;
	auto it = mumSlaveVars.find(varID);
	if(it == mumSlaveVars.end()){
		auto it2 = mumFieldVars.find(varID);
		if(it2 != mumFieldVars.end()) strUnits = it2->second->getUnits();
	}
	else strUnits = it->second->getUnits();
	return strUnits;
}


bool RegImage::writeVar(std::uint32_t const varID, float val){
	auto it = mumSlaveVars.find(varID);
	if(it == mumSlaveVars.end()) return false;
	return mMasterDrv->write(val, it->second->getAddr(), nullptr, RegImage::driverWriteTimeOut, DEFAULT_WRITE_TIMEOUT);
}

bool RegImage::forceVar(std::uint32_t const varID, float forceVal, bool forceBit){
	auto it = mumFieldVars.find(varID);
	if(it == mumFieldVars.end()) return false;
	return mMasterDrv->force(forceVal, it->second->getAddr(), forceBit, nullptr, RegImage::driverWriteTimeOut, DEFAULT_WRITE_TIMEOUT);
}

bool RegImage::writeVarAsButton(std::uint32_t const varID, bool val){
	auto it = mumSlaveVars.find(varID);
	if(it == mumSlaveVars.end()) return false;
	IOAddr tAddr = it->second->getAddr();
	unique_lock<mutex> mutexButtonWrt(mtButtonWritesMtx);
	if(mumButtonWritesToClean.count(tAddr) <= 0 && mumButtonWrites.insert({tAddr, val}).second){
		if(mMasterDrv->write((val ? 1 : 0), tAddr, RegImage::driverWriteSuccess, RegImage::driverWriteTimeOut, DEFAULT_WRITE_TIMEOUT)){
			return true;
		}
		mumButtonWrites.erase(res.first);
	}
	return false;
}

void RegImage::driverWriteTimeOut(IOAddr tAddr){
	auto it = mumVars.find(tAddr);
	if(it != mumVars.end()){
		std::unordered_map<IOAddr, bool>::iterator it1;
		unique_lock<mutex> mutexButtonWrt(mtButtonWritesMtx);
		if(!mumButtonWrites.empty() && ((it1 = mumButtonWrites.find(tAddr)) != mumButtonWrites.end())){ //Si es timeout por primera fase de escritura de boton
			mumButtonWrites.erase(tAddr);
			mutexButtonWrt.unlock();
			emit writeError(it->second->getID());
		} 
		else if(!mumButtonWritesToClean.empty() && ((it1 = mumButtonWritesToClean.find(tAddr)) != mumButtonWritesToClean.end())){ //Si es timeout por segunda fase
			mqUncompletedButtonWrites.push({it1->first, it1->second});
		}
		else{ //Si es timeOut por cualquier otro tipo de escritura
			mutexButtonWrt.unlock();
			emit writeError(it->second->getID());
		} 
	}
}

void RegImage::driverWriteSuccess(IOAddr tAddr){
	unique_lock<mutex> mutexButtonWrt(mtButtonWritesMtx);
	auto it = mumButtonWrites.find(tAddr);
	if(it != mumButtonWrites.end()){ //Si tiene exito la primera fase de escritura de boton
		bool bPreviousVal = it->second;
		mumButtonWrites.erase(it);
		mumButtonWritesToClean.insert({tAddr, !bPreviousVal});
		if(!mMasterDrv->write((bPreviousVal ? 0 : 1), tAddr, RegImage::driverWriteSuccess, RegImage::driverWriteTimeOut, DEFAULT_WRITE_TIMEOUT)){
			mqUncompletedButtonWrites.push({tAddr, !bPreviousVal});
		}
	}
	else{
		auto it2 = mumButtonWritesToClean.find(tAddr);
		if(it2 != mumButtonWritesToClean.end()){ //Si tiene exito la segunda
			mumButtonWritesToClean.erase(it2);
		}
	}
}


bool RegImage::parseRegIOInfo(QString const& regIOInfo, std::unordered_set<IOAddr> & slaveVarsToUpdate, std::unordered_set<IOAddr> & fieldVarsToUpdate){
	QFile infoF(regIOInfo);
	if(!infoF.open(QFile::ReadOnly | QFile::Text)) return false;

	QXmlStreamReader xmlReader(&infoF);

	if(reader.readNextStartElement() && reader.name() == "RegIOInfo"){
		if(reader.readNextStartElement() && reader.name() == "Vars"){
			while(reader.readNextStartElement()){
				if(reader.name() == "Var"){
					if(!parseXmlVar(reader)){
						infoF.close();
						deleteRegVars();
						return false;
					}
				}
			}
			infoF.close();
			return true;
		}
	}
	infoF.close();
	deleteRegVars();
	return false;
}

bool RegImage::parseXmlVar(QXmlStreamReader reader, std::unordered_set<IOAddr> & slaveVarsToUpdate, std::unordered_set<IOAddr> & fieldVarsToUpdate){
	if(reader.attributes().hasAttribute("ID") && reader.attributes().hasAttribute("FR")){
		std::uint32_t uiId = reader.attributes().value("ID").toUInt();
		QString strFr = reader.attributes().value("FR").toString();
		if(strFr != "F" && strFr != "R") return false;
		QString strUse;
		IOAddr tAddr;
		bool bUseReaded = false;
		bool bAddrReaded = false; 
		while(reader.readNextStartElement()){
			if(reader.name() == "HMI"){
				if(reader.attributes().hasAttribute("Use")){
					strUse = reader.attributes().value("Use").toString();
					if(strUse == "NO" || strUse == "WO" || strUse == "RW")
						bUseReaded = true;
				}
			}
			else if(reader.name() == "RemAddr"){
				if(reader.attributes().hasAttribute("Head") && reader.attributes().hasAttribute("Mod") && reader.attributes().hasAttribute("Ch")){
					tAddr.uiHeader = reader.attributes().value("Head").toUInt();
					tAddr.uiModule = reader.attributes().value("Mod").toUInt();
					tAddr.uiChannel = reader.attributes().value("Ch").toUInt();
					bAddrReaded = true;
				}
			}
		}
		if(bUseReaded && bAddrReaded && strUse != "NO"){
			RegVar* var = new RegVar(uiId, tAddr);
			if(strFr == "F"){
				mumVars.insert({tAddr, var});
				mumFieldVars.insert({uiId, var});
				if(strUse == "RW"){
					fieldVarsToUpdate.insert(tAddr);
				}
			}
			else{
				mumVars.insert({tAddr, var});
				mumSlaveVars.insert({uiId, var});
				if(strUse == "RW"){
					slaveVarsToUpdate.insert(tAddr);
				}
			}
			return true;
		}
	}
	return false;
}

bool RegImage::parseRegConfigFile(QString const& regConfigFile){
	//Se leen los limites de las variables y sus unidades
	//Se leen las configuraciones del regulador
}


void RegImage::deleteRegVars(){
	for(auto var : mumVars){
		delete var.second;
	}
	mumVars.clear();
	mumSlaveVars.clear();
	mumFieldVars.clear();
}

void RegImage::driverComError(){
	emit comError();
}

void RegImage::driverRecovered(){
	emit recoveredFromComError();
}