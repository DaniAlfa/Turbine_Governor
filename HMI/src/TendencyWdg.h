#ifndef TENDENCYWDG_H
#define TENDENCYWDG_H

#include <QWidget>
#include "ui_TendencyWdg.h"

class TendencyWdg : public QWidget, private Ui::TendencyWdgUi
{
    Q_OBJECT

public:
    TendencyWdg(QWidget *parent = nullptr);
    ~TendencyWdg();

};
#endif // TENDENCYWDG_H
