#ifndef __BUTTON_INCLUDED__
#define __BUTTON_INCLUDED__

#include "bsp.h"

/* public define */
#define PRESS 0
#define RELEASE 1
#define MULTIPLE_CLICK_MAX_NUMBER 2

#define DEBOUNCE_INTERVAL 20
#define MULTIPLE_CLICK_INTERVAL 300
#define LONG_PRESS_DURATION 5000

/* public types & variables */
extern xQueueHandle buttonQueue;

/* public functions */
void buttonInit(uint8_t pin, uint8_t hw_timer_id);

#endif