#ifndef ALARMSWDG_H
#define ALARMSWDG_H

#include <QWidget>
#include <cstdint>
#include <Alarms.h>
#include <QString>
#include "RegImage.h"
#include "ui_AlarmsWdg.h"

class AlarmsWdg : public QWidget, private Ui::AlarmsWdgUi
{
    Q_OBJECT

public:
    AlarmsWdg(RegImage & regImg, QWidget *parent = nullptr);
    ~AlarmsWdg();

private slots:
	void on_cmdRepo_clicked();	
	void roIntChanged(std::uint32_t const val, std::uint32_t const intID);

	void updateAlarmsDisplay();

private:
	RegImage* mpRegImg;

	std::uint32_t muiNumLogicErrorsInts, muiNumFieldStInts;
	std::uint32_t* mpLogicErrors;
	std::uint32_t* mpFieldSt;

	static QString mStrLogicErrors[NUM_ALARMS];

	void logicErrorIntChanged(std::uint32_t const val, std::uint32_t const intID);
	void fieldStIntChanged(std::uint32_t const val, std::uint32_t const intID);

	void printErrorInLog(QString const& strError);
	void printErrorInLog(QString const& strTag, QString const& strDesc, QuState qState);

	static QString getCurrentDateTime();
	static QString qualityStateToString(QuState const st);
	QString genLogicErrorsReport() const;
	void appendLogicErrorsRep(std::uint32_t uiVal, std::uint32_t uiIdx, QString & strRep) const;
	QString genFieldStReport() const;
	void appendFieldStRep(std::uint32_t uiVal, std::uint32_t uiIdx, QString & strRep) const;

};
#endif // ALARMSWDG_H
