#ifndef ETHERDEVICE_H
#define ETHERDEVICE_H
#include <cstdint>
#include <string>
#include <CommonTypes.h>

//Declaraciones anticipadas
class EthercatDrv;

class EtherDevice{
public:
	EtherDevice(EthercatDrv & drv);
	virtual ~EtherDevice();

	virtual bool read(IOVar & var) = 0;
	virtual bool write(IOVar const& var) = 0;
	virtual void updateDevice(IOAddr const& addr){}
	
	enum BitRepr{Signed, Unsigned, HalfSigned, Invalid};
	static EtherDevice::BitRepr getBitRepr(std::string strRepr);

private:
	EthercatDrv* mtDrv;

protected:
	bool mbInError;
	QState mtLastQState;

	bool writeDevice(IOAddr const& addr, std::uint32_t uiVal);
	bool writeDeviceSync(IOAddr const& addr, std::uint32_t uiVal, std::uint32_t tTimeOut);
	bool readDevice(IOAddr const& addr, std::uint32_t & uiVal);

	bool isModuleOk(IOAddr const& addr) const;
	void newVarError(IOAddr const& addr, QState eState);
	void clearVarError(IOAddr const& addr);
	
	static std::int64_t getMsSinceEpoch();

	//Convierte los bytes de uiVal en un numero float en la representacion adecuada
	static float convertBytes(std::uint32_t uiVal, EtherDevice::BitRepr repr, std::uint8_t uiNumBits); 
	//Funcion inversa a la anterior (En limites de 32 bits puede no dar resultado exacto) 
	static std::uint32_t convertFloat(float fVal, std::uint8_t uiNumBits);

};





#endif
