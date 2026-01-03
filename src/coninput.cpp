// coninput.cpp

#include <stdio.h>
#include <string.h>

#include "hal.h"

#include <putx.h>
#include <constrinput.h>
 #include "tparameters.h"
// #include "test/test_nvdata.h"

static const char* const commands = "?\\??\\reset\\r\\w\\wx\\nvdata\\out";

static const char* const help = 
	" reset\n"
	" r  <register_num>                - read register\n"
	" w  <register_num> <value_dec>    - write register\n"
	" wx <register_num> <value_hex>    - write register\n"
	" nvdata <cmd>  - send cmd to test-nvdata\n";

static uint8_t out=0;
	

//
void processConsoleInput(void)
{
	char *sp;       
	uint32_t i, i1, v;
	int32_t vi;

	sp = cgets();

	if(sp)
	{         
		switch(cparse(commands, sp))
		{
			default: break;

			case -1:								// unrecognized input
				putc('"'); puts(sp); putc('"');
				puts("?\n>");
				break;

			case 0:									// "?" - list commands short
				putc('\n');
				puts(commands);
				puts("\n>");
				break;

			case 1:									// "??" - commands description
				putc('\n');
				puts(help);
				break;

			case 2:									// "reset"
				hal::reset();
				break;

			case 3:									// "r"
				if(sscanf(sp, "r %lu %lu", &i, &i1) == 2)
				{
					for(unsigned int j = i; j < i + i1; j++)
						if(Parameters.read(j, &vi) == 0)
						{
							puts("\np"); putd(j); putc(':'); putl(vi); putc('\n');
						}
						else puts("??\n");
				} 
				else if(sscanf(sp, "r %lu", &i) == 1)
				{
					if(Parameters.read(i, &vi) == 0)
					{
						puts("\np"); putd(i); putc(':'); putl(vi); putc('\n');
					}
					else puts("??\n");
				} 
				else puts("?\n");                                       
				putc('>');
				break;

			case 4:									// "w"
				if(sscanf(sp, "w %lu = %lu", &i, &v) == 2)
				{
					if(Parameters.write(i, v) == 0)
					{
						puts("p"); putd(i); putc(':'); putl(v); putc('\n');
					}
					else puts("?\n");
				} 
				else puts("?\n");
				break;

			case 5:									// "wx"
				if(sscanf(sp, "wx %lu = %x", &i, (unsigned*)&v) == 2)
				{
					if(Parameters.write(i, v) == 0)
					{
						puts("\np"); putd(i); putc(':'); putdwx(v); putc('\n');
					}
					else puts("??\n");
				} 
				else puts("?\n");
				putc('>');
				break;
/*
#ifdef TEST_NV_DATA
			case 6:									// "nvdata .."
				test_nvdata::service(sp);
				putc('>');
				break;
#endif				
*/
			case 7:									// "out"
				if(sscanf(sp, " out %lu", &i) > 0)
					out = i;
				puts("\n>");   
				break;         

 		}		// switch
 	}			// if(sp=cgets())
}


//
void processConsoleOutput(void)
{
	static uint32_t ticks=0;

	if(out > 0  &&  TIMEOUT(ticks, 1000))
	{
		ticks = now;

		if(out == 1)
		{
#ifdef DEBUG_ADC_CODE
/*			for(uint16_t v : HAL::adcCode)
			{
				putw(v);  putc(' ');
			} */
			for(int i=0; i<ADC_NUMC; i++)
			{
				putw(HAL::adcCode[i]);  putc(' ');
			}
			putc('\n');
#endif								
		}	
	}             
}


