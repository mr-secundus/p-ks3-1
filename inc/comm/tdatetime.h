// tdatetime.h

#if !defined(__T_DATE_TIME)
#define __T_DATE_TIME
//
typedef struct tagDateTime
{
	uint16_t	year;
	uint8_t		month, 
//					dayOfMonth,
					date,
					hours,
					minutes,
					seconds,
					reserved;
}TDateTime;
#endif

