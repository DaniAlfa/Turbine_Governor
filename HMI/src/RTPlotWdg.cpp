#include "RTPlotWdg.h"
#include "RTMeasurer.h"
#include <QPainter>
#include <QBrush>
#include <QFontMetricsF>
#include <QList>
#include <QPointF>
#include <algorithm>

#define WDG_CAPTION_HEIGHT	45
#define WDG_MARGIN_CAPTION_TOP		25
#define WDG_MARGIN_CAPTION_BOTTOM	10
#define WDG_MARGIN_CAPTION_LEFT	   20
#define WDG_MARGIN_CAPTION_RIGHT   20
#define WDG_CAPTION_MAX_CHARS 10
#define WDG_CAPTION_VERT_DIFF 10
#define WDG_CAPTION_LINES_SIZE 30
#define WDG_CAPTION_DISTBETWEEN_CAPS 10

#define WDG_MARGIN_SCALE_TOP 5
#define WDG_MARGIN_SCALE_BOTTOM 10
#define WDG_MARGIN_SCALE_LEFT 7 //min 2
#define WDG_MARGIN_SCALE_RIGHT 19 // min 9
#define WDG_SCALE_MAX_CHARS 8
#define WDG_SCALE_TIC_WIDTH 10
#define WDG_SCALE_TIC_HEIGHT 2

RTPlotWdg::RTPlotWdg(RTMeasurer & measurer, RTPlotWdg::PlotConfig config, QWidget *parent) : QWidget(parent), mpRTMeasurer(&measurer), mtConfig(config), mfStartInstant(0){
	
	//Inicializar pens y brushes
	mScaleGridPen = QPen(QBrush(QColor(0, 40, 77)), 1, Qt::DashLine);
	mLimitLinePen = QPen(QBrush(Qt::red), 1, Qt::DashLine);
	mScalePen = QPen(QBrush(QColor(0, 92, 179)), 3, Qt::SolidLine);
	mCaptionPen = QPen(QBrush(Qt::black), 3, Qt::SolidLine);
	mTextColor = QColor(Qt::black);

	mScaleTextFont = QFont("Tahoma", 8); mScaleTextFont.setBold(true);
	mScaleEgusFont = QFont("Tahoma", 9); mScaleEgusFont.setBold(true);
	mCaptionTextFont = QFont("Tahoma", 10); mCaptionTextFont.setBold(true);

	QFontMetricsF fmText(mScaleTextFont);
	mfMaxYAxisTextWidth = std::max(fmText.width(numberToScaleString(mtConfig.fYMaxVal, mtConfig.iAxisYPrecission)), fmText.width(numberToScaleString(mtConfig.fYMinVal, mtConfig.iAxisYPrecission)));
	//QFontMetricsF fm(mScaleEgusFont);
	//QFontMetricsF fmEgus(mScaleEgusFont);
	
	/*
	mCaptionRect.setRect(0, 0, WDG_SCREEN_WIDTH_SIZE, WDG_CAPTION_HEIGHT);
	mScaleRect.setRect(0, WDG_CAPTION_HEIGHT, WDG_SCREEN_WIDTH_SIZE, WDG_SCREEN_HEIGHT_SIZE - WDG_CAPTION_HEIGHT);

	mYLine.setLine(WDG_MARGIN_SCALE_LEFT + mfMaxYAxisTextWidth + WDG_SCALE_TIC_WIDTH, WDG_MARGIN_SCALE_TOP + fmEgus.height() + mScaleRect.y(), WDG_MARGIN_SCALE_LEFT + mfMaxYAxisTextWidth + WDG_SCALE_TIC_WIDTH, WDG_SCREEN_HEIGHT_SIZE - (WDG_MARGIN_SCALE_BOTTOM + fmText.height() + WDG_SCALE_TIC_WIDTH / 2));
	mXLine.setLine(WDG_MARGIN_SCALE_LEFT + mfMaxYAxisTextWidth + WDG_SCALE_TIC_WIDTH, WDG_SCREEN_HEIGHT_SIZE - (WDG_MARGIN_SCALE_BOTTOM + fmText.height() + WDG_SCALE_TIC_WIDTH / 2), WDG_SCREEN_WIDTH_SIZE - fmText.width("ms") - WDG_MARGIN_SCALE_RIGHT, WDG_SCREEN_HEIGHT_SIZE - (WDG_MARGIN_SCALE_BOTTOM + fmText.height() + WDG_SCALE_TIC_WIDTH / 2));
	*/
	//this->setMouseTracking(true); //Para poner captions en las graficas
	
	muiMaxBuffSize = (mtConfig.iDisplayInterval * getTimeConversionFactor(mtConfig.tDisplayUnits, mtConfig.tMeasureUnits)) / mtConfig.iMeasureRate;
	muiMeasureRateMillis = mtConfig.iMeasureRate * getTimeConversionFactor(mtConfig.tMeasureUnits, MILLIS);
	muiPointsTillMove = 0;
	muiTotalMeasures = 0;
	mfTimeBetweenTics = mtConfig.iDisplayInterval / mtConfig.iMaxXNoTics;
	this->startTimer(mtConfig.iMeasureRate / 2);
}


