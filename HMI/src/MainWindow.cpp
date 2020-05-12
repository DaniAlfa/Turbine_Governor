#include "MainWindow.h"
#include "ControlWdg.h"
#include "ControlOptWdg.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    setupUi(this);
    
    StkWidget->insertWidget(0, new ControlWdg(this));
    StkWidget->insertWidget(1, new ControlOptWdg(this));
    StkWidget->setCurrentIndex(0);

    mMainBtnGrp.addButton(cmdControl, 0);
    mMainBtnGrp.addButton(cmdControlOpt, 1);
    cmdControl->setChecked(true);

    connect(&mMainBtnGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &MainWindow::mainButtonClicked);
}

MainWindow::~MainWindow(){
    
}



void MainWindow::mainButtonClicked(int id){
	StkWidget->setCurrentIndex(id);
}