#ifndef __BUTTON_INCLUDED__
#define __BUTTON_INCLUDED__

#include "bsp.h"

/* public define */
#define PRESS 0
#define RELEASE 1
#define MULTIPLE_CLICK_MAX_NUMBER 2

#define DEBOUNCE_INTERVAL 30
#define MULTIPLE_CLICK_INTERVAL 300
#define LONG_PRESS_INTERVAL 5000

/* public types & variables */
enum ButtonStates
{
    RELEASED = 0,
    PRESSED,
    LONG_PRESSED
};

typedef struct ButtonEvent
{
    uint8_t state;
    uint8_t click_count;
} button_event_t;

extern xQueueHandle buttonQueue;

/* public functions */
void buttonInit(uint8_t pin, uint8_t hw_timer_id);

#endif