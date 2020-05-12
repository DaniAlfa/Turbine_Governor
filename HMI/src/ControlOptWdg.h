#ifndef CONTROLOPTWDG_H
#define CONTROLOPTWDG_H

#include <QWidget>
#include "ui_ControlOptWdg.h"

class ControlOptWdg : public QWidget, private Ui::ControlOptWdgUi
{
    Q_OBJECT

public:
    ControlOptWdg(QWidget *parent = nullptr);
    ~ControlOptWdg();

};
#endif // CONTROLOPTWDG_H
