#include "ControlWdg.h"
#include "RegIDS.h"
#include <cstdint>

ControlWdg::ControlWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg){
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


    connect(mpRegImg, &RegImage::varChanged, this, &ControlWdg::varChanged);
}

ControlWdg::~ControlWdg(){

}

void ControlWdg::varChanged(VarImage const& var){
	std::uint32_t uiID = var.getID();
	float fCurrVal = var.getCurrentVal();
	bool bCurrVal = ((int)fCurrVal);
	switch(uiID){
	case Y_REG_STATE: //Estado del regulador
		setRegState(static_cast<RegState>((unsigned)fCurrVal));
		break;
	case SIS_TUR_N0: //Indicador turbina parada
		changeLabelProperty(lbIndN0, "state", (bCurrVal ? "on" : "no"));
		break;
	case SIS_TUR_BRK: //Indicador turbina de velocidad de frenos
		changeLabelProperty(lbIndBr, "state", (bCurrVal ? "on" : "no"));
		break;
	case SIS_TUR_EXC: //Indicador turbina de velocidad de excitacion
		changeLabelProperty(lbIndExc, "state", (bCurrVal ? "on" : "no"));
		break;
	case SIS_TUR_SYNC: //Indicador turbina de velocidad de sincronizacion
		changeLabelProperty(lbIndSync, "state", (bCurrVal ? "on" : "no"));
		break;
	case SAH_TUR_OVS: //Indicador turbina de sobrevelocidad baja
		changeLabelProperty(lbIndLovs, "state", (!bCurrVal ? "off" : "no"));
		break;
	case SAHH_TUR_OVS: //Indicador turbina de sobrevelocidad alta
		changeLabelProperty(lbIndHovs, "state", (!bCurrVal ? "off" : "no"));
		break;
	case ZSC_GEN_LB: //Interruptor de Linea
		changeLabelProperty(lbIndLineB, "state", (bCurrVal ? "on" : "off"));
		break;
	case ZSC_GEN_GB: //Interruptor de generador
		changeLabelProperty(lbIndGenB, "state", (bCurrVal ? "on" : "off"));
		break;
	case Y_REG_LOC: //Regulador en local
		if(bCurrVal){ //local
			changeLabelProperty(lbIndLoc, "state", "on");
			changeLabelProperty(lbIndRem, "state", "no");
		}else{
			changeLabelProperty(lbIndLoc, "state", "no");
			changeLabelProperty(lbIndRem, "state", "on");
		}
		break;
	}
}

void ControlWdg::setRegState(RegState st){
	switch (st){
	case Parado: txtState->setText("Parado"); break;
	case Arrancando: txtState->setText("Arrancando"); break;
	case Parando: txtState->setText("Parando"); break;
	case Descargando: txtState->setText("Descargando"); break;
	case RegVacio: txtState->setText("Reg.Vacio"); break;
	case RegIsla: txtState->setText("Reg.Isla"); break;
	case RegPotencia: txtState->setText("Reg.Potencia"); break;
	case RegApertura: txtState->setText("Reg.Apertura"); break;
	default: txtState->setText("NULL");
	}
}

void ControlWdg::changeLabelProperty(QLabel* label, QString const& strPr, QString const& strPrVal){
	label->setProperty(strPr, strPrVal);
	label->style()->unpolish(label);
	label->style()->polish(label);
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