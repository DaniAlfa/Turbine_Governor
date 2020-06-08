#ifndef POSITIONCTRLWDG_H
#define POSITIONCTRLWDG_H

#include <QWidget>
#include <QString>
#include <cstdint>
#include "RegImage.h"
#include "ui_PositionCtrlWdg.h"


class PositionCtrlWdg : public QWidget, private Ui::PositionCtrlWdgUi{
    Q_OBJECT

public:
    PositionCtrlWdg(QString const& strTitle, std::uint32_t uiReadID, std::uint32_t uiWriteID, RegImage & regImg, QWidget *parent = nullptr);
    void setManual(bool bManual);
    void setWriteLimits(float fMin, float fMax);

private slots:
	void varChanged(VarImage const& var);
	void on_cmdMore_clicked();
	void on_cmdMMore_clicked();
	void on_cmdLess_clicked();
	void on_cmdLLess_clicked();
	void on_cmdStop_clicked();

private:
	RegImage* mpRegImg;

	std::uint32_t muiReadID, muiWriteID;
	bool mbManual;

	float mfMinLimit;
	float mfMaxLimit;
	float mfLastSpeedVal;

};
#endif // POSITIONCTRLWDG_H
