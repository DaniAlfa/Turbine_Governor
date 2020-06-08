#ifndef TURBINEVIEWWDG_H
#define TURBINEVIEWWDG_H

#include <QWidget>
#include "ui_TurbineViewWdg.h"

//Declaraciones Adelantadas
class RegImage;

class TurbineViewWdg : public QWidget, private Ui::TurbineViewWdgUi
{
    Q_OBJECT

public:
    TurbineViewWdg(RegImage & regImg, QWidget *parent = nullptr);
    ~TurbineViewWdg();

private:
	RegImage* mpRegImg;

};
#endif // TURBINEVIEWWDG_H
