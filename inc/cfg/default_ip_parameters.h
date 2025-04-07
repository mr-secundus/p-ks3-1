// default_ip_parameters.h
//
// Default значения IP адресов и номера портов UDP

// Упаковка октетов в uint32_t
//#define X4TOU32(a3, a2, a1, a0)		(((a3) << 24) | ((a2) << 16) | ((a1) << 8) | (a0))

// LITTLE_ENDIAN 
#define X4_TO_IP4_ADDR(a,b,c,d) \
					( ((uint32_t)((d) & 0xff) << 24) | \
						((uint32_t)((c) & 0xff) << 16) | \
            ((uint32_t)((b) & 0xff) << 8)  | \
            (uint32_t)((a) & 0xff) )


// Default значения IP адресов и номера портов UDP
#define DEFAULT_IP				X4_TO_IP4_ADDR(192UL, 168UL, 0, 136UL)
#define DEFAULT_GW				X4_TO_IP4_ADDR(192UL, 168UL, 0, 1UL)
#define DEFAULT_SMASK			X4_TO_IP4_ADDR(255UL, 255UL, 255UL, 0)
#define DEFAULT_LOCAL_PORT		3000
#define DEFAULT_DEST_PORT			4023 
