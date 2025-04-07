#include <cr_section_macros.h>

#include <putx.h>
#include "hal.h"
#include "version.h"
#include "tasks.h"
#include "coninput.h"

//
void putInitState(void)
{
  puts("\nv."); putdw(V_MAJOR); putc('.'); putdw(V_MINOR); putc('.');
  putdw(V_BUILD); putc('.'); putdw(V_FEATURE); putc(' ');
  puts(__DATE__); putc(' '); puts(__TIME__);
  puts("\nMain PLL clock: "); putl(Chip_Clock_GetMainPLLHz() / 1000); puts(" kHz\n"); 
//  puts("\nFCCLK:"); putl(FCCLK); 
//  puts(" FPCLK:"); putl(FPCLK);  putc('\n');
//  puts("ImageCRC:"); putdwx(R188); puts(" ControlCRC:"); putdwx(R189);  putc('\n');
}


//
int main(void) 
{
	hal::init();

  putInitState();
  
  initTasks();

  while(1)
  {
    hal::process();
		processTasks();
		processConsoleInput();       
		processConsoleOutput();
  }

  return 0 ;
}
