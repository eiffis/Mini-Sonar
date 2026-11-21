#include "hcsr04.h"
#include "stm32f429xx.h"
#include <stdint.h>

static void delay(volatile uint32_t count) {
  while (count > 0U) {
    count--;
  }
}

static void delay_us(uint32_t us) {
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = us * 16U;

  while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
  }
}

static void dwt_init(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0U;
}

void hcsr04_init(void) {

  dwt_init();
  // Activation of GPIOB and TIM2
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

  // PB4, TRIGGER in output
  GPIOB->MODER &= ~(3U << (4U * 2U));
  GPIOB->MODER |= (1U << (4U * 2U));

  GPIOB->OTYPER &= ~(1U << 4U);
  GPIOB->PUPDR &= ~(3U << (4U * 2U));
  GPIOB->OSPEEDR &= ~(3U << (4U * 2U));

  // PB4 set to LOW
  GPIOB->BSRR = (1U << (4U + 16U));

  // PB3 : ECHO in Alternate Function
  GPIOB->MODER &= ~(3U << (3U * 2U));
  GPIOB->MODER |= (2U << (3U * 2U));

  // PB3 : AF1 = TIM2_CH2
  GPIOB->AFR[0] &= ~(0xFU << (3U * 4U));
  GPIOB->AFR[0] |= (1U << (3U * 4U));

  GPIOB->PUPDR &= ~(3U << (3U * 2U));

  TIM2->CR1 = 0U;
  TIM2->PSC = 15U;
  TIM2->ARR = 0xFFFFFFFFU;
  TIM2->CNT = 0U;

  // CH2 in Input Capture direct TI2
  TIM2->CCMR1 &= ~(3U << 8U);
  TIM2->CCMR1 |= (1U << 8U);

  // Rising edge
  TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);

  // Activation of CH2
  TIM2->CCER |= TIM_CCER_CC2E;

  TIM2->EGR = TIM_EGR_UG;
  TIM2->SR = 0U;
  TIM2->CR1 |= TIM_CR1_CEN;
}

static inline void set_to_high_trigger(void) { GPIOB->BSRR = (1U << 4U); }

static inline void set_to_low_trigger(void) {
  GPIOB->BSRR = (1U << (4U + 16U));
}

static void hcsr04_set_rising_edge(void) {
  TIM2->CCER &= ~TIM_CCER_CC2E;
  TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);
  TIM2->SR &= ~TIM_SR_CC2IF;
  TIM2->CCER |= TIM_CCER_CC2E;
}
volatile uint32_t debug_hcsr04 = 0U;
volatile uint32_t debug_echo_seen = 0U;

float hcsr04_measure_cm(void) {
  uint32_t start = 0U;
  uint32_t end = 0U;
  uint32_t duration = 0U;
  uint32_t timeout_start = 0U;

  debug_hcsr04 = 0U;

  TIM2->CNT = 0U;
  TIM2->SR = 0U;

  hcsr04_set_rising_edge();

  set_to_low_trigger();
  delay_us(2U);

  set_to_high_trigger();
  delay_us(10U);
  set_to_low_trigger();

  timeout_start = TIM2->CNT;

  while ((TIM2->SR & TIM_SR_CC2IF) == 0U) {
    if ((uint32_t)(TIM2->CNT - timeout_start) >= 30000U) {
      debug_hcsr04 = 1U;
      return -1.0f;
    }
  }

  start = TIM2->CCR2;

  TIM2->CCER &= ~TIM_CCER_CC2E;
  TIM2->CCER |= TIM_CCER_CC2P;
  TIM2->SR &= ~TIM_SR_CC2IF;
  TIM2->CCER |= TIM_CCER_CC2E;

  timeout_start = TIM2->CNT;

  while ((TIM2->SR & TIM_SR_CC2IF) == 0U) {
    if ((uint32_t)(TIM2->CNT - timeout_start) >= 30000U) {
      debug_hcsr04 = 2U;
      hcsr04_set_rising_edge();
      return -1.0f;
    }
  }
  end = TIM2->CCR2;

  duration = end - start;

  float distance_cm = (float)duration / 58.0f;

  hcsr04_set_rising_edge();

  return distance_cm;
}
