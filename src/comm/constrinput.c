/*
 *	constrinput.c
 *               
 *
 *	02.11.05
 *		- введена переменная CON_ECHO для возможности
 *			управлять эхом из программы
 *		- добавлен служебный символ '*' - в строку не заносится
 *		- в cparse ранее пробел считался концом команды, теперь при
 *			копировании команды пробел пропускается
 *
 *  02.11.06
 *    - по умолчанию cparse интерпретирует пробел как конец команды;
 *      для того чтобы это не делалось, в проекте требуется определить
 *      SPASE_NOT_AS_COMMAND_TERMINATOR
 *
 */
             
//#include <ports.h>
#include <putx.h>
                  
//#define CON_ECHO 0

//#define SPASE_NOT_AS_COMMAND_TERMINATOR
                                                   
#define CSTR_SZ			164		// макс. число байт во входной строке
#ifndef MAXCMD_SZ
	#define MAXCMD_SZ	160		// макс. длина команды
#endif

static char cstr[CSTR_SZ+1];			// строка для накопления вх. данных
static short  cbcnt=0,						// счетчик числа байт данных в cstr
		   				cbyte=0;						// последний принятый байт

#ifndef CON_ECHO
short CON_ECHO = 1;		// >0 - эхо вкл.
#endif
                          
//  
// обработчик принятого байта
//
void cprocbyte(unsigned char b)
{
	cbyte = b;
	if(b=='\n') return;		// LF 0x0a
	
	if(b=='~' || b=='*') return;		// служебный

	if(cbcnt & 0x80) cbcnt=0;		// начало новой строки

/*	
#ifdef CON_ECHO
	putb(b);
#endif
*/
	if(CON_ECHO) putc(b);

	if(b=='\b' && cbcnt!=0x80 && cbcnt!=0)	// backspace
	{
		cbcnt--; return;
	}
		
	if(b=='\r')									// CR 0x0d
	{
		cstr[cbcnt] = 0;
		cbcnt |= 0x80;      	 		// строка принята
		return;
	}        

	if(cbcnt >= CSTR_SZ)				// многовато
	{
		cstr[CSTR_SZ] = 0;
		cbcnt |= 0x80;       
	}                                 
	else cstr[cbcnt++] = b;
	return;
}           

//
// возвращает последний принятый символ,
// при последующих вызовах возвращает 0 до тех пор, пока
// не будет принят новый байт
//
char cgetc(void)
{                           
	unsigned char b=cbyte;
	cbyte=0;
	return b;
}
  
//
// возвращает имеющуюся строку байт, завершенную символом '\r',
// возвращает один раз, затем должна быть принята новая строка;
// при отсутствии данных возвращает 0
//
char* cgets(void)
{
	if(!(cbcnt & 0x80)) return 0;
	else
	{
		cbcnt=0; return cstr;
	}
}

//-----------------------------------------------------------------------------

// return:
// 0 -  s1!=s2
// 1 -  s1==s2
unsigned char cstrncmp(char* s1, char* s2)
{
	while(*s1 && *s2)
	{
		if(*s1 != *s2) return 0;
		s1++; s2++;
	}           
	if(!*s1 && !*s2) return 1;
  else return 0;
}

//
// поиск команды во входной строке s; при s=0 используется cstr
// cl - список команд, в котором ведется поиск, вида:
// "cmd1\\cmd2\\cmdn"
// возвращает номер команды, с которой начинается s;
// игнорирует пробелы в s
// возврат:
// >=0	- индекс команды в cl (начиная с 0)
// -1		- ни одна из команд не обнаружена
// -2		- нет входных данных
// -3		- не обнаружено данных в s
//
int cparse(const char* cl, char* s)
{
	char
		cc[MAXCMD_SZ+1],		// сюда копируется очередная команда из cl
		sc[MAXCMD_SZ+1],		// первые x подряд идущих символов из s, не явл. '='//пробелами
												// предполагается, что это команда
		*ccp;
	unsigned char ccnt=0, i;

	if(s==0)		// определиться со входной строкой
	{
		if(!(cbcnt & 0x80)) return -2;
		s=cstr; cbcnt=0;
	}

	// извлечь команду из входной строки
	i=0;
	while(*s && (*s==' ' || *s=='\t') && i++<MAXCMD_SZ) s++;		// удалить пробелы и backslash
	if(!*s) return -3;
	i=0;
//	while(*s && *s!=' ' && *s!='=' && i<MAXCMD_SZ) sc[i++]=*s++;

	while(*s && *s!=' ' && *s!='=' && i<MAXCMD_SZ) 
	{
		if(*s != ' ') sc[i++]=*s;
		s++;
	}
	sc[i]=0;

	while(1)
	{
		for(i=0; i<MAXCMD_SZ; i++) cc[i]=' ';
		cc[i]=0;

		while(*cl && (*cl==' ' || *cl=='\\')){ cl++;}		// удалить ведущие пробелы и backslash

		if(*cl==(char)0) return -1;

		// скопировать очередную команду
		ccp=cc;
		while(*cl && *cl!='\\' && *ccp)
		{
			*ccp=*cl;
			ccp++; cl++;
		}
		*ccp=0;

		if(cstrncmp(sc, cc))
    	return ccnt;
		ccnt++;
	}
}
