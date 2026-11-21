#include "servo.h"
#include <stdint.h>

int main(void) {
  init_servo();

  while (1) {

    servo_set_us(1000);
    for (volatile uint32_t i = 0; i < 3000000; i++)
      ;

    servo_set_us(1500);
    for (volatile uint32_t i = 0; i < 3000000; i++)
      ;

    servo_set_us(2000);
    for (volatile uint32_t i = 0; i < 3000000; i++)
      ;
  }
}
