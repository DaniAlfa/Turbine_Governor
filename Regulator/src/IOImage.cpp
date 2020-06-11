#include "IOImage.h"
#include "AlarmManager.h"
#include "IOFieldDrv.h"
#include "IOSlaveDrv.h"

IOImage::IOImage(IOFieldDrv & ptFieldDrv, IOSlaveDrv & ptSlaveDrv) : mptFieldDrv(&ptFieldDrv), mptSlaveDrv(&ptSlaveDrv), mpAlMan(nullptr){

}

bool IOImage::init(std::string const& strConfigPath){
	for(int i = 0; i < FLD_IN_VARS; ++i){
		mFieldIn[i] = nullptr;
	}
	for(int i = 0; i < SLV_IN_VARS; ++i){
		mSlaveIn[i] = nullptr;
	}
	for(int i = 0; i < SLV_OUT_VARS; ++i){
		mSlaveOut[i] = nullptr;
	}
	for(int i = 0; i < FLD_OUT_VARS; ++i){
		mFieldOut[i] = nullptr;
	}

	IOAddr tAddr;
	tAddr.uiChannel = 39;
	IOVar* ordenArranque = new IOVar(2000000000, tAddr);
	ordenArranque->setCurrentVal(0);
	mSlaveIn[2000000000 - SLV_IN_LOW_RANGE] = ordenArranque;
	mptSlaveDrv->write(*ordenArranque);

	tAddr.uiChannel = 47;
	IOVar* confirCons = new IOVar(2000000008, tAddr);
	confirCons->setCurrentVal(0);
	mSlaveIn[2000000008 - SLV_IN_LOW_RANGE] = confirCons;
	mptSlaveDrv->write(*confirCons);

	tAddr.uiChannel = 48;
	IOVar* cambioRP = new IOVar(2000000009, tAddr);
	cambioRP->setCurrentVal(0);
	mSlaveIn[2000000009 - SLV_IN_LOW_RANGE] = cambioRP;
	mptSlaveDrv->write(*cambioRP);

	tAddr.uiChannel = 49;
	IOVar* cambioRA = new IOVar(2000000010, tAddr);
	cambioRA->setCurrentVal(0);
	mSlaveIn[2000000010 - SLV_IN_LOW_RANGE] = cambioRA;
	mptSlaveDrv->write(*cambioRA);

	tAddr.uiChannel = 68;
	IOVar* spP = new IOVar(2000000012, tAddr);
	spP->setCurrentVal(0);
	mSlaveIn[2000000012 - SLV_IN_LOW_RANGE] = spP;
	mptSlaveDrv->write(*spP);

	tAddr.uiChannel = 69;
	IOVar* spA = new IOVar(2000000013, tAddr);
	spA->setCurrentVal(0);
	mSlaveIn[2000000013 - SLV_IN_LOW_RANGE] = spA;
	mptSlaveDrv->write(*spA);

	tAddr.uiChannel = 129;
	IOVar* turNo = new IOVar(1000000007, tAddr);
	turNo->setCurrentVal(0);
	mFieldOut[1000000007 - FLD_OUT_LOW_RANGE] = turNo;
	mptSlaveDrv->write(*turNo);

	tAddr.uiChannel = 128;
	IOVar* turFR = new IOVar(1000000006, tAddr);
	turFR->setCurrentVal(0);
	mFieldOut[1000000006 - FLD_OUT_LOW_RANGE] = turFR;
	mptSlaveDrv->write(*turFR);

	tAddr.uiChannel = 127;
	IOVar* turEXC = new IOVar(1000000005, tAddr);
	turEXC->setCurrentVal(0);
	mFieldOut[1000000005 - FLD_OUT_LOW_RANGE] = turEXC;
	mptSlaveDrv->write(*turEXC);

	tAddr.uiChannel = 126;
	IOVar* turSyn = new IOVar(1000000004, tAddr);
	turSyn->setCurrentVal(0);
	mFieldOut[1000000004 - FLD_OUT_LOW_RANGE] = turSyn;
	mptSlaveDrv->write(*turSyn);

	tAddr.uiChannel = 125;
	IOVar* turHovs = new IOVar(1000000003, tAddr);
	turHovs->setCurrentVal(1);
	mFieldOut[1000000003 - FLD_OUT_LOW_RANGE] = turHovs;
	mptSlaveDrv->write(*turHovs);

	tAddr.uiChannel = 124;
	IOVar* turLovs = new IOVar(1000000002, tAddr);
	turLovs->setCurrentVal(1);
	mFieldOut[1000000002 - FLD_OUT_LOW_RANGE] = turLovs;
	mptSlaveDrv->write(*turLovs);

	tAddr.uiChannel = 135;
	IOVar* regLoc = new IOVar(1000000013, tAddr);
	regLoc->setCurrentVal(0);
	mFieldOut[1000000013 - FLD_OUT_LOW_RANGE] = regLoc;
	mptSlaveDrv->write(*regLoc);

	tAddr.uiChannel = 139;
	IOVar* consP = new IOVar(1000000017, tAddr);
	consP->setCurrentVal(0);
	mFieldOut[1000000017 - FLD_OUT_LOW_RANGE] = consP;
	mptSlaveDrv->write(*consP);

	tAddr.uiChannel = 140;
	IOVar* consO = new IOVar(1000000018, tAddr);
	consO->setCurrentVal(0);
	mFieldOut[1000000018 - FLD_OUT_LOW_RANGE] = consO;
	mptSlaveDrv->write(*consO);

	tAddr.uiChannel = 141;
	IOVar* consF = new IOVar(1000000019, tAddr);
	consF->setCurrentVal(0);
	mFieldOut[1000000019 - FLD_OUT_LOW_RANGE] = consF;
	mptSlaveDrv->write(*consF);

	tAddr.uiChannel = 148;
	IOVar* turbSt = new IOVar(3000000000, tAddr);
	turbSt->setCurrentVal(0);
	mSlaveOut[3000000000 - SLV_OUT_LOW_RANGE] = turbSt;
	mptSlaveDrv->write(*turbSt);

	tAddr.uiChannel = 149;
	IOVar* medf1 = new IOVar(3000000001, tAddr);
	medf1->setCurrentVal(0);
	mSlaveOut[3000000001 - SLV_OUT_LOW_RANGE] = medf1;
	mptSlaveDrv->write(*medf1);

	tAddr.uiChannel = 150;
	IOVar* medf2 = new IOVar(3000000002, tAddr);
	medf2->setCurrentVal(0);
	mSlaveOut[3000000002 - SLV_OUT_LOW_RANGE] = medf2;
	mptSlaveDrv->write(*medf2);

	tAddr.uiChannel = 151;
	IOVar* medf3 = new IOVar(3000000003, tAddr);
	medf3->setCurrentVal(0);
	mSlaveOut[3000000003 - SLV_OUT_LOW_RANGE] = medf3;
	mptSlaveDrv->write(*medf3);
	return true;
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
	if(mpAlMan != nullptr){
		std::uint32_t const* mpLogicErrors = mpAlMan->getLogicErrors();
		for(std::uint32_t i = 0; i < mpAlMan->getNoLogicErrors(); ++i){
			IOAddr tAddr;
			tAddr.uiChannel = i+1;
			mptSlaveDrv->writeRO(mpLogicErrors[i], tAddr);
		}
		std::uint32_t const* mpFieldStInts = mpAlMan->getFieldStInts();
		for(std::uint32_t i = 0; i < mpAlMan->getNoFieldStInts(); ++i){
			IOAddr tAddr;
			tAddr.uiChannel = mpAlMan->getNoLogicErrors() + i + 1;
			mptSlaveDrv->writeRO(mpFieldStInts[i], tAddr);
		}
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
	return *(mSlaveOut[uiID - SLV_OUT_LOW_RANGE]);
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
	return *(mSlaveOut[uiID - SLV_OUT_LOW_RANGE]);
}

DrvState IOImage::getFieldDrvState() const{
	return mptFieldDrv->getState();
}

DrvState IOImage::getSlaveDrvState() const{
	return mptSlaveDrv->getState();
}

void IOImage::getFldErrors(std::unordered_map<std::uint32_t, QuState> & mErrors){
	std::unordered_map<IOAddr, QuState> mFldErrors;
	mptFieldDrv->getVarErrors(mFldErrors);
	for(auto it : mFldErrors){
		auto it2 = mMFieldAddrIndex.find(it.first);
		if(it2 != mMFieldAddrIndex.end()){
			mErrors.insert({it2->second->getID(), it.second});
		}
	}
}

void IOImage::setAlarmManager(AlarmManager & alMan){
	mpAlMan = &alMan;
}