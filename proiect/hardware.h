#ifndef HARDWARE_H
#define HARDWARE_H

#include "pico/stdlib.h"

void hardware_init(void);
void set_servo_position(uint pin, uint pulse_width_us);
void porneste_pompa(void);
void opreste_pompa(void);
long read_hx711(uint pin_sck, uint pin_dout);
float citeste_temperatura(void);

#endif // HARDWARE_H