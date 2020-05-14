#include "MainWindow.h"
#include "ControlWdg.h"
#include "ControlOptWdg.h"
#include "AlarmsWdg.h"
#include "TurbineViewWdg.h"
#include "TendencyWdg.h"
#include "VarsViewWdg.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    setupUi(this);
    
    StkWidget->insertWidget(0, new ControlWdg(this));
    StkWidget->insertWidget(1, new ControlOptWdg(this));
    StkWidget->insertWidget(2, new TurbineViewWdg(this));
    StkWidget->insertWidget(3, new AlarmsWdg(this));
    StkWidget->insertWidget(4, new TendencyWdg(this));
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
}

MainWindow::~MainWindow(){
    
}



void MainWindow::mainButtonClicked(int id){
	StkWidget->setCurrentIndex(id);
}