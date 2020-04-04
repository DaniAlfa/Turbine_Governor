#include "IOImage.h"

IOImage::IOImage(std::string const& strConfigPath, IOFieldDrv & ptFieldDrv, IOSlaveDrv & ptSlaveDrv) : mptFieldDrv(&ptFieldDrv), mptSlaveDrv(&ptSlaveDrv){

}

IOImage::~IOImage(){
	for(int i = 0; i < FLD_IN_VARS; ++i)
		delete mFieldIn[i];
	for(int i = 0; i < SLV_IN_VARS; ++i)
		delete mSlaveIn[i];
	for(int i = 0; i < SLV_OUT_VARS; ++i)
		delete mSlaveOut[i];
	for(int i = 0; i < FLD_OUT_VARS; ++i)
		delete mFieldOut[i];
}

void IOImage::updateInputs(){
	for(int i = 0; i < FLD_IN_VARS; ++i){
		mptFieldDrv->read(*mFieldIn[i]);
		mptSlaveDrv->updateFieldVar(*mFieldIn[i]);
	}
	for(int i = 0; i < SLV_IN_VARS; ++i){
		mptSlaveDrv->read(*mSlaveIn[i]);
	}
}

void IOImage::updateOutputs(){
	for(int i = 0; i < SLV_OUT_VARS; ++i){
		mptSlaveDrv->write(*mSlaveOut[i]);
	}
	for(int i = 0; i < FLD_OUT_VARS; ++i){
		mptSlaveDrv->updateFieldVar(*mFieldOut[i]);
		mptFieldDrv->write(*mFieldOut[i]);
	}
}

IOVar& IOImage::at(std::uint32_t uiID){
	if(uiID >= FLD_IN_LOW_RANGE && uiID <= FLD_IN_HIGH_RANGE){
		return *(mFieldIn[uiID - FLD_IN_LOW_RANGE]);
	}
	else if(uiID >= FLD_OUT_LOW_RANGE && uiID <= FLD_OUT_HIGH_RANGE){
		return *(mFieldOut[uiID - FLD_OUT_LOW_RANGE]);
	}
	if(uiID >= SLV_IN_LOW_RANGE && uiID <= SLV_IN_HIGH_RANGE){
		return *(mSlaveIn[uiID - SLV_IN_LOW_RANGE]);
	}
	return *(mFieldOut[uiID - SLV_OUT_LOW_RANGE]);
}

IOVar const& IOImage::at(std::uint32_t uiID) const{
	if(uiID >= FLD_IN_LOW_RANGE && uiID <= FLD_IN_HIGH_RANGE){
		return *(mFieldIn[uiID - FLD_IN_LOW_RANGE]);
	}
	else if(uiID >= FLD_OUT_LOW_RANGE && uiID <= FLD_OUT_HIGH_RANGE){
		return *(mFieldOut[uiID - FLD_OUT_LOW_RANGE]);
	}
	if(uiID >= SLV_IN_LOW_RANGE && uiID <= SLV_IN_HIGH_RANGE){
		return *(mSlaveIn[uiID - SLV_IN_LOW_RANGE]);
	}
	return *(mFieldOut[uiID - SLV_OUT_LOW_RANGE]);
}