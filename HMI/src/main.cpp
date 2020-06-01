#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QString>
#include <iostream>
#include <QMetaType>
#include "CommonHMITypes.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qRegisterMetaType<VarImage>("VarImage");

    QFile fStyle(":/main_style");
    fStyle.open(QFile::ReadOnly);
    QString strStyle(fStyle.readAll());
    fStyle.close();
    app.setStyleSheet(strStyle);

    RegImage img;
    QString strError;
    if(!img.init("RegIOInfo.xml", "RegConfig.xml", "MasterDriverCnf.xml", &strError)){
    	std::cout << "Error la inicializacion: " << strError << std::endl;
    	return -1;
    }
    MainWindow w(img);
    //w.show();
    return app.exec();
}
