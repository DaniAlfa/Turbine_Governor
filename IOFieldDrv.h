#ifndef IOFIELDDRV_H
#define IOFIELDDRV_H
#include <string>

//Declaraciones adelantadas
class IOVar;
enum DrvState;

class IOFieldDrv{
public:
	virtual bool init(std::string const& strIfname, std::string & strError) = 0;
	virtual bool close() = 0;
	virtual bool start() = 0;
	virtual bool stop() = 0;
	virtual DrvState getState() const = 0;
	
	virtual bool read(IOVar & var) = 0;
	virtual bool write(IOVar const& var) = 0;
	
};



#endif