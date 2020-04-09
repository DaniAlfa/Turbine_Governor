#include "IOImage.h"

IOImage::IOImage(std::string const& strConfigPath, IOFieldDrv & ptFieldDrv, IOSlaveDrv & ptSlaveDrv) : mptFieldDrv(&ptFieldDrv), mptSlaveDrv(&ptSlaveDrv){

}

IOImage::~IOImage(){
	for(int i = 0; i < FLD_IN_VARS; ++i){
		if(mFieldIn[i] != nullptr) delete mFieldIn[i];
	}
	for(int i = 0; i < SLV_IN_VARS; ++i){
		if(mSlaveIn[i] != nullptr) delete mSlaveIn[i];
	}
	for(int i = 0; i < SLV_OUT_VARS; ++i){
		if(mSlaveOut[i] != nullptr) delete mSlaveOut[i];
	}
	for(int i = 0; i < FLD_OUT_VARS; ++i){
		if(mFieldOut[i] != nullptr) delete mFieldOut[i];
	}
}

void IOImage::updateInputs(){
	for(int i = 0; i < FLD_IN_VARS; ++i){
		if(mFieldIn[i] == nullptr) continue;
		mptFieldDrv->read(*mFieldIn[i]);
		mptSlaveDrv->updateFieldVar(*mFieldIn[i]);
	}
	for(int i = 0; i < SLV_IN_VARS; ++i){
		if(mSlaveIn[i] == nullptr) continue;
		mptSlaveDrv->read(*mSlaveIn[i]);
	}
	for(int i = 0; i < FLD_OUT_VARS; ++i){ //La actualizacion de entradas incluye el estado de calidad de las variables de campo de salida para aplicar el control
		if(mFieldOut[i] == nullptr) continue;
		mptFieldDrv->updateQState(*mFieldOut[i]);
	}
}

void IOImage::updateOutputs(){
	for(int i = 0; i < SLV_OUT_VARS; ++i){
		if(mSlaveOut[i] == nullptr) continue;
		mptSlaveDrv->write(*mSlaveOut[i]);
	}
	for(int i = 0; i < FLD_OUT_VARS; ++i){
		if(mFieldOut[i] == nullptr) continue;
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

DrvState IOImage::getFieldDrvState() const{
	return mptFieldDrv->getState();
}

DrvState IOImage::getSlaveDrvState() const{
	return mptSlaveDrv->getState();
}

void IOImage::getFldErrors(std::unordered_map<std::uint32_t, QState> & mErrors){
	std::unordered_map<IOAddr, QState> mFldErrors;
	mptFieldDrv->getVarErrors(mFldErrors);
	for(auto it : mFldErrors){
		auto it2 = mMFieldAddrIndex.find(it->first);
		if(it2 != mMFieldAddrIndex.end()){
			mErrors.insert({it2->second->getID(), it->second});
		}
	}
}