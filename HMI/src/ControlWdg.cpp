#include "ControlWdg.h"


ControlWdg::ControlWdg(QWidget *parent) : QWidget(parent){
    setupUi(this);
    //lbIndRem->setStyleSheet("QLabel { background-color: rgb(170, 0, 0); }");


    cmdRegTypeP->setChecked(true);
    mRegTypeGrp.addButton(cmdRegTypeP, 0);
    mRegTypeGrp.addButton(cmdRegTypeA, 1);
    connect(&mRegTypeGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::regTypeButtonClicked);

    cmdF1->setChecked(true);
    mFrecWdgGrp.addButton(cmdF1, 0);
    mFrecWdgGrp.addButton(cmdF2, 1);
	mFrecWdgGrp.addButton(cmdF3, 2);
	connect(&mFrecWdgGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::frecButtonClicked);

	cmdN1->setChecked(true);
    mSpeedWdgGrp.addButton(cmdN1, 0);
    mSpeedWdgGrp.addButton(cmdN2, 1);
    mSpeedWdgGrp.addButton(cmdN3, 2);
    connect(&mSpeedWdgGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::speedTypeButtonClicked);

    cmdP1->setChecked(true);
    mPwWdgGrp.addButton(cmdP1, 0);
    mPwWdgGrp.addButton(cmdP2, 1);
    connect(&mPwWdgGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::pwButtonClicked);

}

ControlWdg::~ControlWdg(){

}


void ControlWdg::regTypeButtonClicked(int id){

}


void ControlWdg::frecButtonClicked(int id){
	StkF->setCurrentIndex(id);
}

void ControlWdg::speedTypeButtonClicked(int id){
	StkN->setCurrentIndex(id);
}

void ControlWdg::pwButtonClicked(int id){
	StkP->setCurrentIndex(id);
}