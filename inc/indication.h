// indication.h
//
// Управление индикаторами устройства

#ifndef INDICATION_H
#define INDICATION_H

namespace indication
{
	// Отдельные состояния устройства для управления индикацией
	enum AppState
	{
		kEth_Link = 0,				// Ethernet Link
		kEth_Io,							// обмен по Ethernet
		kRs485_Io,						// обмен по RS485
		kRs485_Err,						// ошибки обмена по RS485
		kHw_Err,							// аппаратная ошибка
		kSetup_Err,						// ошибка загрузки конфигурации из EEPROM
		kMaxState
	};
	
	// Установка значения состояния устройства
	void setState(AppState state, bool value);
	
	// Обработка автомата индикации
	void process(void);
};

#endif
