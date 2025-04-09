// nv_parameters.h
//
// Операции с nvdata - хранение блоко параметров в NV памяти

#ifndef NV_PARAMETERS_H
#define NV_PARAMETERS_H

namespace nv_parameters
{ 
// Коды возврата функций модуля
enum rc_t
{
  rcOk = 0,
  rcError,          // ошибка
  rcNotReady,       // устройство не готово
  rcCrcError,		    // ошибка CRC данных
  rcInvalidData	    // некорректный формат данных
};

// Инициализация периферии, используемой для хранения NV параметров
void init(void);

// Загрузка блока параметров из NV памяти
//
// id						идентификатор блока - см. defines.h/NV_ID_..
// opResult			указатель на переменную, в которой устанавливается (сбрасывается)
//							разряд при ошибке (успешном выполнении) операции - см. описание R99
//
// return				rcOk, rcInvalidData, rcCrcError
rc_t load(uint16_t id, uint32_t* opResult=nullptr);

// Сохранение блока параметров в NV память
//
// id						идентификатор блока - см. defines.h/NV_ID_..
// opResult			указатель на переменную, в которой устанавливается (сбрасывается)
//							разряд при ошибке (успешном выполнении) операции - см. описание R99
//
// return				rcOk, rcInvalidData
rc_t save(uint16_t id, uint32_t* opResult=nullptr);
};  // namespace

#endif