RTPlotWdg::~RTPlotWdg(){
	for (PlotAttributes* plot : mPlots) {
		delete plot;
	}
}

bool RTPlotWdg::addPlot(QString strPlotName, std::function<float()> updateFunct, QColor plotColor, float fPlotLineWidth) {
	if (mPlots.contains(strPlotName)) return false;
	PlotAttributes* plot = new PlotAttributes(strPlotName, updateFunct, plotColor, fPlotLineWidth);
	plot->measureID = mpRTMeasurer->createMeasure(updateFunct, muiMaxBuffSize, muiMeasureRateMillis);
	mPlots.insert(strPlotName, plot);
	return true;
}

void RTPlotWdg::startUpdateTimer(){
	this->startTimer(mtConfig.iMeasureRate / 2);
}

void RTPlotWdg::resizeEvent(QResizeEvent*) {
	QFontMetricsF fmEgus(mScaleEgusFont);
	QFontMetricsF fmText(mScaleTextFont);
	mCaptionRect.setRect(0, 0, width(), WDG_CAPTION_HEIGHT);
	mScaleRect.setRect(0, WDG_CAPTION_HEIGHT, width(), height() - WDG_CAPTION_HEIGHT);

	mYLine.setLine(WDG_MARGIN_SCALE_LEFT + mfMaxYAxisTextWidth + WDG_SCALE_TIC_WIDTH, WDG_MARGIN_SCALE_TOP + fmEgus.height() + mScaleRect.y(), WDG_MARGIN_SCALE_LEFT + mfMaxYAxisTextWidth + WDG_SCALE_TIC_WIDTH, height() - (WDG_MARGIN_SCALE_BOTTOM + fmText.height() + WDG_SCALE_TIC_WIDTH / 2));
	mXLine.setLine(WDG_MARGIN_SCALE_LEFT + mfMaxYAxisTextWidth + WDG_SCALE_TIC_WIDTH, height() - (WDG_MARGIN_SCALE_BOTTOM + fmText.height() + WDG_SCALE_TIC_WIDTH / 2), width() - fmText.width(timeUnitsToString(mtConfig.tDisplayUnits)) - WDG_MARGIN_SCALE_RIGHT, height() - (WDG_MARGIN_SCALE_BOTTOM + fmText.height() + WDG_SCALE_TIC_WIDTH / 2));
}

void RTPlotWdg::timerEvent(QTimerEvent *) {
	unsigned uTotalMeasures = 0;
	unsigned uBuffSize = 0;
	for (PlotAttributes * plot : mPlots) {
		uTotalMeasures = mpRTMeasurer->getMeasures(plot->measureID, plot->lLastMeasures);
		uBuffSize = plot->lLastMeasures.size();
	}
	if (muiMaxBuffSize <= uBuffSize - muiPointsTillMove){
		muiPointsTillMove = (mfTimeBetweenTics * getTimeConversionFactor(mtConfig.tDisplayUnits, mtConfig.tMeasureUnits)) / (mtConfig.iMeasureRate);
		mfStartInstant += mfTimeBetweenTics;
	} 
	else{
		muiPointsTillMove -= (uTotalMeasures - muiTotalMeasures);
		muiTotalMeasures = uTotalMeasures;
	} 
	this->update();
}

