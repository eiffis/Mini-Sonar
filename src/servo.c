#include "servo.h"
#include "stm32f429xx.h"

void init_servo(void) {
  // Activation of GPIO port B and TIMER 4;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  // Check of the retun of the clocks activations;
  (void)RCC->AHB1ENR;
  (void)RCC->APB1ENR;

  // Configure PB6 in alternate function;
  GPIOB->MODER &= ~(3U << 12);
  GPIOB->MODER |= (2U << 12);

  // Push pull;
  GPIOB->OTYPER &= ~(1U << 6);

  // no Pull Up / Pull down;
  GPIOB->PUPDR &= ~(3U << 12);

  // Select TIM4_Channel 1
  GPIOB->AFR[0] &= ~(0xFU << 24);
  GPIOB->AFR[0] |= (2U << 24);

  TIM4->CR1 = 0;
  TIM4->PSC = 15;
  TIM4->ARR = 19999;
  TIM4->CCR1 = 1500;

  // PWM mode 1, and preload of CR1 register;

  TIM4->CCMR1 &= ~((7U << 4) | (1U << 3));
  TIM4->CCMR1 |= ((6U << 4) | (1U << 3));

  // Output of canal 1;
  TIM4->CCER |= TIM_CCER_CC1E;

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

  TIM4->CCR1 = pulse_us;
  return;
}