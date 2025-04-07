// p_slave_data_io.h
//
// Опрос ведомых устройств и обработка принятых данных


namespace p_slave_data_io
{
void init(void);
void start(void); 
void stop(void); 
void process(void);

// Обработка принятого блока данных
//
// srcAddress   сетеовй адрес отправителя
// p					  блок данных
// dataId			  идентификатор типа данных из входящего сообщения
//
// return       > 0   размер обработанного блока
//              < 0   ошибка - см. data_buffers::writeData()
int16_t handleMessageData(uint16_t srcAddress, TDataBuffer* p, uint16_t dataId);

// Обработка сообщения об отсутствии данных
//
// srcAddress   сетеовй адрес отправителя
void handleMessageNoData(uint16_t srcAddress);

};		// namespace
