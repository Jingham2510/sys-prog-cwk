#include "config.h"
#include "stm32f4xx.h"

/* Configures the clock to use HSE (external oscillator) and the PLL
	to get SysClk == AHB == APB1 == APB2 == 36MHz */
static void _configClock(void) {
	// Enable the HSE in bypass mode (there is a 8MHz signal coming from the ST-LINK)
	RCC->CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));

	//Set the main PLL M, N, P, Q, R, and HSE as the input
	//STM32F4DISCOVERY: M = 4, N = 168, P = 2 = 168Mhz SYSCLK, Q = 7, R = N/A (Q & R not used)
	RCC->PLLCFGR = (PLL_M << RCC_PLLCFGR_PLLM_Pos) | (PLL_N << RCC_PLLCFGR_PLLN_Pos) | (PLL_P << RCC_PLLCFGR_PLLP_Pos) | (PLL_Q << RCC_PLLCFGR_PLLQ_Pos) | RCC_PLLCFGR_PLLSRC_HSE;
	RCC->CR |= RCC_CR_PLLON;

	// set APB prescalers
	// APB1 = 4, APB2 = 2
	//STM32F4DISCOVERY: 42Mhz and 84Mhz
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2;

	// wait for PLL lock
	while(!(RCC->CR & RCC_CR_PLLRDY));

	// Switch SYSCLK to PLL
	RCC->CFGR |= RCC_CFGR_SW_HSE;

	/* Configure Flash prefetch, Instruction cache, Data cache and wait state */
	FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

	/* Select the main PLL as system clock source */
	RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Wait till the main PLL is used as system clock source */
	while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);

	SystemCoreClockUpdate();
}

static void _configUSART2(uint32_t baud) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;		/* Enable GPIOA */
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;	/* Enable USART2 Clock */

	GPIOA->MODER &= ~GPIO_MODER_MODER2;
  GPIOA->MODER |=  GPIO_MODER_MODER2_1;	/* Setup TX pin (GPIOA_2) for Alternate Function */

  GPIOA->AFR[0] |= (7 << (4*2));				/* Setup USART TX as the Alternate Function */

  USART2->CR1 |= USART_CR1_UE;					/* Enable USART */

	/* APB1 clock is derived from AHB clock, which is derived from SYSCLK */
	static uint32_t ppre1[] = {1, 1, 1, 1, 2, 4, 8, 16};
	static uint32_t hpre[] = {1, 1, 1, 1, 1, 1, 1, 1, 2, 4, 8, 16, 64, 128, 256, 512};
	uint32_t ahbclock = SystemCoreClock / hpre[(RCC->CFGR & RCC_CFGR_HPRE_Msk) >> RCC_CFGR_HPRE_Pos];
  uint32_t apb1clock = ahbclock / ppre1[(RCC->CFGR & RCC_CFGR_PPRE1_Msk) >> RCC_CFGR_PPRE1_Pos];
  USART2->BRR = (uint16_t)(apb1clock / baud);

  USART2->CR1 |= USART_CR1_TE;					/* Enable Tx */
}

void config_init(void) {
	_configClock();
	_configUSART2(38400);
}
