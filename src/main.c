#include "display.h"
#include "hcsr04.h"
#include "servo.h"
#include "stm32f4xx_hal.h"
#include "system_clock.h"
#include <stdint.h>

static void delay_us(uint32_t us) {
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = us * (SystemCoreClock / 1000000U);

  while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
  }
}

void led_init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
  GPIOG->MODER &= ~(3U << (13U * 2U));
  GPIOG->MODER |= (1U << (13U * 2U));
  GPIOG->OTYPER &= ~(1U << 13U);
  GPIOG->PUPDR &= ~(3U << (13U * 2U));
}

static inline void led_update(float distance_cm) {
  if (distance_cm >= 0.0f && distance_cm <= 30.0f) {
    GPIOG->BSRR = (1U << 13U);
  } else {
    GPIOG->BSRR = (1U << (13U + 16U));
  }
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  display_init();

  init_servo();
  hcsr04_init();
  led_init();

  while (1) {
    for (uint16_t i = 1000U; i <= 2000U; i += 10U) {
      servo_set_us(i);
      delay_us(60000U);
      float distance_cm = hcsr04_measure_cm();
      led_update(distance_cm);
    }

    for (uint16_t i = 2000U; i >= 1000U; i -= 10U) {
      servo_set_us(i);
      delay_us(60000U);
      float distance_cm = hcsr04_measure_cm();
      led_update(distance_cm);
    }
  }
}
