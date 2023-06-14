#include "button.h"

/* local types & variables */
enum ButtonStates
{
    RELEASED = 0,
    PRESSED,
    LONG_PRESSED
};

typedef struct Button
{
    uint8_t gpio_pin;
    uint8_t state = RELEASED;
    uint8_t click_count = 0;
} button_t;

button_t *button;
hw_timer_t *button_timer;

xQueueHandle buttonQueue;

/* private functions prototypes */
void IRAM_ATTR buttonISR();

/* public functions */
void buttonInit(uint8_t pin, uint8_t hw_timer_id)
{
    button = new button_t;

    if (button != NULL)
    {
        buttonQueue = xQueueCreate(2, sizeof(uint8_t));
        button->gpio_pin = pin;
        pinMode(button->gpio_pin, INPUT);
        attachInterrupt(button->gpio_pin, buttonISR, CHANGE);

        button_timer = timerBegin(hw_timer_id, 80, true);
        timerAttachInterrupt(button_timer, buttonISR, false);
        timerAlarmWrite(button_timer, LONG_PRESS_DURATION * 1000, false);
    }
}

/* private functions */
void IRAM_ATTR buttonISR()
{
    uint8_t _curr_button_value = digitalRead(button->gpio_pin);
    uint32_t _curr_time = millis();

    switch (button->state)
    {
    case PRESSED:
        if (_curr_button_value == PRESS)
        {
            if (timerRead(button_timer) >= LONG_PRESS_DURATION * 1000)
            {
                button->state = LONG_PRESSED;
                xQueueSendFromISR(buttonQueue, &button->click_count, NULL);
            }
        }
        else if (timerRead(button_timer) > DEBOUNCE_INTERVAL)
        {
            button->state = RELEASED;
        }
        break;

    case LONG_PRESSED:
        if (_curr_button_value == RELEASE)
        {
            button->state = RELEASED;
            button->click_count = 0;
            timerAlarmDisable(button_timer);
        }
        break;

    case RELEASED:
    default:
        if (_curr_button_value == PRESS && (timerAlarmEnabled(button_timer) != true || timerRead(button_timer) > DEBOUNCE_INTERVAL * 1000))
        {
            if (button->click_count != 0 && timerRead(button_timer) > MULTIPLE_CLICK_INTERVAL * 1000)
            {
                button->click_count = 1;
            }
            else
            {
                ++button->click_count;
            }
            
            button->state = PRESSED;
            xQueueSendFromISR(buttonQueue, &button->click_count, NULL);
            timerAlarmEnable(button_timer);
            timerRestart(button_timer);
        }
        else
        {
            button->click_count = 0;
            timerAlarmDisable(button_timer);
        }
        break;
    }
}