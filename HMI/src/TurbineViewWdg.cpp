#include "TurbineViewWdg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QStyle>
#include "PositionCtrlWdg.h"
#include "RegIDS.h"


TurbineViewWdg::TurbineViewWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg){
    setupUi(this);

    QVector<PositionCtrlWdg*> vWdgs;
    createPositionWdgs(vWdgs);
    createWdgView(vWdgs);

    connect(mpRegImg, &RegImage::varChanged, this, &TurbineViewWdg::varChanged);
}

void TurbineViewWdg::createPositionWdgs(QVector<PositionCtrlWdg*> & vWdgs){
	switch(mpRegImg->getTurbineType()){
	case Francis:
		vWdgs.append(new PositionCtrlWdg("DISTRIBUIDOR", Z_WGT_INY1, ZRC_REG_WGT_INY1, *mpRegImg, this));
		break;
	case Kaplan:
		vWdgs.append(new PositionCtrlWdg("DISTRIBUIDOR", Z_WGT_INY1, ZRC_REG_WGT_INY1, *mpRegImg, this));
		vWdgs.append(new PositionCtrlWdg("ÁLABES", Z_PAD_INY2, ZRC_REG_PAD_INY2, *mpRegImg, this));
		break;
	case Pelton:
		for(int i = 0; i < mpRegImg->getNoPeltonInyec(); ++i){
			vWdgs.append(new PositionCtrlWdg(QString("INYECTOR %1").arg(i+1), Z_WGT_INY1 + i, ZRC_REG_WGT_INY1 + i, *mpRegImg, this));
		}
		break;
	}
}

void TurbineViewWdg::createWdgView(QVector<PositionCtrlWdg*> const& vWdgs){
	if(vWdgs.size() < 4){
		QHBoxLayout *layout = new QHBoxLayout(centralWdg);

		addPositionWdgs(layout, vWdgs);
		centralWdg->setLayout(layout);
	}
	else if(vWdgs.size() < 7){
		QWidget* wdgSup = new QWidget(centralWdg);
		QWidget* wdgInf = new QWidget(centralWdg);
		QVBoxLayout *layout1 = new QVBoxLayout(centralWdg);
		QHBoxLayout *layoutSup = new QHBoxLayout(wdgSup);
		QHBoxLayout *layoutInf = new QHBoxLayout(wdgInf);

		addPositionWdgs(layoutSup, vWdgs, 0, 3);
		addPositionWdgs(layoutInf, vWdgs, 3);

		wdgSup->setLayout(layoutSup);
		wdgInf->setLayout(layoutInf);
		layout1->addWidget(wdgSup);
		layout1->addWidget(wdgInf);
		centralWdg->setLayout(layout1);
	}
}


void TurbineViewWdg::addPositionWdgs(QBoxLayout* layout, QVector<PositionCtrlWdg*> lWgds, int begin, int last){
	if(last == -1) last = lWgds.size();
	layout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	for(int i = begin; i < last; ++i){
		layout->addWidget(lWgds.at(i));
		layout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));
	}
}

void TurbineViewWdg::changeLabelProperty(QLabel* label, char const* strPr, char const* strPrVal){
	label->setProperty(strPr, strPrVal);
	label->style()->unpolish(label);
	label->style()->polish(label);
}

void TurbineViewWdg::varChanged(VarImage const& var){
	if(var.getID() == Y_REG_AUTO){
		changeLabelProperty(lbIndAuto, "state", (var.getCurrentVal()==1 ? "on" : "no"));
		changeLabelProperty(lbIndMan, "state", (var.getCurrentVal()==1 ? "no" : "on"));
	}
}