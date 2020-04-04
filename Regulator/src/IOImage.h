#ifndef IOIMAGE_H
#define IOIMAGE_H
#include <CommonTypes.h>
#include <IDVarList.h>
#include <unordered_map>
#include <string>

//Declaraciones adelantadas
class IOFieldDrv;
class IOSlaveDrv;

class IOImage {
public:
	IOImage(std::string const& strConfigPath, IOFieldDrv & ptFieldDrv, IOSlaveDrv & ptSlaveDrv);
	~IOImage();

	void updateInputs();
	void updateOutputs();
	
	IOVar& operator[](std::uint32_t uiID) { 
		return at(uiID); 
	}
    IOVar const& operator[](std::uint32_t uiID) const { 
    	return at(uiID);
    }

    IOVar& at(std::uint32_t uiID);
    IOVar const& at(std::uint32_t uiID) const;

private:
	IOFieldDrv* mptFieldDrv;
	IOSlaveDrv* mptSlaveDrv;

	IOVar* mFieldIn[FLD_IN_VARS];
	IOVar* mFieldOut[FLD_OUT_VARS];
	IOVar* mSlaveIn[SLV_IN_VARS];
	IOVar* mSlaveOut[SLV_OUT_VARS];

}




#endif