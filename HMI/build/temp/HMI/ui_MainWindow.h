/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowUi
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QStackedWidget *StkWidget;
    QWidget *page1;
    QWidget *page2;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QPushButton *cmdControl;
    QPushButton *cmdControlOpt;
    QSpacerItem *horizontalSpacer;
    QMenuBar *menubar;
    QMenu *menuAyuda;

    void setupUi(QMainWindow *MainWindowUi)
    {
        if (MainWindowUi->objectName().isEmpty())
            MainWindowUi->setObjectName(QString::fromUtf8("MainWindowUi"));
        MainWindowUi->resize(800, 600);
        centralwidget = new QWidget(MainWindowUi);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(2, -1, 2, 2);
        StkWidget = new QStackedWidget(centralwidget);
        StkWidget->setObjectName(QString::fromUtf8("StkWidget"));
        page1 = new QWidget();
        page1->setObjectName(QString::fromUtf8("page1"));
        StkWidget->addWidget(page1);
        page2 = new QWidget();
        page2->setObjectName(QString::fromUtf8("page2"));
        StkWidget->addWidget(page2);

        verticalLayout->addWidget(StkWidget);

        frame = new QFrame(centralwidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(1, 1, 1, 1);
        cmdControl = new QPushButton(frame);
        cmdControl->setObjectName(QString::fromUtf8("cmdControl"));
        cmdControl->setCheckable(true);
        cmdControl->setChecked(false);

        horizontalLayout->addWidget(cmdControl);

        cmdControlOpt = new QPushButton(frame);
        cmdControlOpt->setObjectName(QString::fromUtf8("cmdControlOpt"));
        cmdControlOpt->setCheckable(true);
        cmdControlOpt->setChecked(false);

        horizontalLayout->addWidget(cmdControlOpt);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addWidget(frame);

        MainWindowUi->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindowUi);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 32));
        menuAyuda = new QMenu(menubar);
        menuAyuda->setObjectName(QString::fromUtf8("menuAyuda"));
        MainWindowUi->setMenuBar(menubar);

        menubar->addAction(menuAyuda->menuAction());

        retranslateUi(MainWindowUi);

        StkWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindowUi);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowUi)
    {
        MainWindowUi->setWindowTitle(QApplication::translate("MainWindowUi", "MainWindow", nullptr));
        cmdControl->setText(QApplication::translate("MainWindowUi", "Control", nullptr));
        cmdControlOpt->setText(QApplication::translate("MainWindowUi", "Parametros", nullptr));
        menuAyuda->setTitle(QApplication::translate("MainWindowUi", "Ayuda", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindowUi: public Ui_MainWindowUi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
