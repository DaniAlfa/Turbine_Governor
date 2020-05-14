#ifndef ALARMSWDG_H
#define ALARMSWDG_H

#include <QWidget>
#include <QButtonGroup>
#include "ui_AlarmsWdg.h"

class AlarmsWdg : public QWidget, private Ui::AlarmsWdgUi
{
    Q_OBJECT

public:
    AlarmsWdg(QWidget *parent = nullptr);
    ~AlarmsWdg();

private slots:
	void regTypeButtonClicked(int id);

private:
	QButtonGroup mRegTypeGrp;

};
#endif // ALARMSWDG_H
