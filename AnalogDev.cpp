#include "AnalogDev.h"


AnalogDev::AnalogDev(EthercatDrv & drv, StatusWord const& status, BitRepr repr, std::uint8_t uiNumBits) : EtherDevice(drv), mtStatusB(status), mtBitRepr(repr){
	float p = (1 << (uiNumBits - 1));
	switch(repr){
		case Unsigned:
			mfMinReprLimit = 0;
			mfMaxReprLimit = (1 << uiNumBits) - 1;
		break;
		case Signed:
			mfMinReprLimit = -p;
			mfMaxReprLimit = p - 1;
		break;
		case HalfUnsigned:
			mfMinReprLimit = 0;
			mfMaxReprLimit = (1 << (uiNumBits -1)) - 1;
		break;
		case Invalid:
		default:
			mfMinReprLimit = 1;
			mfMaxReprLimit = -1;
	}
}



bool AnalogDev::read(IOVar & var){
	std::int32_t readedVal;
	if(!readDevice(var.getAddr(), readedVal)) return false;

	var.setCurrentVal(scaleValue(var.getHwMin(), var.getHwMax(), mfMinReprLimit, mfMaxReprLimit, readedVal));
	var.setTimeS(getMsSinceEpoch());
	var.setQState(getVarQState(mtStatusB));
	return true;
}

bool AnalogDev::write(IOVar const& var){
	return writeDevice(var.getAddr(), scaleValue(mfMinReprLimit, mfMaxReprLimit, var.getHwMin(), var.getHwMax(), var.getCurrentVal()));
}
	
void AnalogDev::updateDevice(IOAddr const& addr){

}



float AnalogDev::scaleValue(float fDestMin, float fDestMax, float fOriginMin, float fOriginMax, float fVal){
	return ((fVal - fOriginMin) / (fOriginMax - fOriginMin)) * (fDestMax - fDestMin) + fDestMin;
}

QState AnalogDev::getVarQState(StatusWord const& stByte){
	if(!StatusWord::StatusWordOk(stByte)) 
		return ComError;
	std::int32_t readedVal;
	if(!readDevice(stByte.tAddr, readedVal)) return ComError;
	std::uint16_t statusWord = readedVal;
	std::uint16_t bitMask = 1 << (stByte.tAddr.uiNumBits - stByte.uiUnderRangeBit - 1);
	if(statusWord & bitMask) return UnderRange;
	bitMask = 1 << (stByte.tAddr.uiNumBits - stByte.uiOverRangeBit - 1);
	if(statusWord & bitMask) return OverRange;
	return OK;
}


bool AnalogDev::parametersOk(StatusWord const& stByte, BitRepr repr, std::uint8_t uiNumBits){
	return repr != Invalid && uiNumBits % 8 == 0 && uiNumBits != 24 && uiNumBits > 0 && uiNumBits <= 32 && StatusWord::StatusWordOk(stByte);
}

AnalogDev::BitRepr AnalogDev::getBitRepr(std::string strRepr){
	if(strRepr == "Unsigned") return Unsigned;
	else if(strRepr == "HalfUnsigned") return HalfUnsigned;
	else if(strRepr == "Signed") return Signed;
	return Invalid;
}