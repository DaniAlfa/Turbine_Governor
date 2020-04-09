#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H
#include <cstdint>
#include <unordered_map>

//Declaraciones adelantadas
class IOImage;

class AlarmManager{
public:
	AlarmManager(IOImage & ioImg);
	~AlarmManager();

	void driverFailed();
	void checkDriverErrors();

	void clearAlarms();
	void setAlarm(std::uint32_t uiError);
	void isAlarmSet(std::uint32_t uiError) const;

private:
	IOImage& mtIOImg;
	bool mbWithErrors; //Hay errores sin limpiar

	std::uint32_t* mpuiLogicErrors;
	std::uint32_t muiNumLogicErrorInts;

	std::uint32_t* mpuiFieldQStates;
	std::uint32_t muiNumFieldQStatesInts;

	std::uint8_t* mpuiAlarmLevels;

	std::unordered_map<std::uint32_t, QState> mMLastFldErrors;

	void setAlarmBit(std::uint32_t uiError);
	void setFldStates();
	void clearFldStates();

};






#endif