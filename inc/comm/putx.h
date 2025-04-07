/*
 * putx.h
 *
 */

#ifndef	__PUTX_H
#define	__PUTX_H


#define putc(c)		__putc(c)
#define puts(s)		__puts(s)

	// hal_uarts.cpp
#ifdef __cplusplus
  extern "C" void __putc(const char c);
  extern "C" void __puts(const char* s);
#else
  void __putc(const char c);
  void __puts(const char* s);
#endif

#ifdef __cplusplus
extern "C" {
#endif

void putbx(const unsigned char b);
void putwx(const unsigned w);
void putdwx(const unsigned long dw);
void putdwx_zs(const unsigned long dw);	// zero supression
void putd(int d);
void putl(long d);
//void putw(unsigned short w);
void putdw(unsigned long dw);

void rprintfFloat(char numDigits, double x);

#ifdef __cplusplus
}
#endif

#endif	// __PUTXX_H

