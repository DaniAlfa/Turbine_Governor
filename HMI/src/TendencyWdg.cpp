#include "TendencyWdg.h"
#include "RTPlotWdg.h"
#include "RegIDS.h"
#include <QColor>
#include <cmath>
#include <QHBoxLayout>

TendencyWdg::TendencyWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg){
    setupUi(this);


    connect(&regImg, &RegImage::allVarsUpdated, this, &TendencyWdg::imageVarsInitialized);
    connect(&regImg, &RegImage::varChanged, this, &TendencyWdg::varChanged);
    mRTMeasurer.moveToThread(&mMeasurerThread);

    RTPlotWdg::PlotConfig plotCnf;
    plotCnf.iMeasureRate = 200;
	plotCnf.tMeasureUnits = RTPlotWdg::TIMEUNITS::MILLIS;
	plotCnf.iDisplayInterval = 60;
	plotCnf.tDisplayUnits = RTPlotWdg::TIMEUNITS::SEC;
	plotCnf.iYNoTics = 10;
	plotCnf.iMaxXNoTics = 12;
	plotCnf.fYMaxVal = 100;
	plotCnf.fYMinVal = 0;
	plotCnf.strAxisYName = "Hz";
	plotCnf.iAxisYPrecission = 1;
	plotCnf.iAxisXPrecission = 0;


    mpSpeedPlot = new RTPlotWdg(mRTMeasurer, plotCnf, this);
    int iNoSSensors = regImg.getNoOfSpeedSensors();
    if(iNoSSensors >= 1){
    	mpSpeedPlot->addPlot("Medida 1", [this](){return this->mumLastMeasures[SE_REG_F1];}, Qt::darkRed);
    	if(iNoSSensors >= 2){
    		mpSpeedPlot->addPlot("Medida 2", [this](){return this->mumLastMeasures[SE_REG_F2];}, Qt::darkGreen);
    		if(iNoSSensors >= 3)
    			mpSpeedPlot->addPlot("Medida 3", [this](){return this->mumLastMeasures[SE_REG_F3];}, Qt::darkBlue);
    	}
    } 
    
    plotCnf.iMeasureRate = 200;
	plotCnf.tMeasureUnits = RTPlotWdg::TIMEUNITS::MILLIS;
	plotCnf.iDisplayInterval = 60;
	plotCnf.tDisplayUnits = RTPlotWdg::TIMEUNITS::SEC;
	plotCnf.iYNoTics = 10;
	plotCnf.iMaxXNoTics = 12;
	float fPWMin = regImg.getVarMin(JE_GEN_P1);
	float fPWMax = regImg.getVarMax(JE_GEN_P1);
	float fPWRange = fPWMax - fPWMin;
	plotCnf.fYMaxVal = fPWMax + ceil(fPWRange*0.15);
	plotCnf.fYMinVal = fPWMin + ceil(fPWRange*0.03);
	plotCnf.strAxisYName = regImg.getVarUnits(JE_GEN_P1);
	plotCnf.iAxisYPrecission = 1;
	plotCnf.iAxisXPrecission = 0;


	mpPWPlot = new RTPlotWdg(mRTMeasurer, plotCnf, this);
	int iNoPWSensors = regImg.getNoOfPowerSensors();
    if(iNoPWSensors >= 1){
    	mpPWPlot->addPlot("Medida 1", [this](){return this->mumLastMeasures[JE_GEN_P1];}, Qt::darkRed);
    	if(iNoPWSensors >= 2){
    		mpPWPlot->addPlot("Medida 2", [this](){return this->mumLastMeasures[JE_GEN_P2];}, Qt::darkGreen);
    	}
    } 

    QHBoxLayout* frecLayout = new QHBoxLayout(frecTab);
    frecLayout->addWidget(mpSpeedPlot);
    frecTab->setLayout(frecLayout);

    QHBoxLayout* pwLayout = new QHBoxLayout(pwTab);
    pwLayout->addWidget(mpPWPlot);
    pwTab->setLayout(pwLayout);

    mMeasurerThread.start();
}

TendencyWdg::~TendencyWdg(){
	QMetaObject::invokeMethod(&mRTMeasurer, "stopMeasures", Qt::BlockingQueuedConnection);
	mMeasurerThread.quit();
}


void TendencyWdg::imageVarsInitialized(){
	disconnect(mpRegImg, &RegImage::allVarsUpdated, this, &TendencyWdg::imageVarsInitialized);
	QMetaObject::invokeMethod(&mRTMeasurer, "startMeasures", Qt::QueuedConnection);
	mpSpeedPlot->startUpdateTimer();
	mpPWPlot->startUpdateTimer();
}

void TendencyWdg::varChanged(VarImage const& var){
	std::uint32_t varID = var.getID();
	switch(varID){
	case JE_GEN_P1: //Medida de potencia 1
	case JE_GEN_P2:
	case SE_REG_F1: //Medida de frecuencia 1
	case SE_REG_F2: //Medida de frecuencia 2
	case SE_REG_F3: //Medida de frecuencia 3
		mumLastMeasures[varID] = var.getCurrentVal();
		break;
	}
}
