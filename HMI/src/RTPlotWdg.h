#ifndef RTPLOTWDG_H
#define RTPLOTWDG_H
#include <QWidget>
#include <QString>
#include <QMap>

#include <QRectF>
#include <QPen>
#include <QLineF>
#include <QFont>

#include <functional>
#include <list>

//Declaraciones adelantadas
class RTMeasurer;

class RTPlotWdg : public QWidget {
	Q_OBJECT
public:
	enum TIMEUNITS{SEC, MILLIS, MIN};
	class PlotConfig {
	public:
		int iMeasureRate; //Tasa de medida aplicable a todas las funciones de la grafica
		RTPlotWdg::TIMEUNITS tMeasureUnits; //Unidades de la tasa de medida
		int iDisplayInterval; //Intervalo de tiempo que se mostrara en la grafica
		RTPlotWdg::TIMEUNITS tDisplayUnits; //Unidades del intervalo de tiempo
		int iYNoTics; //Numero de secciones en las que se divide el eje y
		int iMaxXNoTics; //Numero de secciones en las que se divide el eje x
		float fYMaxVal; //Valor maximo del eje Y
		float fYMinVal; //Valor minimo del eje Y
		QString strAxisYName; //Unidades de la variable Y
		int iAxisYPrecission; //Precision en los numeros mostrados en el eje Y
		int iAxisXPrecission; //Precision en los numeros mostrados en el eje X
	};

	RTPlotWdg(RTMeasurer & measurer, RTPlotWdg::PlotConfig config, QWidget *parent = nullptr);
	~RTPlotWdg();

	bool addPlot(QString strPlotName, std::function<float()> updateFunct, QColor plotColor, float fPlotLineWidth = 3);
	void startUpdateTimer();

	//static RTPlotWdg::PlotConfig getDefaultConfig() { return { 500, true, 11, 21, 0, 0, "", 0, 0}; }

protected:
	
	class PlotAttributes {
	public:
		PlotAttributes(QString strPlotName, std::function<float()> f, QColor color, float fLineWidth) : mstrName(strPlotName), updateF(f), mColor(color),
		mfLineWidth(fLineWidth) {}
		QString mstrName;
		std::function<float()> updateF;
		int measureID;
		QColor mColor;
		float mfLineWidth;
		std::list<float> lLastMeasures;
	};

	RTMeasurer* mpRTMeasurer;

	QMap<QString, PlotAttributes*> mPlots;
	
	PlotConfig mtConfig; //Configuracion del widget
	unsigned muiMaxBuffSize; //Tamaño maximo de los buffers de medidas 
	unsigned muiMeasureRateMillis; //Tasa de medicion en milisegundos
	unsigned muiPointsTillMove; //Puntos que faltan para que la grafica avance un instante de tiempo
	unsigned muiTotalMeasures; //Numero total de medidas

	float mfMaxYAxisTextWidth; //Calculo del ancho maximo del texto del eje y
	float mfStartInstant; //Ultimo tiempo en el que comienza la grafica
	float mfTimeBetweenTics; //Tiempo entre las divisiones del eje x

	QPen	mScalePen,
			mScaleGridPen,
			mLimitLinePen, 
			mCaptionPen;	

	QFont mScaleTextFont,
		  mCaptionTextFont,
		  mScaleEgusFont;

	QRectF mScaleRect,
		   mCaptionRect;

	QLineF mXLine,
		   mYLine;
	QColor mTextColor;

	//Eventos de pintado, cambio de tamaño del widget y timer para la actualizacion de las medidas
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent* event);
	void timerEvent(QTimerEvent *event);

	//Funciones de pintado
	void paintCaption(QPainter & painter);
	void paintScale(QPainter & painter);
	void paintPlots(QPainter & painter);

	//Funciones de utilidad
	static QString numberToScaleString(float strNumber, int iPrecission);
	static int getTimeConversionFactor(TIMEUNITS origin, TIMEUNITS dest);
	static QString timeUnitsToString(TIMEUNITS units);
};

#endif //RTPLOTWDG_H 
