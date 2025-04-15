#include <string.h>
#include <stdio.h>

#include <putx.h>

#include "hal.h"
#include "hal_uarts.h"
#include "cfg/defines.h"
#include "board.h"

//-----------------------------------------------------------------------------
//                                Data
//-----------------------------------------------------------------------------

namespace hal
{
/* Structure for initial base clock states */
struct CLK_BASE_STATES 
{
	CHIP_CGU_BASE_CLK_T clk;		/* Base clock */
	CHIP_CGU_CLKIN_T clkin;			/* Base clock source, see UM for allowable souorces per base clock */
	bool autoblock_enab;				/* Set to true to enable autoblocking on frequency change */
	bool powerdn;								/* Set to true if the base clock is initially powered down */
};

/* Initial base clock states are mostly on */
const struct CLK_BASE_STATES InitClkStates[] = 
{
	/* Ethernet Clock base */
	{CLK_BASE_PHY_TX, CLKIN_ENET_TX, true, false},
	{CLK_BASE_PHY_RX, CLKIN_ENET_TX, true, false},

	/* Clocks derived from dividers */
	{CLK_BASE_USB1, CLKIN_IDIVD, true, true}
};

const PINMUX_GRP_T pinmuxing[] = 
{
	// TXD1	P3.4	P3.4	Tx RS485-1
	// RXD1	P3.5	P3.5	Rx RS485-1
	//      P7.5	P7.0	DE RS485-1     GPIO3.13    P7.5 на опытной плате
	{0x3, 4, (SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0x3, 5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC4)},
	
	//	TXD3	P4.1	P4.1	Tx   RS232
	//	RXD3	P4.2	P4.2	Rx   RS232
	//	RTS3	P4.0	P4.0	RTS  RS232
	//	CTS3	P4.10	P4.10	CTS  RS232
	{0x4, 1, (SCU_MODE_INACT | SCU_MODE_FUNC6)},
	{0x4, 2, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC6)},
	
	// Board LEDs
//	{0x6,  9, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC0)},
//	{0x6, 11, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC0)},
//	{0x2,  7, (SCU_MODE_INBUFF_EN | SCU_MODE_PULLUP | SCU_MODE_FUNC0)},

	// PHY reset
  {PHY_RST_PORT, PHY_RST_BIT, SCU_MODE_FUNC0}, 	

	// Внимание! Подключение некоторых сигналов отличается от платы SK4337
	//
	// ENET Pin mux (RMII Pins) 
	{0x1, 18, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, //+ TXD0
	{0x1, 20, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, //+ TXD1
	{0x0,  1, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC6)}, //+ TXEN
	{0x1, 15, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, //+ RXD0
	{0x0,  0, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2)}, //+ RXD1
	{0x1, 16, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC7)}, //+ RX_DV
	{0x1, 17, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, //+ MDIO
	{0x7,  7, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC6)}, //+ MDC
	{0x1, 19, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0)}, //+ REFCLK
};

typedef struct
{
	uint8_t port;
	uint8_t pin;
} io_port_t;

static const io_port_t gpioLEDBits[] = 
{
	{LED0_GPIO_PORT, LED0_GPIO_BIT}, {LED1_GPIO_PORT, LED1_GPIO_BIT},
	{LED2_GPIO_PORT, LED2_GPIO_BIT}, {LED3_GPIO_PORT, LED3_GPIO_BIT}
};

volatile uint32_t ticks = 0;

bool wdiState = false;					// состояние выхода управления внешним WDT
};

//-----------------------------------------------------------------------------
//                                ISR
//-----------------------------------------------------------------------------

extern volatile uint32_t systick_timems;		// lwip/src/lpc18xx_43xx_systick_arch.c

// SysTick ISR
extern "C" void SysTick_Handler(void)
{
	hal::ticks++;
	systick_timems++;
}


// TIMER1 ISR
//
// Timer1 используется для отсчета таймаута выключения драйвера RS485-1
extern "C" void TIMER1_IRQHandler(void)
{
	if(Chip_TIMER_MatchPending(LPC_TIMER1, 1)) 
	{
		Chip_TIMER_ClearMatch(LPC_TIMER1, 1);
		Chip_TIMER_Disable(LPC_TIMER1);

		hal::setUart1Drv(false);
	}
}

//-----------------------------------------------------------------------------
//                          Private functions
//-----------------------------------------------------------------------------

