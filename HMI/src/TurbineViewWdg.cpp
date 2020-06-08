#include "TurbineViewWdg.h"
#include "RegImage.h"


TurbineViewWdg::TurbineViewWdg(RegImage & regImg, QWidget *parent) : QWidget(parent), mpRegImg(&regImg){
    setupUi(this);

}

TurbineViewWdg::~TurbineViewWdg(){

}
