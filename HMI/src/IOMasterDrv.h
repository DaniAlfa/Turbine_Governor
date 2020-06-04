#ifndef IOMASTERDRV_H
#define IOMASTERDRV_H
#include <string>
#include <functional>
#include "CommonHMITypes.h"
#include <unordered_set>

class IOMasterDrv{
public:
	virtual ~IOMasterDrv(){};
	virtual bool init(std::string const& strConfigPath, std::unordered_set<IOAddr> const& slaveVarsToUpdate, std::unordered_set<IOAddr> const& fieldVarsToUpdate, std::function<void()> const& comErrorCallB, std::function<void()> const& recoveredFromErrorCallB) = 0;
	virtual bool close() = 0;
	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual DrvState getState() const = 0;
	virtual std::string getLastErrorInfo() const = 0;
	
	virtual bool read(VarImage & var, IOAddr const tAddr) = 0;

	virtual bool write(float const val, IOAddr const& tAddr, std::function<void(IOAddr)> const& timeOut, std::uint32_t tWriteTimeOut, std::function<void(IOAddr)> const& writeSuccess  = [](IOAddr){}) = 0;
	virtual bool force(float const fVal, IOAddr const& tAddr, bool bForceBitVal, std::function<void(IOAddr)> const& timeOut, std::uint32_t tWriteTimeOut, std::function<void(IOAddr)>const& writeSuccess  = [](IOAddr){}) = 0;
	
	virtual bool read(std::uint32_t & uiVal, IOAddr tAddt) = 0; 


	virtual void getChangedVars(std::unordered_set<IOAddr> & usChanges) = 0;
	
};



#endif