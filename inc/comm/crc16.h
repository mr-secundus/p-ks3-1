// crc16.h

#ifdef __cplusplus
  extern "C" {
#endif

extern const unsigned short Crc16Table[256];


unsigned short UpdateCRC16(unsigned char bdata, unsigned short CRC);
unsigned short UpdateCRC(unsigned char bdata, unsigned short crc);
//unsigned short calcBlockCRC16(unsigned char *data, unsigned short sz);
unsigned short calcBlockCRC16i(unsigned char *data, unsigned short sz, unsigned short initCRC);

#ifdef __cplusplus
  }
#endif

#define UPDATE_CRC16(Octet, CRC) Crc16Table[(CRC >> 8) & 255] ^ (CRC << 8) ^ Octet;





