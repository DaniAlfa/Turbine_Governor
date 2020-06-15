#include "ControlWdg.h"
#include "RegIDS.h"
#include <cstdint>
#include <QStyle>

ControlWdg::ControlWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg){
    setupUi(this);
    //lbIndRem->setStyleSheet("QLabel { background-color: rgb(170, 0, 0); }");

    cmdRegTypeP->setChecked(true);
    mRegTypeGrp.addButton(cmdRegTypeP, 0);
    mRegTypeGrp.addButton(cmdRegTypeA, 1);
    connect(&mRegTypeGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::regTypeButtonClicked);

    cmdF1->setChecked(true);
    cmdN1->setChecked(true);
    mFrecWdgGrp.addButton(cmdF1, 0);
    mSpeedWdgGrp.addButton(cmdN1, 0);
    int NoSpeeds = mpRegImg->getNoOfSpeedSensors();
    if(NoSpeeds > 1){
    	mFrecWdgGrp.addButton(cmdF2, 1);
    	mSpeedWdgGrp.addButton(cmdN2, 1);
    	if(NoSpeeds > 2){
    		mFrecWdgGrp.addButton(cmdF3, 2);
    		mSpeedWdgGrp.addButton(cmdN3, 2);
    	}
    	else{
    		cmdF3->setVisible(false);
    		cmdN3->setVisible(false);
    	} 
    }else{
    	cmdF2->setVisible(false);
    	cmdF3->setVisible(false);
    	cmdN2->setVisible(false);
    	cmdN3->setVisible(false);
    }
	connect(&mFrecWdgGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::frecButtonClicked);
    connect(&mSpeedWdgGrp, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ControlWdg::speedTypeButtonClicked);

    cmdP1->setChecked(true);
    mPwWdgGrp.addButton(cmdP1, 0);
    if(mpRegImg->getNoOfPowerSensors() > 1){
    	mPwWdgGrp.addButton(cmdP2, 1);
    }else{
    	cmdP2->setVisible(false);
    }
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
			mbRegInLoc = true;
		}else{
			changeLabelProperty(lbIndLoc, "state", "no");
			changeLabelProperty(lbIndRem, "state", "on");
			mbRegInLoc = false;
		}
		updateSPState();
		updateRegChangeButtons();
		enableControlButtons(!mbRegInLoc);
		break;
	case ZC_REG_PWSP: //Cambio en sp de potencia
		mfLastPWSP = fCurrVal;
		updateSPDisplayTxt();
		break;
	case ZC_REG_OPSP: //Cambio en sp de apertura
		mfLastOPSP = fCurrVal;
		updateSPDisplayTxt();
		break;
	case ZC_REG_SSP: //Cambio en sp de frecuencia
		mfLastFrecSP = fCurrVal;
		updateSPDisplayTxt();
		break;
	case JE_GEN_P1: //Medida de potencia 1
		dplP1->setText(QString::number(fCurrVal, 'f', 1) + var.getUnits());
		dplP1P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		break;
	case JE_GEN_P2:
		dplP2->setText(QString::number(fCurrVal, 'f', 1) + var.getUnits());
		dplP2P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		break;
	case SE_REG_F1: //Medida de frecuencia 1
		dplF1->setText(QString::number(fCurrVal, 'f', 1) + var.getUnits());
		dplF1P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		dplN1->setText(QString::number(fCurrVal * 60, 'f', 1) + "Rmp");
		dplN1P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		break;
	case SE_REG_F2: //Medida de frecuencia 2
		dplF2->setText(QString::number(fCurrVal, 'f', 1) + var.getUnits());
		dplF2P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		dplN2->setText(QString::number(fCurrVal * 60, 'f', 1) + "Rmp");
		dplN2P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		break;
	case SE_REG_F3: //Medida de frecuencia 3
		dplF3->setText(QString::number(fCurrVal, 'f', 1) + var.getUnits());
		dplF3P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		dplN3->setText(QString::number(fCurrVal * 60, 'f', 1) + "Rmp");
		dplN3P->setText(QString::number(var.getCurrentValLin(), 'f', 1) + "%");
		break;
	}
}


