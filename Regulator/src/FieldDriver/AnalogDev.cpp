#include "AnalogDev.h"


AnalogDev::AnalogDev(EthercatDrv & drv, StatusWord const& status, EtherDevice::BitRepr repr, std::uint8_t uiNumBits) : EtherDevice(drv), mtStatusW(status), mtBitRepr(repr){
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
		case HalfSigned:
			mfMinReprLimit = 0;
			mfMaxReprLimit = p - 1;
		break;
		case Invalid:
		default:
			mfMinReprLimit = 1;
			mfMaxReprLimit = -1;
	}
}


bool AnalogDev::read(IOVar & var){
	std::uint32_t uiReadedVal;
	float fHwMin = var.getHwMin();
	float fHwMax = var.getHwMax();
	if(fHwMin < mfMinReprLimit || fHwMax > mfMaxReprLimit || fHwMax <= fHwMin) return false;
	if(!readDevice(var.getAddr(), uiReadedVal)) return false;


	var.setCurrentVal(scaleValue(var.getEguMin(), var.getEguMax(), fHwMin, fHwMax, convertBytes(uiReadedVal, mtBitRepr, var.getAddr().uiNumBits)));
	var.setTimeS(getMsSinceEpoch());
	var.setQState(getVarQState(mtStatusW));
	return true;
}

bool AnalogDev::write(IOVar const& var){
	float fHwMin = var.getHwMin();
	float fHwMax = var.getHwMax();
	float fEguMin = var.getEguMin();
	float fEguMax = var.getEguMax();
	if(fHwMax <= fHwMin || fEguMax <= fEguMin) return false;
	float fWriteVal = scaleValue(var.getHwMin(), var.getHwMax(), var.getEguMin(), var.getEguMax(), var.getCurrentVal());
	//if(fWriteVal < mfMinReprLimit || fWriteVal > mfMaxReprLimit) return false;

	return writeDevice(var.getAddr(), convertFloat(fWriteVal, var.getAddr().uiNumBits));
}
	
void AnalogDev::updateDevice(IOAddr const& addr){

	//Gestion errores
}


float AnalogDev::scaleValue(float fDestMin, float fDestMax, float fOriginMin, float fOriginMax, float fVal){
	return ((fVal - fOriginMin) / (fOriginMax - fOriginMin)) * (fDestMax - fDestMin) + fDestMin;
}

QState AnalogDev::getVarQState(StatusWord const& stWord){
	if(!StatusWord::StatusWordOk(stWord)) 
		return ComError;
	std::uint32_t statusWord;
	if(!readDevice(stWord.tAddr, statusWord)) return ComError;
	std::uint32_t bitMask = 1 << (stWord.tAddr.uiNumBits - stWord.uiUnderRangeBit - 1);
	if(statusWord & bitMask) return UnderRange;
	bitMask = 1 << (stWord.tAddr.uiNumBits - stWord.uiOverRangeBit - 1);
	if(statusWord & bitMask) return OverRange;
	return OK;
}


bool AnalogDev::parametersOk(StatusWord const& stWord, EtherDevice::BitRepr repr, std::uint8_t uiNumBits){
	return repr != Invalid && uiNumBits % 8 == 0 && uiNumBits != 24 && uiNumBits > 0 && uiNumBits <= 32 && StatusWord::StatusWordOk(stWord);
}