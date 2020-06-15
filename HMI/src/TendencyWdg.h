#ifndef TENDENCYWDG_H
#define TENDENCYWDG_H

#include <QWidget>
#include <QThread>
#include <unordered_map>
#include <cstdint>
#include "RTMeasurer.h"
#include "RegImage.h"
#include "ui_TendencyWdg.h"

//Declaraciones adelantadas
class RTPlotWdg;

class TendencyWdg : public QWidget, private Ui::TendencyWdgUi{
    Q_OBJECT

public:
    TendencyWdg(RegImage & regImg, QWidget *parent = nullptr);
    ~TendencyWdg();

private slots:
	void imageVarsInitialized();
	void varChanged(VarImage const& var);

private:
	RegImage* mpRegImg;

	std::unordered_map<std::uint32_t, float> mumLastMeasures;

	QThread mMeasurerThread;
	RTMeasurer mRTMeasurer;

	RTPlotWdg* mpSpeedPlot;
	RTPlotWdg* mpPWPlot;
};
#endif // TENDENCYWDG_H
