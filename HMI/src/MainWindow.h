#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include "RegImage.h"
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, private Ui::MainWindowUi
{
    Q_OBJECT

public:
    MainWindow(RegImage & regImg, QWidget *parent = nullptr);
    ~MainWindow();

protected:
	void timerEvent(QTimerEvent *event) override;

private slots:
	void mainButtonClicked(int id);

	void imageVarsInitialized();

	void comError();

	void recoveredFromComError();

	void writeError(std::uint32_t const varID);

private:
	QButtonGroup mMainBtnGrp;

	RegImage* mpRegImg;


};
#endif // MAINWINDOW_H
