#ifndef COMMONHMITYPES_H 
#define COMMONHMITYPES_H

#include <string>
#include <CommonTypes.h>
Q_DECLARE_METATYPE(IOAddr)

class VarImage{
public:
	VarImage(std::uint32_t uiVarId) : muiVarId(uiVarId), mtQState(OK), mbForcedVal(0) {}

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
	void setQState(QState tState){mtQState = tState;}
	QState getQState() const {return mtQState;}
	void setForced(bool bForced) {mbForcedVal = bForced;}
	bool getForced() const {return mbForcedVal;}
	void setUnits(std::string strUnits) {mstrUnits = strUnits;}
	std::string getUnits() const {return mstrUnits;}

private:
	std::uint32_t muiVarId;
	std::int64_t miTimeS;
	float mfTrueVal;
	float mfForcedVal;
	float mfEguMin;
	float mfEguMax;
	QState mtQState;
	bool mbForcedVal;
	std::string mstrUnits;
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
	void setUnits(std::string strUnits) {mstrUnits = strUnits;}
	std::string getUnits() const {return mstrUnits;}

private:
	std::uint32_t muiVarId;
	float mfEguMin;
	float mfEguMax;
	IOAddr mtAddr;
	std::string mstrUnits;
};


enum RegState{Parado, Arrancando, Parando, Descargando, RegVacio, RegIsla, RegPotencia, RegApertura};




#endif //COMMONHMITYPES_H