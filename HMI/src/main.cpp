#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    QFile fStyle(":/main_style");
    fStyle.open(QFile::ReadOnly);
    QString strStyle(fStyle.readAll());
    fStyle.close();

    app.setStyleSheet(strStyle);

    MainWindow w;
    w.show();
    return app.exec();
}
