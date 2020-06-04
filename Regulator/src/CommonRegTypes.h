#ifndef COMMONREGTYPES_H
#define COMMONREGTYPES_H
#include <CommonTypes.h>

class IOVar{
public:
	IOVar(std::uint32_t uiVarId, IOAddr tAddr) : muiVarId(uiVarId), mtAddr(tAddr), mtQState(OK), mbForcedVal(0) {}

	std::uint32_t getID() const {return muiVarId;}
	IOAddr getAddr() const { return mtAddr; }
	void setCurrentVal(float fCurrentVal) { mfTrueVal = fCurrentVal; }
	float getCurrentVal() const {return (mbForcedVal) ? mfForcedVal : mfTrueVal;}
	float getTrueVal() const {return mfTrueVal;}
	float getForcedVal() const {return mfForcedVal;}
	void setForcedVal(float fForcedVal){mfForcedVal = fForcedVal;}
	void setTimeS(std::int64_t const& iTimeS) { miTimeS = iTimeS;}
	std::int64_t getTimeS() const {return miTimeS;}
	void setPulseInfo(double const& dDur, std::int64_t const& iTimeS){ 
		mdPulseDur = dDur;
		miPulseTimeS = iTimeS;
	}
	double getPulseDur() const {return mdPulseDur;}
	std::int64_t getPulseTimeS() const {return miPulseTimeS;}
	float getHwMin() const {return mfHWMin;}
	float getHwMax() const {return mfHWMax;}
	void setHwMin(float fVal) {mfHWMin = fVal;}
	void setHwMax(float fVal) {mfHWMax = fVal;}
	float getEguMin() const {return mfEguMin;}
	float getEguMax() const {return mfEguMax;}
	void setEguMin(float fVal) {mfEguMin = fVal;}
	void setEguMax(float fVal) {mfEguMax = fVal;}
	void setQState(QuState tState){mtQState = tState;}
	QuState getQState() const {return (mbForcedVal) ? OK : mtQState;}
	void setForced(bool bForced) {mbForcedVal = bForced;}
	bool getForced() const {return mbForcedVal;}
	

private:
	std::uint32_t muiVarId;
	std::int64_t miTimeS;
	float mfTrueVal;
	float mfForcedVal;
	float mfEguMin;
	float mfEguMax;
	float mfHWMin;
	float mfHWMax;
	IOAddr mtAddr;
	double mdPulseDur;
	std::int64_t miPulseTimeS;
	QuState mtQState;
	bool mbForcedVal;
};




#endif //COMMONREGTYPES_H 
