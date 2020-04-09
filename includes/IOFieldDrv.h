#ifndef IOFIELDDRV_H
#define IOFIELDDRV_H
#include <string>
#include <CommonTypes.h>
#include <unordered_map>
#include <functional>

class IOFieldDrv{
public:
	virtual ~IOFieldDrv(){};
	virtual bool init(std::string const& strConfigPath, std::function<void()> const& errorCallB) = 0;
	virtual bool close() = 0;
	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual DrvState getState() const = 0;
	virtual std::string getLastErrorInfo() const = 0;
	
	virtual bool read(IOVar & var) = 0;
	virtual bool write(IOVar const& var) = 0;
	virtual void getVarErrors(std::unordered_map<IOAddr, QState> & mErrors) = 0;
	virtual void updateQState(IOVar & var) = 0;
};



#endif