#ifndef REGIMAGE_H
#define REGIMAGE_H

#include <QObject>
#include <QString>
#include <cstdint>
#include <queue>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <QtXml>
#include "IOMasterDrv.h"
#include "CommonHMITypes.h"

class RegImage : public QObject{
	Q_OBJECT
public:
	RegImage();
	~RegImage();
	bool init(QString const& regIOInfo, QString const& regConfigFile, QString const& masterDrvConfig, QString* strErrorInfo = nullptr);
	bool start();
	bool stop();

	void updateImage();
	
	float getVarMin(std::uint32_t const varID) const;
	float getVarMax(std::uint32_t const varID) const;
	QString getVarUnits(std::uint32_t const varID) const;
	QString getVarTag(std::uint32_t const varID) const;
	QString getVarDesc(std::uint32_t const varID) const;

	bool writeVar(std::uint32_t const varID, float val);
	bool forceVar(std::uint32_t const varID, float forceVal, bool forceBit = true);

	bool writeVarAsButton(std::uint32_t const varID, bool val);


	int getNoOfSpeedSensors() const{ return 3;}
	int getNoOfPowerSensors() const{ return 2;}
	TurbType getTurbineType() const {return Pelton;} 
	int getNoPeltonInyec() const {return 6;}
	std::uint32_t getNumLogicErrorInts() const {return muiNumLogicErrorInts;}
	std::uint32_t getNumFieldStInts() const {return muiNumFieldQStatesInts;}

signals:
	void allVarsUpdated(); //Indica al inicio cuando se han leido todas las variables al menos una vez
	void comError();
	void recoveredFromComError();

	void varChanged(VarImage const& var);
	void roIntChanged(std::uint32_t const val, std::uint32_t const intID);

	void writeError(std::uint32_t const varID);


private:
	IOMasterDrv* mMasterDrv;

	std::unordered_map<IOAddr, RegVar*> mumVars;

	std::unordered_map<std::uint32_t, RegVar*> mumSlaveVars;
	std::unordered_map<std::uint32_t, RegVar*> mumFieldVars;


	std::uint32_t muiNumLogicErrorInts;
	std::uint32_t muiNumFieldQStatesInts;
	std::uint32_t muiVarsToUpdate;

	std::unordered_set<IOAddr>* usLastVarChanges; //Para saber cuando se han leido todas las variables 1 vez


	//Estructuras para gestionar la escritura como boton (escribo 1 y luego la imagen se encarga de escribir el 0 cuando termine)
	std::mutex mtButtonWritesMtx;

	std::unordered_map<IOAddr, bool> mumButtonWrites; //Tabla para marcar la escritura de primera fase
	std::unordered_map<IOAddr, bool> mumButtonWritesToClean; //Tabla para marcar la escritura de segunda fase
	std::queue<std::pair<IOAddr, bool>> mqUncompletedButtonWrites; //Tabla para guadar las escrituras de segunda fase incompletas y realizarlas en otro momento

	void deleteRegVars();
	bool parseRegIOInfo(QString const& regIOInfo, std::unordered_set<IOAddr> & slaveVarsToUpdate, std::unordered_set<IOAddr> & fieldVarsToUpdate);
	bool parseXmlVar(QXmlStreamReader & reader, std::unordered_set<IOAddr> & slaveVarsToUpdate, std::unordered_set<IOAddr> & fieldVarsToUpdate);
	bool parseRegConfigFile(QString const& regConfigFile);

	void driverComError();
	void driverRecovered();
	void driverWriteTimeOut(IOAddr tAddr);
	void driverWriteSuccess(IOAddr tAddr);
	void newVarUpdated(IOAddr tAddr); //Funcion para registar la variable en usLastVarChanges y emitir la señal cuando sea apropiado
	void processUncompletedWrites();
};





#endif //REGIMAGE_H 
