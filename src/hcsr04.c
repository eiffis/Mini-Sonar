#include "hcsr04.h"
#include "stm32f429xx.h"

void hcsr04_init(void) {
  // Activation of GPIO port A, and TIMER 2;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // Set the PA1 in output mode;
  GPIOA->MODER &= ~(3U << 2U);
  GPIOA->MODER |= (1U << 2U);

  // Push pull output;
  GPIOA->OTYPER &= ~(1U << 1U);

  // Output speed in medium;
  GPIOA->OSPEEDR &= ~(3U << 2U);
  GPIOA->OSPEEDR |= (1U << 2U);

  // No pull-up / pull-down;
  GPIOA->PUPDR &= ~(3U << 2U);

  // Initial state to LOW;
  GPIOA->BSRR = (1U << (1U + 16U));

  // Configure PA0 in alternate function for ECHO;
  GPIOA->MODER &= ~(3U << 0U);
  GPIOA->MODER |= (2U << 0U);

  // PA0 in TIM2_Channel 1;
  GPIOA->AFR[0] &= ~(0xFU << 0U);
  GPIOA->AFR[0] |= (1U << 0U);

  // No pull-up / pull-down;
  GPIOA->PUPDR &= ~(3U << 0U);

  // Configuration of TIMER 2;
  TIM2->CR1 = 0U;

  // 16 MHz / (15 + 1) = 1 MHz;
  TIM2->PSC = 15U;

  // Counter 32 bits;
  TIM2->ARR = 0xFFFFFFFFU;

  // Counter set to 0;
  TIM2->CNT = 0U;

  // Configuration of CH1 in Input Capture;
  // CC1S = 01 : direct input TI1;
  TIM2->CCMR1 &= ~(3U << 0U);
  TIM2->CCMR1 |= (1U << 0U);

  // Capture in rising edge;
  // CC1P = 0 : Rising edge;
  TIM2->CCER &= ~(1U << 1U);

  // Activation of CH1 capture;
  TIM2->CCER |= (1U << 0U);

  // prescaler load;
  TIM2->EGR = TIM_EGR_UG;

  // Erase status flags;
  TIM2->SR = 0U;

  // Start TIM2;
  TIM2->CR1 |= TIM_CR1_CEN;
}
