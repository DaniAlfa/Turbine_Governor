#include "PositionCtrlWdg.h"

PositionCtrlWdg::PositionCtrlWdg(QString const& strTitle, std::uint32_t uiReadID, std::uint32_t uiWriteID, RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg), muiReadID(uiReadID), muiWriteID(uiWriteID){
    setupUi(this);

    lbTitle->setText(strTitle);
    connect(mpRegImg, &RegImage::varChanged, this, &PositionCtrlWdg::varChanged);
}



void PositionCtrlWdg::varChanged(VarImage const& var){
	std::uint32_t uiID = var.getID();
	if(uiID == muiReadID){
		txtOp->setText(QString::number(var.getCurrentVal(), 'f', 1) + "%");
	}
	else if(uiID == muiWriteID){
		txtVel->setText(QString::number(var.getCurrentVal(), 'f', 1));
		mfLastSpeedVal = var.getCurrentVal();
	}
}


void PositionCtrlWdg::setManual(bool bManual){
	mbManual = bManual;
	cmdMore->setEnabled(mbManual);
	cmdMMore->setEnabled(mbManual);
	cmdLess->setEnabled(mbManual);
	cmdLLess->setEnabled(mbManual);
	cmdStop->setEnabled(mbManual);
}

void PositionCtrlWdg::setWriteLimits(float fMin, float fMax){
	mfMinLimit = fMin;
	mfMaxLimit = fMax;
}


void PositionCtrlWdg::on_cmdMore_clicked(){
	float fWriteVal = mfLastSpeedVal + 1;
	if(fWriteVal > mfMaxLimit) fWriteVal = mfMaxLimit;
	mpRegImg->writeVal(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdMMore_clicked(){
	float fWriteVal = mfLastSpeedVal + 5;
	if(fWriteVal > mfMaxLimit) fWriteVal = mfMaxLimit;
	mpRegImg->writeVal(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdLess_clicked(){
	float fWriteVal = mfLastSpeedVal - 1;
	if(fWriteVal < mfMinLimit) fWriteVal = mfMinLimit;
	mpRegImg->writeVal(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdLLess_clicked(){
	float fWriteVal = mfLastSpeedVal - 5;
	if(fWriteVal < mfMinLimit) fWriteVal = mfMinLimit;
	mpRegImg->writeVal(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdStop_clicked(){
	mpRegImg->writeVal(muiWriteID, 0);
}