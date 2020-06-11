#ifndef TURBINEVIEWWDG_H
#define TURBINEVIEWWDG_H

#include <QWidget>
#include <QVector>
#include "RegImage.h"
#include "ui_TurbineViewWdg.h"

//Declaraciones Adelantadas
class PositionCtrlWdg;

class TurbineViewWdg : public QWidget, private Ui::TurbineViewWdgUi
{
    Q_OBJECT

public:
    TurbineViewWdg(RegImage & regImg, QWidget *parent = nullptr);

private slots:
	void varChanged(VarImage const& var);

private:
	RegImage* mpRegImg;

	
	void createPositionWdgs(QVector<PositionCtrlWdg*> & vWdgs);
	void addPositionWdgs(QBoxLayout* layout, QVector<PositionCtrlWdg*> lWgds, int begin = 0, int last = -1);
	void createWdgView(QVector<PositionCtrlWdg*> const& vWdgs);
	void changeLabelProperty(QLabel* label, char const* strPr, char const* strPrVal);

};
#endif // TURBINEVIEWWDG_H
