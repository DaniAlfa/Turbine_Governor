#include "ControlOptWdg.h"
#include "RegIDS.h"
#include <QModelIndex>

ControlOptWdg::ControlOptWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg), mpSelectedPIDInfo(nullptr), miSelectedRow(-1){
   setupUi(this);
   mpTableModel = new PIDTableModel(regImg, tvPIDS);
   tvPIDS->setModel(mpTableModel);
   tvPIDS->setSelectionBehavior(QAbstractItemView::SelectRows);
   tvPIDS->setSelectionMode(QAbstractItemView::SingleSelection);
   tvPIDS->horizontalHeader()->resizeSection(0,30);
   tvPIDS->horizontalHeader()->resizeSection(1,500);
   tvPIDS->horizontalHeader()->resizeSection(2,130);
   tvPIDS->horizontalHeader()->resizeSection(3,130);
   tvPIDS->horizontalHeader()->resizeSection(4,130);
   tvPIDS->horizontalHeader()->resizeSection(5,130);
   tvPIDS->horizontalHeader()->resizeSection(6,130);
   frSelection->setVisible(false);

   spOpLim->setMinimum(mpRegImg->getVarMin(C_REG_OPLIMIT));
   spOpLim->setMaximum(mpRegImg->getVarMax(C_REG_OPLIMIT));
   connect(tvPIDS->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ControlOptWdg::selectionChanged);
   connect(mpRegImg, &RegImage::varChanged, this, &ControlOptWdg::varChanged);
}

void ControlOptWdg::selectionChanged(QItemSelection const&, QItemSelection const&){
	QModelIndex index = tvPIDS->selectionModel()->currentIndex();
	if(!index.isValid()){
		 frSelection->setVisible(false);
		 mpSelectedPIDInfo = nullptr;
		 return;
	}
	mpSelectedPIDInfo = mpTableModel->getPIDInfo(index.row());
	if(mpSelectedPIDInfo == nullptr) return;
	miSelectedRow = index.row();
	pidLabel->setText(QString("PID %1").arg(miSelectedRow + 1));
	updateSelectionFrame(spKp, C_REG_PID1_KP, mpSelectedPIDInfo->fKp);
	updateSelectionFrame(spKi, C_REG_PID1_KI, mpSelectedPIDInfo->fKi);
	updateSelectionFrame(spKd, C_REG_PID1_KD, mpSelectedPIDInfo->fKd);
	updateSelectionFrame(spSt, C_REG_PID1_ST, mpSelectedPIDInfo->uiScanT);
	updateSelectionFrame(spDb, C_REG_PID1_DB, mpSelectedPIDInfo->fDB);
	frSelection->setVisible(true);
}

void ControlOptWdg::on_cmdSendPID_clicked(){
	if(mpSelectedPIDInfo == nullptr) return;
	updatePIDVal(mpSelectedPIDInfo->fKp, spKp->value(), C_REG_PID1_KP);
	updatePIDVal(mpSelectedPIDInfo->fKi, spKi->value(), C_REG_PID1_KI);
	updatePIDVal(mpSelectedPIDInfo->fKd, spKd->value(), C_REG_PID1_KD);
	updatePIDVal(mpSelectedPIDInfo->uiScanT, spSt->value(), C_REG_PID1_ST);
	updatePIDVal(mpSelectedPIDInfo->fDB, spDb->value(), C_REG_PID1_DB); 
}

void ControlOptWdg::updateSelectionFrame(QDoubleSpinBox* sp, std::uint32_t uiStartID, float fVal){
	std::uint32_t uiId = uiStartID + miSelectedRow*5;
	sp->setValue(fVal);
	sp->setMinimum(mpRegImg->getVarMin(uiId));
	sp->setMaximum(mpRegImg->getVarMax(uiId));
}

void ControlOptWdg::updateSelectionFrame(QSpinBox* sp, std::uint32_t uiStartID, std::uint32_t uiVal){
	std::uint32_t uiId = uiStartID + miSelectedRow*5;
	sp->setValue(uiVal);
	sp->setMinimum(mpRegImg->getVarMin(uiId));
	sp->setMaximum(mpRegImg->getVarMax(uiId));
}

void ControlOptWdg::updatePIDVal(float fPrevVal, float fNewVal, std::uint32_t uiStartID){
	if(fPrevVal != fNewVal){
		mpRegImg->writeVar(uiStartID + miSelectedRow*5, fNewVal);
	}
}

void ControlOptWdg::varChanged(VarImage const& var){
	if(C_REG_OPLIMIT == var.getID()){
		txtOpLim->setText(QString::number(var.getCurrentVal(), 'f', 1));
	}
}

void ControlOptWdg::on_cmdSendOPLim_clicked(){
	mpRegImg->writeVar(C_REG_OPLIMIT, spOpLim->value());
}