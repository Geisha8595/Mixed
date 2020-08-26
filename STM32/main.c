/* THE CODE HAS NOT YET BEEN TESTED!!!

   I'm using stm32l412k8t6
   
   The files that are needed for compiling and linking
   
   cmsis_compiler.h
   cmsis_gcc.h
   cmsis_version.h
   core_cm4.h
   mpu_armv7.h
   startup_stm32l412xx.s
   stm32l4xx.h
   stm32l412xx.h
   STM32L412K8T6_FLASH.ld
   system_stm32l4xx.c
   system_stm32l4xx.h
   
   These files are found in the stm32cube package which can be downloaded from STMicroelectronics's web site
*/

// sysbolic names for the registers
#include "stm32l412xx.h"

int main(){

    // turning the red led on (just for testing)

    // enbling clocking on GPIOA
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // configuring pa12 in general purpose output mode
    GPIOA->MODER &= ~0x2000000;

    // turning pa12 high (led is turned on)
    GPIOA->ODR |= 0x1000;

    // enabling prefetch mode
    FLASH->ACR |= 0x100;
    
    // configuring the microcontroller for 2 wait states
    FLASH->ACR |= 0x2;
	
    /* at startup MSI-oscilator, at 4 MHz, is selected as system clock

       Internal High Speed oscilator (HSI16) at 16 Mhz is turned on */
    RCC->CR = RCC_CR_HSION;

    // waiting for the HSI16 to lock
    while(!(RCC->CR & RCC_CR_HSIRDY));
    
    // configuring the PLL

    // resetting PLL configuration register
    RCC->PLLCFGR &= 0x00000000;
    RCC->PLLCFGR |= 0x00001000;

    // selecting HSI16 as clock source */
    RCC->PLLCFGR |= 0x2;

    /* leaving the PLLM divider as 1 (default) 
       the VCO (voltage controlled oscillator) input freqeuncy has to 
       be between 4 and 16 MHz */

    // setting the PLLN mutltiplier to 8
    RCC->PLLCFGR &= ~0x1000;
    RCC->PLLCFGR |= 0x800;

    // leaving the PLLR divider as 2 (default)

    // enabling the PLLCLK output (system clock)
    RCC->PLLCFGR |= 0x1000000;
	
    // RCC->PLLCFGR |= 0x1000000; // Main PLL PLLCLK output enable ?

    // enabling PLL (Main PLL enable)
    RCC->CR |= RCC_CR_PLLON;

    // waiting for the PLL to lock
    while(!(RCC->CR & RCC_CR_PLLRDY));
	
    // selecting the PLL as system clock source
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
	
    // waiting for the system clock to lock
    while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
	
    /* setting the global clock variable
       SystemCoreClockUpdate() could be called from system_stm32l4xx.c */
    SystemCoreClock = 64000000;

    // disabling MSI clock 
    RCC->CR &= ~0x1;

    // enabling clocking on TIM1
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    /* enabling clocking on GPIOA. The output of TIM1 can be mapped to pins
       PA8, PA9, PA10 and PA11 and those pins are connected to PORT A, which
       is connected to AHB2 bus */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // configuring the pins

    /* marking pins PA8, PA9, PA10 and PA11 as alternate function pins
       reset value: 0xABFF FFFF
       alternate function mode: 0x10 
       p. 267 reference manual */
    GPIOA->MODER &= ~0x10000;  // PA8
    GPIOA->MODER &= ~0x40000;  // PA9
    GPIOA->MODER &= ~0x100000; // PA10
    GPIOA->MODER &= ~0x400000; // PA11

    /* setting TIM1 as alternate function for the selected pins
       reset value: 0x0000 0000
       TIM1: 0x1
       p. 271 reference manual, p. 63 datasheet */
    GPIOA->AFR[1] |= 0x1;    // PA8,  TIM1_CH1
    GPIOA->AFR[1] |= 0x10;   // PA9,  TIM1_CH2
    GPIOA->AFR[1] |= 0x100;  // PA10, TIM1_CH3
    GPIOA->AFR[1] |= 0x1000; // PA11, TIM1_CH4

    /* setting the selected pins' output to "very high speed" (100 Hz)
       reset value: 0x0C00 0000
       100 Hz: 0x11
       p.268 reference manual */
    GPIOA->OSPEEDR |= 0x30000;  // PA8    
    GPIOA->OSPEEDR |= 0xC0000;  // PA9
    GPIOA->OSPEEDR |= 0x300000; // PA10
    GPIOA->OSPEEDR |= 0xC00000; // PA11

    // configuring the timer 1

    /* setting TIM1 prescaler

       TIM1 is running of PCLK which is running of HCLK, which
       is running of SYSCLK (64 MHz)
       PCLK prescaler (divider: 1 (default)
       HCLK prescaler (divider): 1 (default)

       maximum frequency for both PCLK and HCLK is 80 MHz

       desired pwm frequency: 16 KHz
       desired pwm steps: 100
       TIM1 frequency: x
       
       (1/x) * 100 = 1/16 000
       100/x = 0.0000625
       x = 1 600 000 Hz
       TIM1 prescaler: 64 000 000 / 1 600 000 - 1 = 39 = 0x27 */
    TIM1->PSC = 0x27;

    // setting auto reload value to (100 - 1)
    TIM1->ARR = 0x63;

    // setting MOE bit for pwm (BDTR applies only to advanced timers)
    TIM1->BDTR |= 0x8000;

    // setting normal pwm mode (not inverted) for channels
    TIM1->CCMR1 |= 0x60;   // channel 1
    TIM1->CCMR1 |= 0x6000; // channel 2
    TIM1->CCMR2 |= 0x60;   // channel 3
    TIM1->CCMR2 |= 0x6000; // channel 4

    /* enabling capture/compare outputs as active high (high up to duty sycle and off
       for the remainder of the period) */
    TIM1->CCER |= 0x1;    // capture/compare 1
    TIM1->CCER |= 0x10;   // capture/compare 2
    TIM1->CCER |= 0x100;  // capture/compare 3
    TIM1->CCER |= 0x1000; // capture/compare 4

    /* setting the ug bit resets the timer's counter and tells it to use the new 
       prescaler/autoreload values */
    TIM1->EGR |= 0x3;

    // setting motors to 50 %
    TIM1->CCR1 = 0x32;
    TIM1->CCR2 = 0x32;
    TIM1->CCR3 = 0x32;
    TIM1->CCR4 = 0x32;

    // enabling the counter (pins start outputting pwm signal)
    TIM1->CR1 |= 0x1;

    // delay (could and propably should be longer)
    for(int i = 0; i < 500000000; i++);

    // disabling the timer and the output of pwm signal
    TIM1->CR1 &= ~0x1;
}
