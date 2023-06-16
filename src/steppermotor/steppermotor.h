#ifndef __STEPPERMOTOR_INCLUDED__
#define __STEPPERMOTOR_INCLUDED__

#include "bsp.h"

void stepperMotorInit(uint8_t coil1_pin, uint8_t coil2_pin, uint8_t coil3_pin, uint8_t coil4_pin, uint8_t hw_timer_id);
void stepperMotorMove(int16_t nb_steps);

#endif