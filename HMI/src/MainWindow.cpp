#include "MainWindow.h"
#include "ControlWdg.h"
#include "ControlOptWdg.h"
#include "AlarmsWdg.h"
#include "TurbineViewWdg.h"
#include "TendencyWdg.h"
#include "VarsViewWdg.h"


//#define IMAGE_UPDATE_MILLIS 15
#define IMAGE_UPDATE_MILLIS 50

MainWindow::MainWindow(RegImage & regImg, QWidget *parent) : QMainWindow(parent), mpRegImg(&regImg){
    setupUi(this);
    
    StkWidget->insertWidget(0, new ControlWdg(regImg, this));
    StkWidget->insertWidget(1, new ControlOptWdg(regImg, this));
    StkWidget->insertWidget(2, new TurbineViewWdg(regImg, this));
    StkWidget->insertWidget(3, new AlarmsWdg(regImg, this));
    StkWidget->insertWidget(4, new TendencyWdg(regImg, this));
    StkWidget->insertWidget(5, new VarsViewWdg(this));
    StkWidget->setCurrentIndex(0);

    mMainBtnGrp.addButton(cmdControl, 0);
    mMainBtnGrp.addButton(cmdControlOpt, 1);
    mMainBtnGrp.addButton(cmdTurb, 2);
    mMainBtnGrp.addButton(cmdAlarms, 3);
    mMainBtnGrp.addButton(cmdTenden, 4);
    mMainBtnGrp.addButton(cmdTest, 5);
    cmdControl->setChecked(true);
    connect(&mMainBtnGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &MainWindow::mainButtonClicked);


    connect(&regImg, &RegImage::allVarsUpdated, this, &MainWindow::imageVarsInitialized);
    connect(&regImg, &RegImage::comError, this, &MainWindow::comError);
    connect(&regImg, &RegImage::recoveredFromComError, this, &MainWindow::recoveredFromComError);
    connect(&regImg, &RegImage::writeError, this, &MainWindow::writeError);

   	regImg.start();
   	startTimer(IMAGE_UPDATE_MILLIS);
}

MainWindow::~MainWindow(){
    mpRegImg->stop();
}

void MainWindow::timerEvent(QTimerEvent *){
	mpRegImg->updateImage();
}

void MainWindow::imageVarsInitialized(){
	this->show();
}



void MainWindow::mainButtonClicked(int id){
	StkWidget->setCurrentIndex(id);
}

void MainWindow::comError(){

}
	
void MainWindow::recoveredFromComError(){

}

void MainWindow::writeError(std::uint32_t const varID){

}
