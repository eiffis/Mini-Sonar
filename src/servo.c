#include "servo.h"
#include "stm32f429xx.h"

void init_servo(void) {
  // Activation of GPIO port B and TIMER 4;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  // Check of the retun of the clocks activations;
  (void)RCC->AHB1ENR;
  (void)RCC->APB1ENR;

  // Configure PB7 in alternate function (PB6 is used by the SDRAM);
  GPIOB->MODER &= ~(3U << 14);
  GPIOB->MODER |= (2U << 14);

  // Push pull;
  GPIOB->OTYPER &= ~(1U << 7);

  // no Pull Up / Pull down;
  GPIOB->PUPDR &= ~(3U << 14);

  // Select TIM4_Channel 2
  GPIOB->AFR[0] &= ~(0xFU << 28);
  GPIOB->AFR[0] |= (2U << 28);

  TIM4->CR1 = 0;
  TIM4->PSC = 89; // 90 MHz timer clock -> 1 MHz
  TIM4->ARR = 19999;
  TIM4->CCR2 = 1500;

  // PWM mode 1, and preload of CCR2 register;

  TIM4->CCMR1 &= ~((7U << 12) | (1U << 11));
  TIM4->CCMR1 |= ((6U << 12) | (1U << 11));

  // Output of canal 2;
  TIM4->CCER |= TIM_CCER_CC2E;

  // Preload of ARR;
  TIM4->CR1 |= TIM_CR1_ARPE;

  // Preload values;
  TIM4->EGR = TIM_EGR_UG;

  // Start timer;
  TIM4->CR1 |= TIM_CR1_CEN;

  return;
}

void servo_set_us(uint16_t pulse_us) {

  if (pulse_us < 1000U) {
    pulse_us = 1000U;
  }

  if (pulse_us > 2000U) {
    pulse_us = 2000U;
  }

  TIM4->CCR2 = pulse_us;
  return;
}