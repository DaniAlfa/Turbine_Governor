/********************************************************************************
** Form generated from reading UI file 'ControlOptWdg.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROLOPTWDG_H
#define UI_CONTROLOPTWDG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDial>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlOptWdgUi
{
public:
    QHBoxLayout *horizontalLayout;
    QDial *dial;

    void setupUi(QWidget *ControlOptWdgUi)
    {
        if (ControlOptWdgUi->objectName().isEmpty())
            ControlOptWdgUi->setObjectName(QString::fromUtf8("ControlOptWdgUi"));
        ControlOptWdgUi->resize(681, 490);
        horizontalLayout = new QHBoxLayout(ControlOptWdgUi);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        dial = new QDial(ControlOptWdgUi);
        dial->setObjectName(QString::fromUtf8("dial"));

        horizontalLayout->addWidget(dial);


        retranslateUi(ControlOptWdgUi);

        QMetaObject::connectSlotsByName(ControlOptWdgUi);
    } // setupUi

    void retranslateUi(QWidget *ControlOptWdgUi)
    {
        ControlOptWdgUi->setWindowTitle(QApplication::translate("ControlOptWdgUi", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ControlOptWdgUi: public Ui_ControlOptWdgUi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLOPTWDG_H