namespace hal
{
// Set up and initialize clocking
void setupClocking(void)
{
	/* Enable Flash acceleration and setup wait states */
	Chip_CREG_SetFlashAcceleration(MAX_CLOCK_FREQ);

	/* Setup System core frequency to MAX_CLOCK_FREQ */
	Chip_SetupCoreClock(CLKIN_CRYSTAL, MAX_CLOCK_FREQ, true);

	/* Setup system base clocks and initial states. This won't enable and
		 disable individual clocks, but sets up the base clock sources for
		 each individual peripheral clock. */
	for (uint32_t i = 0; i < (sizeof(InitClkStates) / sizeof(InitClkStates[0])); i++) 
		Chip_Clock_SetBaseClock(InitClkStates[i].clk, InitClkStates[i].clkin,
								InitClkStates[i].autoblock_enab, InitClkStates[i].powerdn);

	/* Reset and enable 32Khz oscillator */
//	LPC_CREG->CREG0 &= ~((1 << 3) | (1 << 2));
//	LPC_CREG->CREG0 |= (1 << 1) | (1 << 0);
}


// Инициализация дискретных выходов
void initGpio()
{
	for(uint32_t idx = 0; idx < (sizeof(gpioLEDBits) / sizeof(io_port_t)); ++idx) 
	{
		Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, gpioLEDBits[idx].port, gpioLEDBits[idx].pin);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpioLEDBits[idx].port, gpioLEDBits[idx].pin, false);
	}
	
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, PHY_RST_GPIO_PORT, PHY_RST_GPIO_BIT);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, WDI_GPIO_PORT, WDI_GPIO_BIT);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, U1_DIR_GPIO_PORT, U1_DIR_GPIO_BIT);
}
	

//-----------------------------------------------------------------------------
//                              Public
//-----------------------------------------------------------------------------

// Reset MCU
void reset()
{
	NVIC_SystemReset();
	while(1);
}                


// Стробирование Watchdog
void clearWDT(void)
{
/*	static bool wdtst = false;
	wdtst ^= 1;
	pExtWDT.set(wdtst);  
	disable();
	LPC_WDT->FEED = 0xAA; 
	LPC_WDT->FEED = 0x55; 
	enable(); */
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, WDI_GPIO_PORT, WDI_GPIO_BIT, wdiState);
	wdiState = wdiState ? false : true;  
}


// Задержка ms миллисекунд
void delay(uint32_t ms)
{
	uint32_t t = now;
	while(!TIMEOUT(t, ms));
}


// Установить состояние сигнала RESET для ETH PHY
//
// st		true/false - Reset on/off
void setEthPhyReset(bool st)
{
	// PHY reset active low
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, PHY_RST_GPIO_PORT, PHY_RST_GPIO_BIT, !st);
}


// Установить состояние выхода управления индикатором
//
// LEDNumber		kLed0 .. kLed3
// st						true/false
void setLed(uint8_t LEDNumber, bool st)
{
	if (LEDNumber < (sizeof(gpioLEDBits) / sizeof(io_port_t)))
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpioLEDBits[LEDNumber].port, gpioLEDBits[LEDNumber].pin, st);
}


// Инициализация управления драйвером RS485-1
//
// Начальная инициализация таймера, по которому выполняется выключение драйвера RS485-1.
// 
// baud			скорость UART, бит/сек
// extra		дополнительная задержка в тактах МК
void initUart1DrvControl(uint32_t baud, uint32_t extra)
{
	Chip_TIMER_Init(LPC_TIMER1);
	Chip_RGU_TriggerReset(RGU_TIMER1_RST);
	while(Chip_RGU_InReset(RGU_TIMER1_RST)) {}
	
	// timer 1 peripheral clock rate
	uint32_t timerFreq = Chip_Clock_GetRate(CLK_MX_TIMER1);
	
	// Timer setup for interrupt on Match1
	Chip_TIMER_Reset(LPC_TIMER1);
	Chip_TIMER_MatchEnableInt(LPC_TIMER1, 1);
	Chip_TIMER_SetMatch(LPC_TIMER1, 1, (timerFreq / (baud / 10) + extra));
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER1, 1);
	Chip_TIMER_Disable(LPC_TIMER1);
	
	NVIC_EnableIRQ(TIMER1_IRQn);
	NVIC_ClearPendingIRQ(TIMER1_IRQn);	
}


// Установить состояние выхода управления драйвером RS485-1
//
// st		true/false - on/off
void setUart1Drv(bool st)
{
	Chip_GPIO_SetPinState(LPC_GPIO_PORT, U1_DIR_GPIO_PORT, U1_DIR_GPIO_BIT, st);
}


// Пуск отсчета интервала выключения драйвера RS485-1
void startUart1DrvTimer(void)
{
	Chip_TIMER_Reset(LPC_TIMER1);
	Chip_TIMER_Enable(LPC_TIMER1);
}


//
void init(void)
{
	Chip_SCU_SetPinMuxing(pinmuxing, sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));

	setupClocking();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	
  Chip_GPIO_Init(LPC_GPIO_PORT);
  Chip_ENET_RMIIEnable(LPC_ETHERNET);
	
  setEthPhyReset(false);
	initGpio();
	
	setLed(kLed0, false);
	setLed(kLed1, false);
	setLed(kLed2, false);
	setLed(kLed3, false);
	
  hal::uarts::init();
}


//
void process(void)
{
	clearWDT();
	hal::uarts::process();
}

};		// namespace hal