void ControlWdg::enableControlButtons(bool bEnable){
	cmdStart->setEnabled(bEnable);
	cmdStop->setEnabled(bEnable);
	cmdTrip->setEnabled(bEnable);
	cmdUnl->setEnabled(bEnable);
}

void ControlWdg::on_cmdStart_clicked(){
	mpRegImg->writeVarAsButton(HMSM_REG_STR, true);
}

void ControlWdg::on_cmdStop_clicked(){
	mpRegImg->writeVarAsButton(HMSM_REG_STP, true);
}

void ControlWdg::on_cmdTrip_clicked(){
	mpRegImg->writeVarAsButton(HMSM_REG_TRIP, false);
}

void ControlWdg::on_cmdUnl_clicked(){
	mpRegImg->writeVarAsButton(HMSM_REG_UNL, true);
}

void ControlWdg::updateSPDisplayTxt(){
	if(mtLastRegSt < 4){ //si no esta en reg
		txtSP->setText("");
	}
	else if(mtLastRegSt == RegVacio || mtLastRegSt == RegIsla){
		txtSP->setText(QString::number(mfLastFrecSP, 'f', 1) + " " + mpRegImg->getVarUnits(ZC_REG_SSP));
	}
	else if(mtLastRegSt == RegPotencia){
		txtSP->setText(QString::number(mfLastPWSP, 'f', 1) + " " + mpRegImg->getVarUnits(ZC_REG_PWSP));
	}
	else{
		txtSP->setText(QString::number(mfLastOPSP, 'f', 1) + " " + mpRegImg->getVarUnits(ZC_REG_OPSP));
	}
}
void ControlWdg::updateSPState(){
	bool enableSPSend = (mtLastRegSt > 5) && !mbRegInLoc;
	spSPIn->setEnabled(enableSPSend);
	cmdSendSP->setEnabled(enableSPSend);
	if(enableSPSend){
		std::uint32_t spVarID = ZR_REG_PWSP;
		if(mtLastRegSt == RegApertura){
			spVarID = ZR_REG_OPSP;
		}
		spSPIn->setMinimum(mpRegImg->getVarMin(spVarID));
		spSPIn->setMaximum(mpRegImg->getVarMax(spVarID));
		spSPIn->setValue(mpRegImg->getVarMin(spVarID));
	}
}

void ControlWdg::updateRegChangeButtons(){
	cmdRegTypeA->setEnabled(!mbRegInLoc && mtLastRegSt > 5);
	cmdRegTypeP->setEnabled(!mbRegInLoc && mtLastRegSt > 5);
	if(!mbRegInLoc){
		cmdRegTypeA->setChecked(mtLastRegSt != RegPotencia);
		cmdRegTypeP->setChecked(mtLastRegSt != RegApertura);
	}
	
}

void ControlWdg::on_cmdSendSP_clicked(){
	if(mpRegImg->writeVar((mtLastRegSt == RegPotencia) ? ZR_REG_PWSP : ZR_REG_OPSP, spSPIn->value()))
		mpRegImg->writeVarAsButton(HMSM_REG_SPOK, true);
}

void ControlWdg::setRegState(RegState st){
	mtLastRegSt = st;
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
	updateSPState();
	updateSPDisplayTxt();
	updateRegChangeButtons();
}

void ControlWdg::changeLabelProperty(QLabel* label, char const* strPr, char const* strPrVal){
	label->setProperty(strPr, strPrVal);
	label->style()->unpolish(label);
	label->style()->polish(label);
}


void ControlWdg::regTypeButtonClicked(int id){ //Id 0 potencia, 1 apertura
	if(id == 0){
		mpRegImg->writeVarAsButton(HMSM_REG_PWREG, true);
	}
	else{
		mpRegImg->writeVarAsButton(HMSM_REG_OPREG, true);
	}
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