void RTPlotWdg::paintEvent(QPaintEvent *) {
	QPainter painter(this);

	//Rectangulo exterior
	painter.setBrush(QColor(Qt::white));
	painter.drawRect(this->contentsRect());
	painter.setBrush(QColor(Qt::black));
	painter.drawPoint(0, 0);
	painter.setBrush(QColor(128, 179, 255));
	//Ajuste de escala y renderizado
	//double fFactorX = (double)this->width() / (double)sizeHint().width();
	//double fFactorY = (double)this->height() / (double)sizeHint().height();
	//painter.scale(fFactorX, fFactorY);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	//Pintado de widget
	paintCaption(painter);
	painter.setBrush(QColor(179, 217, 255));
	paintScale(painter);
	paintPlots(painter);
}



void RTPlotWdg::paintCaption(QPainter & painter) {
	painter.setPen(mCaptionPen);
	QFontMetricsF fm(mCaptionTextFont);
	
	painter.drawRect(mCaptionRect);
	painter.setFont(mCaptionTextFont);

	float fX = WDG_MARGIN_CAPTION_LEFT + mCaptionRect.x();
	float fY = WDG_MARGIN_CAPTION_TOP + mCaptionRect.y();

	for (PlotAttributes const* plot : mPlots) {
		QString strCaption = plot->mstrName;
		if (strCaption.size() > WDG_CAPTION_MAX_CHARS) {
			strCaption.resize(WDG_CAPTION_MAX_CHARS - 3);
			strCaption += "...";
		}
		float fCaptionLenght = fm.width(strCaption) + WDG_CAPTION_LINES_SIZE + 4;
		if (fX + fCaptionLenght > mCaptionRect.width() - WDG_MARGIN_CAPTION_RIGHT) {
			fY += WDG_CAPTION_VERT_DIFF;
			if (fY + fm.height() > mCaptionRect.height() - WDG_MARGIN_CAPTION_BOTTOM) break;
		}

		painter.setPen(plot->mColor);
		painter.drawLine(fX, fY, fX + WDG_CAPTION_LINES_SIZE, fY);
		fX += WDG_CAPTION_LINES_SIZE + 4;

		painter.setPen(mTextColor);
		
		painter.drawText(fX, fY + (fm.height() / 4), strCaption);
		fX += fm.width(strCaption) + WDG_CAPTION_DISTBETWEEN_CAPS;
	}
}


void RTPlotWdg::paintScale(QPainter & painter) {
	QFontMetricsF fm(mScaleEgusFont);
	painter.setPen(mTextColor);
	painter.setFont(mScaleEgusFont);

	painter.setPen(Qt::black);
	painter.drawRect(mScaleRect);

	painter.drawText(mYLine.x1(), mYLine.y1() - 4, mtConfig.strAxisYName);
	painter.drawText(mXLine.x2() + 4, mXLine.y2() + fm.height() / 4, timeUnitsToString(mtConfig.tDisplayUnits));

	fm = QFontMetricsF(mScaleTextFont);

	painter.setPen(mScalePen);
	painter.setFont(mScaleTextFont);
	painter.drawLine(mYLine);
	painter.drawLine(mXLine);

	float fX = mXLine.x1();
	float fY = mXLine.y1();
	float fHalfTicWidth = WDG_SCALE_TIC_WIDTH / 2;
	float fWidthBetweenXTics = abs(mXLine.x2() - mXLine.x1()) / (mtConfig.iMaxXNoTics);
	float fWidthBetweenYTics = abs(mYLine.y1() - mYLine.y2()) / (mtConfig.iYNoTics);

	float fTimeInstant = mfStartInstant;
	for (int i = 0; i <= mtConfig.iMaxXNoTics; ++i) {
		painter.setPen(mScalePen);
		painter.drawLine(fX, fY - fHalfTicWidth, fX, fY + fHalfTicWidth);
		painter.setPen(mTextColor);
		QString strNumber = numberToScaleString(fTimeInstant, mtConfig.iAxisXPrecission);
		painter.drawText(fX - fm.width(strNumber) / 2, fY + fHalfTicWidth + fm.height(), strNumber);
		if (i != 0) {
			/*
			if (mfNumberOfMeasures == i + 1) painter.setPen(mLimitLinePen);
			else painter.setPen(mScaleGridPen);
			
			painter.drawLine(fX, fY, fX, mYLine.y1());
			*/
			painter.setPen(mScaleGridPen);
			painter.drawLine(fX, fY, fX, mYLine.y1());
		}
		fTimeInstant += mfTimeBetweenTics;
		fX += fWidthBetweenXTics;
	}

	fX = mYLine.x2();
	fY = mYLine.y2();
	float fYVal = mtConfig.fYMinVal;
	float fYIncrement = abs(mtConfig.fYMaxVal - mtConfig.fYMinVal) / (mtConfig.iYNoTics);
	for (int i = 0; i <= mtConfig.iYNoTics; ++i) {
		painter.setPen(mScalePen);
		painter.drawLine(fX - fHalfTicWidth, fY, fX + fHalfTicWidth, fY);
		painter.setPen(mTextColor);
		QString strNumber = numberToScaleString(fYVal, mtConfig.iAxisYPrecission);
		painter.drawText(fX - fHalfTicWidth * 2 - fm.width(strNumber), fY + (fm.height() / 5), strNumber);
		if (i != 0) {
			painter.setPen(mScaleGridPen);
			painter.drawLine(fX, fY, mXLine.x2(), fY);
		}
		fYVal += fYIncrement;
		fY -= fWidthBetweenYTics;
	}
}

