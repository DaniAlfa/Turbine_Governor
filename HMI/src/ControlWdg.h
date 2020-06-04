#ifndef CONTROLWDG_H
#define CONTROLWDG_H

#include <QWidget>
#include <QButtonGroup>
#include "RegImage.h"
#include "ui_ControlWdg.h"

class ControlWdg : public QWidget, private Ui::ControlWdgUi
{
    Q_OBJECT

public:
    ControlWdg(RegImage & regImg, QWidget *parent = nullptr);
    ~ControlWdg();

private slots:
	void regTypeButtonClicked(int id);
	void frecButtonClicked(int id);
	void speedTypeButtonClicked(int id);
	void pwButtonClicked(int id);
	void on_cmdSendSP_clicked();
	void on_cmdStart_clicked();
	void on_cmdStop_clicked();
	void on_cmdTrip_clicked();
	void on_cmdUnl_clicked();

	void varChanged(VarImage const& var);

private:
	QButtonGroup mRegTypeGrp, mFrecWdgGrp, mSpeedWdgGrp, mPwWdgGrp;

	RegImage* mpRegImg;

	float mfLastFrecSP, mfLastPWSP, mfLastOPSP;
	RegState mtLastRegSt;
	bool mbRegInLoc;

	void setRegState(RegState st);
	void changeLabelProperty(QLabel* label, char const* strPr, char const* strPrVal);
	void updateSPDisplayTxt();
	void updateSPState();
	void updateRegChangeButtons();
	void enableControlButtons(bool bEnable);


};
#endif // CONTROLWDG_H
