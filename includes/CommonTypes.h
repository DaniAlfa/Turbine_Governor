#ifndef IOIMAGE_H
#define IOIMAGE_H
#include <cstdint>
#include <functional>

enum DrvState{Running, Stopped, COMError, VarError, UnInit};


enum QState{ OK = 0, OverRange = 1, UnderRange = 2, ComError = 3};

class IOAddr{
public:

	std::uint8_t uiHeader;
	std::uint8_t uiModule;
	std::uint8_t uiChannel;
	std::uint8_t uiNumBits;

	bool operator==(IOAddr const& other) const { 
		return other.uiHeader == uiHeader && other.uiModule == uiModule && other.uiChannel == uiChannel && other.uiNumBits == uiNumBits;
	}
};

namespace std {
    template<> 
    struct hash<IOAddr> {
        std::size_t operator()(IOAddr const& addr) const noexcept {
            return ((((hash<uint8_t>()(addr.uiHeader) ^ (hash<uint8_t>()(addr.uiModule) << 1)) >> 1) ^ (hash<uint8_t>()(addr.uiChannel) << 1)) >> 1) 
            ^ (hash<uint8_t>()(addr.uiNumBits) << 1);
        }
    };
}

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
	void setQState(QState tState){mtQState = tState;}
	QState getQState() const {return (mbForcedVal) ? OK : mtQState;}
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
	QState mtQState;
	bool mbForcedVal;
};


class RegVar{
public:
	RegVar(std::uint32_t uiVarId, IOAddr tAddr) : muiVarId(uiVarId), mtAddr(tAddr), mtQState(OK), mbForcedVal(0) {}

	std::uint32_t getID() const {return muiVarId;}
	IOAddr getAddr() const { return mtAddr; }
	void setCurrentVal(float fCurrentVal) { mfTrueVal = fCurrentVal; }
	float getCurrentVal() const {return (mbForcedVal) ? mfForcedVal : mfTrueVal;}
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
	IOAddr mtAddr;
	QState mtQState;
	bool mbForcedVal;
	std::string mstrUnits;
};

class RegWOVar{
public:
	RegWOVar(std::uint32_t uiVarId, IOAddr tAddr) : muiVarId(uiVarId), mtAddr(tAddr) {}

	std::uint32_t getID() const {return muiVarId;}
	IOAddr getAddr() const { return mtAddr; }
	float getEguMin() const {return mfEguMin;}
	float getEguMax() const {return mfEguMax;}
	void setEguMin(float fVal) {mfEguMin = fVal;}
	void setEguMax(float fVal) {mfEguMax = fVal;}
	

private:
	std::uint32_t muiVarId;
	float mfEguMin;
	float mfEguMax;
	IOAddr mtAddr;
};

#endif
