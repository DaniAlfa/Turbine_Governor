#ifndef RTMEASURER_H
#define RTMEASURER_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <functional>
#include <vector>
#include <list>
#include <unordered_map>
#include <utility>

class RTMeasurer : public QObject{
	Q_OBJECT
public:
	RTMeasurer(QObject* parent = nullptr);
	~RTMeasurer();

	unsigned getMeasures(int measureID, std::list<float> & fList); //Devuelve el numero de medidas totales tomadas
	int createMeasure(std::function<float()> f, unsigned uBufferSize, int iMillisRate); //Devuelve el identificador de la medida

public slots:
	void startMeasures();

protected:
	class Measure{
	public:
		Measure(std::function<float()> f, unsigned uBufferSize) : f(f), uBufferSize(uBufferSize), uTotalMeasures(0) {}
		std::list<float> lBuffer;
		std::function<float()> f;
		unsigned uBufferSize;
		unsigned uTotalMeasures;
	};

	QMutex mLock; //Mutex para el acceso a mvMeasures

	std::vector<Measure> mvMeasures;

	//Mapa de millisegundos de actualizacion con pareja con time que notifica en esos milisegundos
	//y los identificadores de las medidas a realizar en la lista
	std::unordered_map<int, std::pair<QTimer*, std::list<int>>> mumTimers; 

};





#endif //RTMEASURER_H 