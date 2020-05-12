#include "ControlWdg.h"


ControlWdg::ControlWdg(QWidget *parent) : QWidget(parent){
    setupUi(this);
    //lbIndRem->setStyleSheet("QLabel { background-color: rgb(170, 0, 0); }");


    cmdRegTypeP->setChecked(true);
    mRegTypeGrp.addButton(cmdRegTypeP, 0);
    mRegTypeGrp.addButton(cmdRegTypeA, 1);
    connect(&mRegTypeGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::regTypeButtonClicked);
}

ControlWdg::~ControlWdg(){

}


void ControlWdg::regTypeButtonClicked(int id){

}