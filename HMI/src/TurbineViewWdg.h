#ifndef TURBINEVIEWWDG_H
#define TURBINEVIEWWDG_H

#include <QWidget>
#include "ui_TurbineViewWdg.h"

class TurbineViewWdg : public QWidget, private Ui::TurbineViewWdgUi
{
    Q_OBJECT

public:
    TurbineViewWdg(QWidget *parent = nullptr);
    ~TurbineViewWdg();

};
#endif // TURBINEVIEWWDG_H
