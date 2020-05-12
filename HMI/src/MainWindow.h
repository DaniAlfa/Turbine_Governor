#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindowUi
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
	void mainButtonClicked(int id);

private:
	QButtonGroup mMainBtnGrp;

};
#endif // MAINWINDOW_H