void RTPlotWdg::paintPlots(QPainter & painter) {
	float fX = mXLine.x1();
	float fWidthBetweenXTics = abs(mXLine.x2() - mXLine.x1()) / (mtConfig.iMaxXNoTics);
	float fWidthBetweenPoints = (mtConfig.iMeasureRate * fWidthBetweenXTics) / (mfTimeBetweenTics * getTimeConversionFactor(mtConfig.tDisplayUnits, mtConfig.tMeasureUnits));
	float fWidthBetweenYTics = abs(mYLine.y1() - mYLine.y2()) / (mtConfig.iYNoTics);
	float fYIncrement = abs(mtConfig.fYMaxVal - mtConfig.fYMinVal) / (mtConfig.iYNoTics);
	for (PlotAttributes const* plot : mPlots) {
		QPen plotPen = QPen();
		plotPen.setWidthF(plot->mfLineWidth);
		plotPen.setColor(plot->mColor);
		painter.setPen(plotPen);
		fX = mXLine.x1();
		QPointF lastPoint;
		QPointF actualPoint;
		for (auto it = plot->lLastMeasures.cbegin(); it != plot->lLastMeasures.cend(); ++it) {
			actualPoint = QPointF(fX, mXLine.y1() - ((*it / fYIncrement) * fWidthBetweenYTics));
			painter.drawPoint(actualPoint);
			if (!lastPoint.isNull()) painter.drawLine(actualPoint, lastPoint);
			lastPoint = actualPoint;
			fX += fWidthBetweenPoints;
		}
	}
	
}


QString RTPlotWdg::numberToScaleString(float fVal, int iPrecission){
	QString strNumber = QString("%1").arg(fVal, 0, 'f', iPrecission);
	if(strNumber.size() > WDG_SCALE_MAX_CHARS) strNumber = QString("%1").arg(fVal, 0, 'E', 2);
	return strNumber;
} 

int RTPlotWdg::getTimeConversionFactor(TIMEUNITS origin, TIMEUNITS dest){
	if(origin == dest) return 1;
	else if(origin == MIN){
		if(dest == SEC) return 60;
		else return 60000;
	} else if(origin == SEC){
		if(dest == MILLIS)
			return 1000;
	}
	return 0;
}

QString RTPlotWdg::timeUnitsToString(TIMEUNITS units){
	QString strUnits = "";
	switch(units){
	case MILLIS:
		strUnits = "ms";
		break;
	case SEC:
		strUnits = "s";
		break;
	case MIN:
		strUnits = "min";
		break;
	}
	return strUnits;
}
