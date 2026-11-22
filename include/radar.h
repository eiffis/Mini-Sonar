#ifndef RADAR_H
#define RADAR_H

#include <stdint.h>

void radar_init(void);
void radar_update(uint16_t pulse_us, float distance_cm);

#endif
