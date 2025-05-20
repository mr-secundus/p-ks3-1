// putxx.c
//
// 19.01.06 добавлена rprintfFloat, источник - armlib/rprintf.c
//
// 08.07.08 вызовы puts заменены на __puts для исключения конфликтов
//          со стандартной библиотекой
//

#include <putx.h>

const char hexAsciiTable[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

//-----------------------------------------------------------------------------
// перекодировка значения байта в два символа ASCII -
// соотв. младшая и старшая тетрады в HEX-формате
unsigned short hexb2asciiw(unsigned char b)
{
	unsigned short wh, wl;
	wl = hexAsciiTable[b & 0x0f];
	wh = hexAsciiTable[(b >> 4) & 0x0f];
	return (wl<<8) | wh;	// СТАРШИЙ БАЙТ ПЕРВЫЙ
}

//-------------------------------------------------------------------
//
int putchar(int c)
{
	putc(c); return c;
}

               
//-------------------------------------------------------------------
void putbx(const unsigned char b)
{
	putc(hexAsciiTable[(b >> 4) & 0x0f]);
	putc(hexAsciiTable[b & 0x0f]);
}

//-------------------------------------------------------------------
void putwx(const unsigned w)
{
	putc(hexAsciiTable[(w >> 12) & 0x0f]);
	putc(hexAsciiTable[(w >> 8) & 0x0f]);
	putc(hexAsciiTable[(w >> 4) & 0x0f]);
	putc(hexAsciiTable[w & 0x0f]);
}

//-------------------------------------------------------------------
void putdwx(const unsigned long dw)
{
	putwx(dw>>16);
	putwx(dw);
}

//-------------------------------------------------------------------
//
// вывод unsigned long в hex с подавлением ведущих нолей
//
void putdwx_zs(const unsigned long w)
{ 
	register unsigned char b0, b1=0, *p, c=4;
	                          
	p = ((unsigned char*)&w)+3;
	do
	{               
		b0 = (*p >> 4) & 0x0f;
		if(b0 || b1)
		{
			b1=0xff; putc(hexAsciiTable[b0]);
		}                
		b0 = *p & 0x0f;
		if(b0 || b1)
		{
			b1=0xff; putc(hexAsciiTable[b0]);
		}       
		p--;         
	}while(--c);
	if(!b1) putc('0');
}                

//-------------------------------------------------------------------
void putd(short d)
{
	unsigned char buf[7]; 
	register unsigned char* p = &buf[sizeof(buf)-1];
	register unsigned short r; 
	register unsigned short w = d < 0 ? -d : d;
	buf[6] = 0;
	do
	{                                 
		p--;
	  r = w % 10;
	 	*p = r + 0x30;
	  w /= 10;
	} while ( w );
	if ( d < 0 )
	{ 
		p--;
		*p = '-';
	}
	puts((const char*)p);
}

//-------------------------------------------------------------------
void putl(long d)
{
	unsigned char buf[12]; 
	register unsigned char* p = &buf[sizeof(buf)-1];
	register unsigned long r; 
	register unsigned long dw = d < 0 ? -d : d;
	buf[11] = 0;
	do
	{                                 
		p--;
	  r = dw % 10;
	 	*p = r + 0x30;
	  dw /= 10;
	} while ( dw );
	if ( d < 0 )
	{ 
		p--;
		*p = '-';
	}
	puts((const char*)p);
}

//-------------------------------------------------------------------
void putw( unsigned short w )
{
	unsigned char buf[6]; 
	register unsigned char* p = &buf[sizeof(buf)-1];
	register unsigned short r;
	buf[5] = 0;
	do {                                 
				p--;
	  		r = w % 10;
	   		*p = r + 0x30;
	   		w /= 10;
	} while ( w );
	puts((const char*)p);
}
//-------------------------------------------------------------------
void putdw( unsigned long w )
{
	unsigned char buf[11]; 
	register unsigned char* p = &buf[sizeof(buf)-1];
	register unsigned long r;
	buf[10] = 0;
	do {                                 
				p--;
	  		r = w % 10;
	   		*p = r + 0x30;
	   		w /= 10;
	} while ( w );
	puts((const char*)p);
}


//-------------------------------------------------------------------
//
void rprintfFloat(char numDigits, double x)
{
	unsigned char firstplace = 0;
	unsigned char i, digit;
	double place = 1.0;
	
	// print polarity character
	if(x<0) putc('-');
	// convert to absolute value
	x = (x>0)?(x):(-x);
	
	// find starting digit place
	for(i=0; i<15; i++)
	{
		if((x/place) < 10.0) break;
		else place *= 10.0;
	}

	// print digits
	for(i=0; i<numDigits; i++)
	{
		digit = (x/place);

		if(digit | firstplace | (place == 1.0))
		{
			firstplace = 1;
			putc(digit+0x30);
		}
		else putc(' ');
		
		if(place == 1.0)
		{
			putc('.');
		}
		
		x -= (digit*place);
		place /= 10.0;
	}
}
