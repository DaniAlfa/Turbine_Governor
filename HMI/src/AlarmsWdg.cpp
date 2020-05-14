#include "AlarmsWdg.h"


AlarmsWdg::AlarmsWdg(QWidget *parent) : QWidget(parent){
    setupUi(this);

/*
    cmdRegTypeP->setChecked(true);
    mRegTypeGrp.addButton(cmdRegTypeP, 0);
    mRegTypeGrp.addButton(cmdRegTypeA, 1);
    */
    connect(&mRegTypeGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &AlarmsWdg::regTypeButtonClicked);

}

AlarmsWdg::~AlarmsWdg(){

}


void AlarmsWdg::regTypeButtonClicked(int id){

}