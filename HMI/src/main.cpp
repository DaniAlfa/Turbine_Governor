#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QString>
#include <iostream>
#include <QMetaType>
#include "CommonHMITypes.h"

int main(int argc, char *argv[]){
    if(argc != 4){
        std::cout << "Argumentos insuficientes" << std::endl;
        return -1;
    }
    QApplication app(argc, argv);
    qRegisterMetaType<VarImage>("VarImage");

    QFile fStyle(":/main_style");
    fStyle.open(QFile::ReadOnly);
    QString strStyle(fStyle.readAll());
    fStyle.close();
    app.setStyleSheet(strStyle);

    RegImage img;
    QString strError;
    if(!img.init(argv[1], argv[2], argv[3], &strError)){
    	std::cout << "Error la inicializacion: " << strError.toLatin1().data() << std::endl;
    	return -1;
    }
    MainWindow w(img);
    //w.show();
    return app.exec();
}
