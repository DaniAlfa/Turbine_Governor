#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QString>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    QFile fStyle(":/main_style");
    fStyle.open(QFile::ReadOnly);
    QString strStyle(fStyle.readAll());
    fStyle.close();
    app.setStyleSheet(strStyle);

    RegImage img;
    if(!img.init("RegIOInfo.xml", "RegConfig.xml")){
    	std::cout << "Error en los archivos de inicializacion" << std::endl;
    	return -1;
    }
    MainWindow w(img);
    //w.show();
    return app.exec();
}
