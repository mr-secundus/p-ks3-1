// p_slavemsghandlers_hiio_config.h
//
// Локальные настройки для прикладного обработчика сообщений протокола связи с ВУ 
// Используется в src/protocol/p_slavemsghandlers_hiio.cpp

#define Handle_MsgPWrite
#define Handle_MsgPRead
//#define Handle_MsgPValue
//#define Handle_DateTime
//#define Handle_NV_Data
//#define Handle_MsgWakeup
//#define Handle_ADC1_Buffers

#define UseExternalAppTypeHandler				// доп. внешний обработчик для MsgAppType
