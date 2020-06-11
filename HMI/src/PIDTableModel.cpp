#include "PIDTableModel.h"
#include "RegIDS.h"

QString PIDTableModel::mStrHeaders[MODEL_COLUMNS] = {"Nº", "Aplicación", "KP", "KI", "KD", "Tiempo de Scan", "Banda Muerta"};
QString PIDTableModel::mPIDDomains[MODEL_PIDS] = {"Regución en vacio", "Regulación en isla", "Regulación en potencia", "Distribuidor/Inyector 1", "Álabes/Inyector 2", "Inyector 3", "Inyector 4", "Inyector 5", "Inyector 6"};


PIDTableModel::PIDTableModel(RegImage & regImg, QObject *parent) : QAbstractTableModel(parent){
	mDataModel = new PIDInfo[MODEL_PIDS];
	connect(&regImg, &RegImage::varChanged, this, &PIDTableModel::varChanged);
}

PIDTableModel::~PIDTableModel(){
	delete[] mDataModel;
}

int PIDTableModel::rowCount(QModelIndex const&) const{
	return MODEL_PIDS;
}

int PIDTableModel::columnCount(QModelIndex const&) const{
	return MODEL_COLUMNS;
}

QVariant PIDTableModel::data(QModelIndex const& index, int role) const{
	if (role == Qt::DisplayRole){
		PIDInfo& info = mDataModel[index.row()];
		switch(index.column()){
		case 0:
			return QString("%1").arg(index.row() + 1);
		case 1:
			return mPIDDomains[index.row()];
		case 2:
			return QString::number(info.fKp, 'f', 2);
		case 3:
			return QString::number(info.fKi, 'f', 2);
		case 4:
			return QString::number(info.fKd, 'f', 2);
		case 5:
			return QString("%1").arg(info.uiScanT);
		case 6:
			return QString::number(info.fDB, 'f', 1);
		}
	}
	else if (role == Qt::TextAlignmentRole){
       return Qt::AlignCenter + Qt::AlignVCenter;
	}
	return QVariant();
}

QVariant PIDTableModel::headerData(int section, Qt::Orientation orientation, int role) const{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal && section >= 0 && section < MODEL_COLUMNS) {
        return mStrHeaders[section];
    }
    return QVariant();
}


void PIDTableModel::varChanged(VarImage const& var){
	std::uint32_t uiID = var.getID();
	if(uiID >= C_REG_PID1_KP && uiID <= C_REG_PID9_DB){
		int iPid = (uiID - C_REG_PID1_KP) / 5.0;
		int iVal = (uiID - C_REG_PID1_KP) % 5;
		if(iPid >= 0 && iPid < MODEL_PIDS){
			PIDInfo& info = mDataModel[iPid];
			switch(iVal){
			case 0:
				info.fKp = var.getCurrentVal();
				break;
			case 1:
				info.fKi = var.getCurrentVal();
				break;
			case 2:
				info.fKd = var.getCurrentVal();
				break;
			case 3:
				info.uiScanT = (std::uint32_t) var.getCurrentVal();
				break;
			case 4:
				info.fDB = var.getCurrentVal();
				break;
			}
			QModelIndex idx = createIndex(iPid, iVal + 2);
    		emit dataChanged(idx, idx, {Qt::DisplayRole});
		}
	}
}

PIDTableModel::PIDInfo const* PIDTableModel::getPIDInfo(int row) const{
	if(row >= 0 && row < MODEL_PIDS){
		return &(mDataModel[row]);
	}
	return nullptr;
}