#include "PositionCtrlWdg.h"
#include "RegIDS.h"

PositionCtrlWdg::PositionCtrlWdg(QString const& strTitle, std::uint32_t uiReadID, std::uint32_t uiWriteID, RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg), muiReadID(uiReadID), muiWriteID(uiWriteID){
    setupUi(this);

    setManual(false);
    lbTitle->setText(strTitle);
    //mfMinLimit = mpRegImg->getVarMin(uiWriteID);
	//mfMaxLimit = mpRegImg->getVarMax(uiWriteID);
	mfMinLimit = -10;
	mfMaxLimit = 10;
	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
	else if(uiID == Y_REG_AUTO){
		setManual(var.getCurrentVal() == 0);
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


void PositionCtrlWdg::on_cmdMore_clicked(){
	float fWriteVal = mfLastSpeedVal + 1;
	if(fWriteVal > mfMaxLimit) fWriteVal = mfMaxLimit;
	mpRegImg->writeVar(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdMMore_clicked(){
	float fWriteVal = mfLastSpeedVal + 5;
	if(fWriteVal > mfMaxLimit) fWriteVal = mfMaxLimit;
	mpRegImg->writeVar(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdLess_clicked(){
	float fWriteVal = mfLastSpeedVal - 1;
	if(fWriteVal < mfMinLimit) fWriteVal = mfMinLimit;
	mpRegImg->writeVar(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdLLess_clicked(){
	float fWriteVal = mfLastSpeedVal - 5;
	if(fWriteVal < mfMinLimit) fWriteVal = mfMinLimit;
	mpRegImg->writeVar(muiWriteID, fWriteVal);
}

void PositionCtrlWdg::on_cmdStop_clicked(){
	mpRegImg->writeVar(muiWriteID, 0);
}


QSize PositionCtrlWdg::sizeHint() const{
	return QSize(400, 237);
}