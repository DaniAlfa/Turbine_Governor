#ifndef PIDTABLEMODEL_H 
#define PIDTABLEMODEL_H

#include <QAbstractTableModel>
#include <cstdint>
#include "RegImage.h"
#define MODEL_COLUMNS 7
#define MODEL_PIDS 9


class PIDTableModel : public QAbstractTableModel{
    Q_OBJECT
public:
    PIDTableModel(RegImage & regImg, QObject *parent = nullptr);
    ~PIDTableModel();

    class PIDInfo{
	public:
		float fKp, fKi, fKd, fDB;
		std::uint32_t uiScanT;
	};

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    int columnCount(QModelIndex const& parent = QModelIndex()) const override;
    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    PIDInfo const* getPIDInfo(int row) const;

private slots:
	void varChanged(VarImage const& var);

private:
	static QString mStrHeaders[MODEL_COLUMNS];
	static QString mPIDDomains[MODEL_PIDS];

	PIDInfo* mDataModel;
};




#endif //PIDTABLEMODEL_H