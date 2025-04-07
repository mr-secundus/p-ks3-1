#include <string.h>
#include "hal.h"
#include "hal_uarts.h"
#include "cfg/defines.h"

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
	// 
	// На схеме выход управления драйвером RS485-1 (RTS1) подключен к P7.0, который можно 
	// использовать только как GPIO. Аппаратный выход управления драйвером U3_DIR есть 
	// на P4.4, который на схеме LED0. Решение - исправления на плате:
	//   P4.4 -> RTS1
	//   P3.2 -> LED0
	//
	{0x3, 4, (SCU_MODE_INACT | SCU_MODE_FUNC4)},
	{0x3, 5, (SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC4)},
	{0x4, 4, (SCU_MODE_INACT | SCU_MODE_FUNC6)},			// P4.4  U3_DIR
	
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

	/* ENET Pin mux (RMII Pins) */
	{0x1, 18, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, /* TXD0 */
	{0x1, 20, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, /* TXD1 */
	{0x0,  1, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC6)}, /* TXEN */
	{0x1, 15, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, /* RXD0 */
	{0x0,  0, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2)},  /* RXD1 */
	{0x1, 16, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC7)}, /* CRS_DV */
	{0x1, 17, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC3)}, /* MDIO */
	{0x2,  0, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC7)}, /* MDC */
	{0x1, 19, (SCU_MODE_HIGHSPEEDSLEW_EN | SCU_MODE_INACT | SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC0)}, /* REFCLK */
};

typedef struct
{
	uint8_t port;
	uint8_t pin;
} io_port_t;

static const io_port_t gpioLEDBits[] = {{3, 2}, {4, 5}, {4, 6}, {4, 8}};

volatile uint32_t ticks = 0;

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


//
void initLed()
{
	uint32_t idx;
	for (idx = 0; idx < (sizeof(gpioLEDBits) / sizeof(io_port_t)); ++idx) 
	{
		Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, gpioLEDBits[idx].port, gpioLEDBits[idx].pin);
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpioLEDBits[idx].port, gpioLEDBits[idx].pin, (bool) true);
	}
}
	

//-----------------------------------------------------------------------------
//                              Public
//-----------------------------------------------------------------------------

//
void delay(uint32_t ms)
{
	uint32_t t = now;
	while(!TIMEOUT(t, ms));
}


//
void setLed(uint8_t LEDNumber, bool st)
{
	if (LEDNumber < (sizeof(gpioLEDBits) / sizeof(io_port_t)))
		Chip_GPIO_SetPinState(LPC_GPIO_PORT, gpioLEDBits[LEDNumber].port, gpioLEDBits[LEDNumber].pin, st);
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
	
  hal::uarts::init();

	initLed();
	
	setLed(kLed0, false);
	setLed(kLed1, false);
	setLed(kLed2, false);
	setLed(kLed3, false);
}


//
void process(void)
{
	hal::uarts::process();
}

};		// namespace hal
