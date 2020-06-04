#ifndef CONTROL_H 
#define CONTROL_H
#include <string>

//Declaraciones adelantadas
class IOImage;
class AlarmManager;

class Control{
public:
	Control(IOImage & ioImg, AlarmManager & alMan);
	~Control();

	bool init(std::string const& strConfigPath);

	void updateControl();

private:
	IOImage* io;
	AlarmManager* mpAlMan;

};




#endif //CONTROL_H