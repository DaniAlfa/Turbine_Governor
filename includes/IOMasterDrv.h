#ifndef IOMASTERDRV_H
#define IOMASTERDRV_H
#include <string>
#include <functional>
#include <CommonTypes.h>

class IOMasterDrv{
public:
	virtual ~IOMasterDrv(){}
	virtual bool init(std::string const& strConfigPath, std::function<void(IOAddr)> const& writeTimeOutCallB, std::function<void()> const& comErrorCallB) = 0;
	virtual bool close() = 0;
	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual DrvState getState() const = 0;
	virtual std::string getLastErrorInfo() const = 0;
	
	virtual bool read(RegVar & var) = 0;
	virtual bool write(RegVar const& var, std::uint32_t tWriteTimeOut) = 0;
	virtual bool force(RegVar const& var, std::uint32_t tWriteTimeOut) = 0;
	
};



#endif