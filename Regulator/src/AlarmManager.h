#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H
#include <cstdint>
#include <unordered_map>
#include <atomic>
#include "CommonRegTypes.h"

class IOImage;

class AlarmManager{
public:
	AlarmManager(IOImage & ioImg);
	~AlarmManager();

	void driverFailed();
	void checkDriverErrors();

	void clearAlarms();
	void setAlarm(std::uint32_t uiError);
	bool isAlarmSet(std::uint32_t uiError) const;

	std::uint32_t getNoLogicErrors() const{return muiNumLogicErrorInts;}
	std::uint32_t const* getLogicErrors() const{ return mpuiLogicErrors;}
	std::uint32_t getNoFieldSt() const{return muiNumFieldQStatesInts;}
	std::uint32_t const* getFieldSt() const{ return mpuiFieldQStates;}

private:
	IOImage & mtIOImg;
	std::atomic<bool> mbWithErrors; //Hay errores sin limpiar

	std::uint32_t* mpuiLogicErrors;
	std::uint32_t muiNumLogicErrorInts;

	std::uint32_t* mpuiFieldQStates;
	std::uint32_t muiNumFieldQStatesInts;

	std::uint8_t* mpuiAlarmLevels;

	std::unordered_map<std::uint32_t, QuState> mMLastFldErrors;

	void setAlarmBit(std::uint32_t uiError);
	void setFldStates();
	void clearFldStates();

};






#endif