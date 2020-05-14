#ifndef VARSVIEWWDG_H
#define VARSVIEWWDG_H

#include <QWidget>
#include "ui_VarsViewWdg.h"

class VarsViewWdg : public QWidget, private Ui::VarsViewWdgUi
{
    Q_OBJECT

public:
    VarsViewWdg(QWidget *parent = nullptr);
    ~VarsViewWdg();

};
#endif // VARSVIEWWDG_H
