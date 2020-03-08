#ifndef ANALOGDEV_H
#define ANALOGDEV_H
#include <cstdint>
#include <string>
#include "EtherDevice.h"
#include "CommonTypes.h"

//Declaraciones adelantadas
class EthercatDrv;


class AnalogDev : public EtherDevice{
public:
	class StatusWord{
	public:
		IOAddr tAddr;
		std::uint8_t uiUnderRangeBit;
		std::uint8_t uiOverRangeBit;
		static bool StatusWordOk(StatusWord const& stByte){
			return stByte.tAddr.uiNumBits > 0 && stByte.tAddr.uiNumBits <= 16 && stByte.tAddr.uiNumBits % 8 == 0 && 
			stByte.uiUnderRangeBit <= stByte.tAddr.uiNumBits - 1 && stByte.uiUnderRangeBit >= 0 && 
			stByte.uiOverRangeBit <= stByte.tAddr.uiNumBits - 1 && stByte.uiOverRangeBit >= 0;
		}
	};
	enum BitRepr{Signed, Unsigned, HalfUnsigned, Invalid};

	AnalogDev(EthercatDrv & drv, StatusWord const& status, BitRepr repr, std::uint8_t uiNumBits);
	bool read(IOVar & var);
	bool write(IOVar const& var);
	void updateDevice(IOAddr const& addr);

	static bool parametersOk(StatusWord const& status, BitRepr repr, std::uint8_t uiNumBits);
	static AnalogDev::BitRepr getBitRepr(std::string strRepr);

protected:
	StatusWord mtStatusB;
	BitRepr mtBitRepr;

	float mfMinReprLimit,mfMaxReprLimit;

	float scaleValue(float fDestMin, float fDestMax, float fOriginMin, float fOriginMax, float fVal);

	QState getVarQState(StatusWord const& stByte);

};



#endif