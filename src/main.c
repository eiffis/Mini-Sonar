#include "servo.h"
#include <stdint.h>

static void delay(volatile uint32_t count) {
  while (count > 0U) {
    count--;
  }
}

int main(void) {
  init_servo();

  while (1) {

    for (uint16_t i = 1000U; i <= 2000U; i += 10U) {
      servo_set_us(i);
      delay(15000U);
    }

    for (uint16_t i = 2000U; i >= 1000U; i -= 10U) {
      servo_set_us(i);
      delay(15000U);
    }
  }
}
