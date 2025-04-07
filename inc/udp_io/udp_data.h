// udp_data.h

#if !defined UDP_DATA_H
#define      UDP_DATA_H

typedef struct udp_pcb 	udp_conn_t;

//
// Буферизация входных/выходных данных
//
class UdpData
{ 
public:
	uint8_t			*buffer;				// данные
	uint16_t		size;						// размер данных в буфере, байт
	udp_conn_t	*conn;					// исользуемое udp connection
	void				*ex;						// extra data
	
	UdpData(uint8_t *abuffer, uint16_t asize, udp_conn_t *aconn, void *aex = nullptr) :
		buffer	(abuffer),
		size		(asize),
		conn		(aconn),
		ex(aex)
		{} 

  // 
	UdpData & operator=(const UdpData& c)
	{                                                 
		buffer	= c.buffer;
		size 		= c.size;
		conn		= c.conn;
		ex			= c.ex;
		return *this;
	}             
	
	//
	// Возвращает указатель на область прикладных данных в buffer
	//
	//inline uint8_t* appData(void) { return &buffer[UIP_LLH_LEN + UIP_IPUDPH_LEN]; }
	inline uint8_t* appData(void) { return &buffer[0]; }

	//
	// Возвращает указатель на область прикладных данных в p[]
	//
	//static inline uint8_t* appData(uint8_t* p) { return &p[UIP_LLH_LEN + UIP_IPUDPH_LEN]; }
	static inline uint8_t* appData(uint8_t* p) { return &p[0]; }
};
	
#endif
