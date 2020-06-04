#ifndef COMMONHMITYPES_H 
#define COMMONHMITYPES_H

#include <QMetaType>
#include <QString>
#include <CommonTypes.h>

Q_DECLARE_METATYPE(IOAddr)

class VarImage{
public:
	VarImage(std::uint32_t uiVarId) : muiVarId(uiVarId), mtQState(OK), mbForcedVal(0) {}
	VarImage() : muiVarId(-1), mtQState(OK), mbForcedVal(0) {}

	std::uint32_t getID() const {return muiVarId;}
	void setCurrentVal(float fCurrentVal) { mfTrueVal = fCurrentVal; }
	float getCurrentVal() const {return (mbForcedVal) ? mfForcedVal : mfTrueVal;}
	float getCurrentValLin() const{
		float fVal = (mbForcedVal) ? mfForcedVal : mfTrueVal;
		return (((fVal - mfEguMin) * 100) / (mfEguMax - mfEguMin));
	}
	float getTrueVal() const {return mfTrueVal;}
	float getForcedVal() const {return mfForcedVal;}
	void setForcedVal(float fForcedVal){mfForcedVal = fForcedVal;}
	void setTimeS(std::int64_t const& iTimeS) { miTimeS = iTimeS;}
	std::int64_t getTimeS() const {return miTimeS;}
	float getEguMin() const {return mfEguMin;}
	float getEguMax() const {return mfEguMax;}
	void setEguMin(float fVal) {mfEguMin = fVal;}
	void setEguMax(float fVal) {mfEguMax = fVal;}
	void setQState(QuState tState){mtQState = tState;}
	QuState getQState() const {return mtQState;}
	void setForced(bool bForced) {mbForcedVal = bForced;}
	bool getForced() const {return mbForcedVal;}
	void setUnits(QString strUnits) {mstrUnits = strUnits;}
	QString getUnits() const {return mstrUnits;}

private:
	std::uint32_t muiVarId;
	std::int64_t miTimeS;
	float mfTrueVal;
	float mfForcedVal;
	float mfEguMin;
	float mfEguMax;
	QuState mtQState;
	bool mbForcedVal;
	QString mstrUnits;
};
Q_DECLARE_METATYPE(VarImage)

class RegVar{
public:
	RegVar(std::uint32_t uiVarId, IOAddr tAddr) : muiVarId(uiVarId), mtAddr(tAddr) {}

	std::uint32_t getID() const {return muiVarId;}
	IOAddr getAddr() const { return mtAddr; }
	float getEguMin() const {return mfEguMin;}
	float getEguMax() const {return mfEguMax;}
	void setEguMin(float fVal) {mfEguMin = fVal;}
	void setEguMax(float fVal) {mfEguMax = fVal;}
	void setUnits(QString strUnits) {mstrUnits = strUnits;}
	QString getUnits() const {return mstrUnits;}

private:
	std::uint32_t muiVarId;
	float mfEguMin;
	float mfEguMax;
	IOAddr mtAddr;
	QString mstrUnits;
};


enum RegState{Parado, Arrancando, Parando, Descargando, RegVacio, RegIsla, RegPotencia, RegApertura};




#endif //COMMONHMITYPES_H