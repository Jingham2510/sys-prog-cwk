#include "config.h"
#include "stm32f3xx.h"

/* Configures the clock to use HSE (external oscillator) and the PLL
	to get SysClk == AHB == APB1 == APB2 == 36MHz */
static void _configClock(void) {
	/* Enable HSE */
	RCC->CR |= RCC_CR_HSEON;
	/* Wait for the HSE to be ready (should be instant, externally clocked) */
	while(!(RCC->CR & RCC_CR_HSERDY));
	
	/* Configure the PLL */
  /* Reset PREDIV[3:0] bits */
  RCC->CFGR2 &= ~RCC_CFGR2_PREDIV;
	/* Configure /2 prescale (4MHz input) */
	RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV2;
	/* Set HSE as the PLL source */
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
	/* Set PLLMUL[3:0] bits to 0111b for PLL x9 (4MHz in, 36MHz out) */
	RCC->CFGR |= RCC_CFGR_PLLMUL5;
	/* Enable the main PLL */
	RCC->CR |= RCC_CR_PLLON;

	/* Wait for the PLL to be ready */
	while(!(RCC->CR & RCC_CR_PLLRDY));
 
	/* Select the PLL as system clock source */
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Wait till the PLL is used as system clock source */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	
	/* TEMPORARY: Enable MCO for testing */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;	/* Enable USART2 Clock */
	GPIOA->MODER &= ~GPIO_MODER_MODER8;
  GPIOA->MODER |=  GPIO_MODER_MODER8_1;		/* Setup MCO pin for Alternate Function */
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8_Msk;
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFRH0_Msk;
	RCC->CFGR |= RCC_CFGR_PLLNODIV;
	RCC->CFGR &= ~RCC_CFGR_MCOPRE;
	RCC->CFGR |= RCC_CFGR_MCO;

	SystemCoreClockUpdate();
}

static void _configUSART2(uint32_t baud) {
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;		/* Enable GPIOA */
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
