#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

void init_servo(void);
void servo_set_us(uint16_t pulse_us);

#endif