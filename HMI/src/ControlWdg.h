#ifndef CONTROLWDG_H
#define CONTROLWDG_H

#include <QWidget>
#include <QButtonGroup>
#include "ui_ControlWdg.h"

class ControlWdg : public QWidget, private Ui::ControlWdgUi
{
    Q_OBJECT

public:
    ControlWdg(QWidget *parent = nullptr);
    ~ControlWdg();

private slots:
	void regTypeButtonClicked(int id);
	void frecButtonClicked(int id);
	void speedTypeButtonClicked(int id);
	void pwButtonClicked(int id);

private:
	QButtonGroup mRegTypeGrp, mFrecWdgGrp, mSpeedWdgGrp, mPwWdgGrp;

};
#endif // CONTROLWDG_H
