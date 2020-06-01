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

	void varChanged(VarImage const& var);

private:
	QButtonGroup mRegTypeGrp, mFrecWdgGrp, mSpeedWdgGrp, mPwWdgGrp;

	RegImage* mpRegImg;

	void setRegState(RegState st);
	void changeLabelProperty(QString const& strPr, QString const& strPrVal);

};
#endif // CONTROLWDG_H
