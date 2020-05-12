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

private:
	QButtonGroup mRegTypeGrp;

};
#endif // CONTROLWDG_H
