#include "RTMeasurer.h"


RTMeasurer::RTMeasurer(QObject* parent) : QObject(parent){}

RTMeasurer::~RTMeasurer(){
	for(auto it : mumTimers){
		delete it.second.first;
	}
}


int RTMeasurer::createMeasure(std::function<float()> f, unsigned uBufferSize, int iMillisRate){
	int measureID;
	mLock.lock();
	measureID = mvMeasures.size();
	mvMeasures.push_back({f, uBufferSize});
	auto it = mumTimers.find(iMillisRate);
	if(it != mumTimers.end()){
		it->second.second.push_back(measureID);
	}
	else{
		std::list<int>& measureIDs = mumTimers.insert({iMillisRate, {nullptr, {}}}).first->second.second;
		measureIDs.push_back(measureID);
		QTimer* timer = new QTimer();
		timer->moveToThread(this->thread());
		QObject::connect(timer, &QTimer::timeout, [this, measureIDs](){
			mLock.lock();
	        for(auto id : measureIDs){
	        	Measure& m = mvMeasures[id];
	        	m.lBuffer.push_back(m.f());
	        	if(m.uBufferSize < m.lBuffer.size()){
	        		m.lBuffer.pop_front();
	        	}
	        	++m.uTotalMeasures;
	        }
	        mLock.unlock();
	    });
		mumTimers[iMillisRate].first = timer;
	} 
	mLock.unlock();
	return measureID;
}

void RTMeasurer::startMeasures(){
	mLock.lock();
	for(auto it : mumTimers){
		it.second.first->start(it.first);
	}
	mLock.unlock();
}

unsigned RTMeasurer::getMeasures(int measureID, std::list<float> & fList){
	mLock.lock();
	unsigned uTotalMeasures = 0;
	if((unsigned)measureID < mvMeasures.size() && measureID >= 0){
		fList = std::list<float>(mvMeasures[measureID].lBuffer);
		uTotalMeasures = mvMeasures[measureID].uTotalMeasures;
	}
	mLock.unlock();
	return uTotalMeasures;
}