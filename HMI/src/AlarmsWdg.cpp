#include "AlarmsWdg.h"
#include <IDVarList.h>
#include <algorithm>
#include <QMetaObject>
#include <QDateTime>
#define NUM_QUSTATE_BITS 2 //Solo con bits pares
#define QUSTATE_MASK 0x3

QString AlarmsWdg::mStrLogicErrors[NUM_ALARMS] = ALARM_DESC;

AlarmsWdg::AlarmsWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg), mpLogicErrors(nullptr), mpFieldSt(nullptr){
    setupUi(this);

    muiNumLogicErrorsInts = regImg.getNumLogicErrorInts();
    muiNumFieldStInts = regImg.getNumFieldStInts();
    mpLogicErrors = new std::uint32_t[muiNumLogicErrorsInts];
    mpFieldSt = new std::uint32_t[muiNumFieldStInts];
    connect(&regImg, &RegImage::roIntChanged, this, &AlarmsWdg::roIntChanged);
}

AlarmsWdg::~AlarmsWdg(){
	if(mpLogicErrors != nullptr)
		delete[] mpLogicErrors;
	if(mpFieldSt != nullptr)
    	delete[] mpFieldSt;
}


void AlarmsWdg::roIntChanged(std::uint32_t const val, std::uint32_t const intID){
	if(intID > 0){
		if(intID <= muiNumLogicErrorsInts){
			logicErrorIntChanged(val, intID);
		}
		else if(intID <= muiNumFieldStInts + muiNumLogicErrorsInts){
			fieldStIntChanged(val, intID);
		}
	}
}

void AlarmsWdg::logicErrorIntChanged(std::uint32_t const val, std::uint32_t const intID){
	int idx = intID - 1;
	std::uint32_t oldInt = mpLogicErrors[idx];
	std::uint32_t offset = 32 * idx;
	std::uint32_t newErrors = (val ^ oldInt) & (~oldInt);
	std::uint32_t mask = (1 << 31);
	int iStopBit = std::min<std::uint32_t>(offset + 32, NUM_ALARMS);
	for(int i = offset; i < iStopBit; ++i){
		if(mask & newErrors){
			printErrorInLog(mStrLogicErrors[i]);
		}
		mask = (mask >> 1);
	}
	mpLogicErrors[idx] = val;
	//QMetaObject::invokeMethod(this, "updateAlarmsDisplay", Qt::QueuedConnection);
	updateAlarmsDisplay();
}

void AlarmsWdg::fieldStIntChanged(std::uint32_t const val, std::uint32_t const intID){
	int idx = intID - 1 - muiNumLogicErrorsInts;
	std::uint32_t oldInt = mpFieldSt[idx];
	std::uint32_t mask = (QUSTATE_MASK << (32 - NUM_QUSTATE_BITS));
	std::uint32_t bitOffset = (32 * idx);
	std::uint32_t varOffset =  bitOffset / NUM_QUSTATE_BITS;
	int iStopBit = std::min<std::uint32_t>(32, ((FLD_IN_VARS + FLD_OUT_VARS) * NUM_QUSTATE_BITS) - bitOffset);
	for(int i = 0; i < iStopBit; i = i + NUM_QUSTATE_BITS){
		std::uint32_t newQuState = mask & val;
		std::uint32_t oldQuState = mask & oldInt;
		if(newQuState && newQuState != oldQuState){
			std::uint32_t varID;
			if(varOffset < FLD_IN_VARS){ // Entrada
				varID = FLD_IN_LOW_RANGE + varOffset;
			}
			else{
				varID = FLD_OUT_LOW_RANGE + varOffset - FLD_IN_VARS;
			}
			QuState qstate = static_cast<QuState>(newQuState >> ((32 - NUM_QUSTATE_BITS) - i));
			printErrorInLog(mpRegImg->getVarTag(varID), mpRegImg->getVarDesc(varID), qstate);
		}
		mask = (mask >> NUM_QUSTATE_BITS);
		++varOffset;
	}
	mpFieldSt[idx] = val;
	//QMetaObject::invokeMethod(this, "updateAlarmsDisplay", Qt::QueuedConnection);
	updateAlarmsDisplay();
}

void AlarmsWdg::printErrorInLog(QString const& strError){
	txtEditLastAl->appendHtml(QString("<p style=\"color:red\">") + getCurrentDateTime() + QString(":: Alarma: ") + strError + QString("</p>"));
}

