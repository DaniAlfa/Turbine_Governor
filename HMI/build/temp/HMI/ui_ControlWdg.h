/********************************************************************************
** Form generated from reading UI file 'ControlWdg.ui'
**
** Created by: Qt User Interface Compiler version 5.12.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROLWDG_H
#define UI_CONTROLWDG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLCDNumber>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlWdgUi
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *lineEdit;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_10;
    QLabel *label_9;
    QLabel *label_8;
    QLabel *label_7;
    QLabel *label_6;
    QLabel *label_11;
    QGroupBox *groupBox_3;
    QWidget *widget;
    QFrame *frame_3;
    QGridLayout *gridLayout;
    QLabel *label;
    QLCDNumber *lcdNumber;
    QLCDNumber *lcdNumber_2;
    QFrame *frame_4;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QLCDNumber *lcdNumber_3;
    QLCDNumber *lcdNumber_4;
    QFrame *frame_5;
    QGridLayout *gridLayout_3;
    QLabel *label_3;
    QLCDNumber *lcdNumber_5;
    QLCDNumber *lcdNumber_6;
    QWidget *widget_2;
    QGridLayout *gridLayout_4;
    QFrame *frame_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QGroupBox *groupBox_6;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *cmdRegTypeP;
    QPushButton *cmdRegTypeA;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pushButton_6;
    QFrame *frame_6;
    QHBoxLayout *horizontalLayout_3;
    QLabel *lbIndRem;
    QLabel *lbIndLoc;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *ControlWdgUi)
    {
        if (ControlWdgUi->objectName().isEmpty())
            ControlWdgUi->setObjectName(QString::fromUtf8("ControlWdgUi"));
        ControlWdgUi->resize(885, 611);
        verticalLayout = new QVBoxLayout(ControlWdgUi);
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        frame = new QFrame(ControlWdgUi);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        groupBox = new QGroupBox(frame);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 151, 78));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setReadOnly(true);

        verticalLayout_2->addWidget(lineEdit);

        groupBox_2 = new QGroupBox(frame);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(160, 10, 571, 64));
        horizontalLayout_4 = new QHBoxLayout(groupBox_2);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_10 = new QLabel(groupBox_2);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_4->addWidget(label_10);

        label_9 = new QLabel(groupBox_2);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout_4->addWidget(label_9);

        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_4->addWidget(label_8);

        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_4->addWidget(label_7);

        label_6 = new QLabel(groupBox_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_4->addWidget(label_6);

        label_11 = new QLabel(groupBox_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_4->addWidget(label_11);

        groupBox_3 = new QGroupBox(frame);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(750, 10, 120, 80));

        verticalLayout->addWidget(frame);

        widget = new QWidget(ControlWdgUi);
        widget->setObjectName(QString::fromUtf8("widget"));
        frame_3 = new QFrame(widget);
        frame_3->setObjectName(QString::fromUtf8("frame_3"));
        frame_3->setGeometry(QRect(130, 60, 171, 91));
        frame_3->setFrameShape(QFrame::StyledPanel);
        frame_3->setFrameShadow(QFrame::Raised);
        gridLayout = new QGridLayout(frame_3);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(frame_3);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lcdNumber = new QLCDNumber(frame_3);
        lcdNumber->setObjectName(QString::fromUtf8("lcdNumber"));
        lcdNumber->setProperty("intValue", QVariant(60));

        gridLayout->addWidget(lcdNumber, 1, 0, 1, 2);

        lcdNumber_2 = new QLCDNumber(frame_3);
        lcdNumber_2->setObjectName(QString::fromUtf8("lcdNumber_2"));
        lcdNumber_2->setProperty("value", QVariant(100.000000000000000));

        gridLayout->addWidget(lcdNumber_2, 0, 1, 1, 1);

        frame_4 = new QFrame(widget);
        frame_4->setObjectName(QString::fromUtf8("frame_4"));
        frame_4->setGeometry(QRect(350, 60, 171, 91));
        frame_4->setFrameShape(QFrame::StyledPanel);
        frame_4->setFrameShadow(QFrame::Raised);
        gridLayout_2 = new QGridLayout(frame_4);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_2 = new QLabel(frame_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 0, 0, 1, 1);

        lcdNumber_3 = new QLCDNumber(frame_4);
        lcdNumber_3->setObjectName(QString::fromUtf8("lcdNumber_3"));
        lcdNumber_3->setProperty("value", QVariant(100.000000000000000));

        gridLayout_2->addWidget(lcdNumber_3, 0, 1, 1, 1);

        lcdNumber_4 = new QLCDNumber(frame_4);
        lcdNumber_4->setObjectName(QString::fromUtf8("lcdNumber_4"));
        lcdNumber_4->setProperty("intValue", QVariant(60));

        gridLayout_2->addWidget(lcdNumber_4, 1, 0, 1, 2);

        frame_5 = new QFrame(widget);
        frame_5->setObjectName(QString::fromUtf8("frame_5"));
        frame_5->setGeometry(QRect(590, 60, 161, 91));
        frame_5->setFrameShape(QFrame::StyledPanel);
        frame_5->setFrameShadow(QFrame::Raised);
        gridLayout_3 = new QGridLayout(frame_5);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_3 = new QLabel(frame_5);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setWordWrap(true);

        gridLayout_3->addWidget(label_3, 0, 0, 1, 1);

        lcdNumber_5 = new QLCDNumber(frame_5);
        lcdNumber_5->setObjectName(QString::fromUtf8("lcdNumber_5"));
        lcdNumber_5->setProperty("value", QVariant(100.000000000000000));

        gridLayout_3->addWidget(lcdNumber_5, 0, 1, 1, 1);

        lcdNumber_6 = new QLCDNumber(frame_5);
        lcdNumber_6->setObjectName(QString::fromUtf8("lcdNumber_6"));
        lcdNumber_6->setProperty("intValue", QVariant(60));

        gridLayout_3->addWidget(lcdNumber_6, 1, 0, 1, 2);


        verticalLayout->addWidget(widget);

        widget_2 = new QWidget(ControlWdgUi);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        gridLayout_4 = new QGridLayout(widget_2);
        gridLayout_4->setSpacing(0);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        frame_2 = new QFrame(widget_2);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame_2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton = new QPushButton(frame_2);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(frame_2);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout->addWidget(pushButton_2);

        pushButton_3 = new QPushButton(frame_2);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout->addWidget(pushButton_3);

        groupBox_6 = new QGroupBox(frame_2);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        horizontalLayout_2 = new QHBoxLayout(groupBox_6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        cmdRegTypeP = new QPushButton(groupBox_6);
        cmdRegTypeP->setObjectName(QString::fromUtf8("cmdRegTypeP"));
        cmdRegTypeP->setCheckable(true);

        horizontalLayout_2->addWidget(cmdRegTypeP);

        cmdRegTypeA = new QPushButton(groupBox_6);
        cmdRegTypeA->setObjectName(QString::fromUtf8("cmdRegTypeA"));
        cmdRegTypeA->setCheckable(true);

        horizontalLayout_2->addWidget(cmdRegTypeA);


        horizontalLayout->addWidget(groupBox_6);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        pushButton_6 = new QPushButton(frame_2);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        horizontalLayout->addWidget(pushButton_6);


        gridLayout_4->addWidget(frame_2, 1, 1, 1, 2);

        frame_6 = new QFrame(widget_2);
        frame_6->setObjectName(QString::fromUtf8("frame_6"));
        frame_6->setMinimumSize(QSize(85, 50));
        frame_6->setMaximumSize(QSize(85, 50));
        frame_6->setFrameShape(QFrame::StyledPanel);
        frame_6->setFrameShadow(QFrame::Raised);
        horizontalLayout_3 = new QHBoxLayout(frame_6);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(2, 2, 2, 2);
        lbIndRem = new QLabel(frame_6);
        lbIndRem->setObjectName(QString::fromUtf8("lbIndRem"));
        lbIndRem->setStyleSheet(QString::fromUtf8(""));
        lbIndRem->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(lbIndRem);

        lbIndLoc = new QLabel(frame_6);
        lbIndLoc->setObjectName(QString::fromUtf8("lbIndLoc"));
        lbIndLoc->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(lbIndLoc);


        gridLayout_4->addWidget(frame_6, 0, 2, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_4->addItem(horizontalSpacer, 0, 1, 1, 1);


        verticalLayout->addWidget(widget_2);


        retranslateUi(ControlWdgUi);

        QMetaObject::connectSlotsByName(ControlWdgUi);
    } // setupUi

    void retranslateUi(QWidget *ControlWdgUi)
    {
        ControlWdgUi->setWindowTitle(QApplication::translate("ControlWdgUi", "Form", nullptr));
        groupBox->setTitle(QApplication::translate("ControlWdgUi", "Estado", nullptr));
        groupBox_2->setTitle(QApplication::translate("ControlWdgUi", "Comparador de velocidades", nullptr));
        label_10->setText(QApplication::translate("ControlWdgUi", "N=0", nullptr));
        label_9->setText(QApplication::translate("ControlWdgUi", "BRK", nullptr));
        label_8->setText(QApplication::translate("ControlWdgUi", "EXC", nullptr));
        label_7->setText(QApplication::translate("ControlWdgUi", "SYNC", nullptr));
        label_6->setText(QApplication::translate("ControlWdgUi", "LOVS", nullptr));
        label_11->setText(QApplication::translate("ControlWdgUi", "HOVS", nullptr));
        groupBox_3->setTitle(QApplication::translate("ControlWdgUi", "Conexiones", nullptr));
        label->setText(QApplication::translate("ControlWdgUi", "Frecuencia", nullptr));
        label_2->setText(QApplication::translate("ControlWdgUi", "Velocidad", nullptr));
        label_3->setText(QApplication::translate("ControlWdgUi", "Potencia Activa", nullptr));
        pushButton->setText(QApplication::translate("ControlWdgUi", "Arranque", nullptr));
        pushButton_2->setText(QApplication::translate("ControlWdgUi", "Parada", nullptr));
        pushButton_3->setText(QApplication::translate("ControlWdgUi", "Descarga", nullptr));
        groupBox_6->setTitle(QApplication::translate("ControlWdgUi", "Tipo regulacion", nullptr));
        cmdRegTypeP->setText(QApplication::translate("ControlWdgUi", "Potencia", nullptr));
        cmdRegTypeA->setText(QApplication::translate("ControlWdgUi", "Apertura", nullptr));
        pushButton_6->setText(QApplication::translate("ControlWdgUi", "Disparo", nullptr));
        lbIndRem->setText(QApplication::translate("ControlWdgUi", "REM", nullptr));
        lbIndLoc->setText(QApplication::translate("ControlWdgUi", "LOC", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ControlWdgUi: public Ui_ControlWdgUi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLWDG_H
