#ifndef CONTROLOPTWDG_H
#define CONTROLOPTWDG_H

#include <QWidget>
#include <cstdint>
#include "PIDTableModel.h"
#include "RegImage.h"
#include "ui_ControlOptWdg.h"


class ControlOptWdg : public QWidget, private Ui::ControlOptWdgUi
{
    Q_OBJECT

public:
    ControlOptWdg(RegImage & regImg, QWidget *parent = nullptr);

private slots:
	void selectionChanged(QItemSelection const& selected, QItemSelection const& deselected);
	void on_cmdSendPID_clicked();
	void on_cmdSendOPLim_clicked();

	void varChanged(VarImage const& var);

private:
	RegImage* mpRegImg;

	PIDTableModel* mpTableModel;
	PIDTableModel::PIDInfo const* mpSelectedPIDInfo;
	int miSelectedRow;

	
	void updateSelectionFrame(QDoubleSpinBox* sp, std::uint32_t uiStartID, float fVal);
	void updateSelectionFrame(QSpinBox* sp, std::uint32_t uiStartID, std::uint32_t uiVal);

	void updatePIDVal(float fPrevVal, float fNewVal, std::uint32_t uiStartID);

};
#endif // CONTROLOPTWDG_H