void AlarmsWdg::printErrorInLog(QString const& strTag, QString const& strDesc, QuState qState){
	txtEditLastAl->appendHtml(QString("<p style=\"color:red\">") + getCurrentDateTime() + QString(":: Fallo en señal [") + strTag + QString("-") +
		strDesc + QString("]: ") + qualityStateToString(qState) + QString("</p>"));
}

void AlarmsWdg::updateAlarmsDisplay(){
	QString strLogicErrors = genLogicErrorsReport();
	QString strFieldSt = genFieldStReport();
	txtEditActive->clear();
	if(strLogicErrors == "" && strFieldSt == ""){
		txtEditActive->appendHtml(QString("<p style=\"color:green\">:: La regulación funciona correctamente</p>"));
	}
	else{
		if(strLogicErrors != ""){
			txtEditActive->appendHtml(QString("<pre style=\"color:red\">:: Alarmas:\n") + strLogicErrors + QString("</pre>"));
			txtEditActive->appendHtml("<p></p>");
		}
		if(strFieldSt != ""){
			txtEditActive->appendHtml(QString("<pre style=\"color:red\">:: Fallos en señales:\n") + strFieldSt + QString("</pre>"));
		}
	}
}

QString AlarmsWdg::genLogicErrorsReport() const{
	QString strRep = "";
	for(std::uint32_t i = 0; i < muiNumLogicErrorsInts; ++i){
		appendLogicErrorsRep(mpLogicErrors[i], i, strRep);
	}
	return strRep;
}

void AlarmsWdg::appendLogicErrorsRep(std::uint32_t uiVal, std::uint32_t uiIdx, QString & strRep) const{
	std::uint32_t offset = 32 * uiIdx;
	std::uint32_t mask = (1 << 31);
	int iStopBit = std::min<std::uint32_t>(32 + offset, NUM_ALARMS);
	for(int i = offset; i < iStopBit; ++i){
		if(mask & uiVal){
			strRep.append("	");
			strRep.append(mStrLogicErrors[i]);
			strRep.append("\n");
		}
		mask = (mask >> 1);
	}
}

QString AlarmsWdg::genFieldStReport() const{
	QString strRep = "";
	for(std::uint32_t i = 0; i < muiNumFieldStInts; ++i){
		appendFieldStRep(mpFieldSt[i], i, strRep);
	}
	return strRep;
}

void AlarmsWdg::appendFieldStRep(std::uint32_t uiVal, std::uint32_t uiIdx, QString & strRep) const{
	std::uint32_t mask = (QUSTATE_MASK << (32 - NUM_QUSTATE_BITS));
	std::uint32_t bitOffset = (32 * uiIdx);
	std::uint32_t varOffset =  bitOffset / NUM_QUSTATE_BITS;
	int iStopBit = std::min<std::uint32_t>(32, ((FLD_IN_VARS + FLD_OUT_VARS) * NUM_QUSTATE_BITS) - bitOffset);
	for(int i = 0; i < iStopBit; i = i + NUM_QUSTATE_BITS){
		std::uint32_t quState = mask & uiVal;
		if(quState){
			std::uint32_t varID;
			if(varOffset < FLD_IN_VARS){ // Entrada
				varID = FLD_IN_LOW_RANGE + varOffset;
			}
			else{
				varID = FLD_OUT_LOW_RANGE + varOffset - FLD_IN_VARS;
			}
			QuState qstate = static_cast<QuState>(quState >> ((32 - NUM_QUSTATE_BITS) - i));
			strRep.append("	");
			strRep.append("[");
			strRep.append(mpRegImg->getVarTag(varID));
			strRep.append("-");
			strRep.append(mpRegImg->getVarDesc(varID));
			strRep.append("]: ");
			strRep.append(qualityStateToString(qstate));
			strRep.append("\n");
		}
		mask = (mask >> NUM_QUSTATE_BITS);
		++varOffset;
	}
}

void AlarmsWdg::on_cmdRepo_clicked(){
	mpRegImg->writeVarAsButton(HMSM_REG_ALREP, true);
}

QString AlarmsWdg::getCurrentDateTime(){
	QDateTime dt = QDateTime::currentDateTime();
	return QString("[") + dt.toString("dd.MM.yyyy-hh:mm:ss") + QString("//") + QString("%1]").arg(dt.toMSecsSinceEpoch());
}

QString AlarmsWdg::qualityStateToString(QuState const st){
	switch(st){
	case OK: return "OK";
	case OverRange: return "OverRange";
	case UnderRange: return "UnderRange";
	case ComError: return "Perdida de conexión";
	}
	return "";
}