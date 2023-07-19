#include "button.h"

/* local types & variables */
typedef struct Button
{
    uint8_t gpio_pin;
    uint64_t last_update_time;
    uint8_t state;
    uint8_t click_count;
} button_t;

button_t *button;
button_event_t *button_event;
hw_timer_t *button_timer;

TaskHandle_t buttonTaskHandler;
xQueueHandle buttonQueue;

/* private functions prototypes */
void IRAM_ATTR buttonISR();
void IRAM_ATTR buttonTimerISR();
void buttonTask(void *pvParameters);

/* public functions */
void buttonInit(uint8_t pin, uint8_t hw_timer_id)
{
    button = new button_t;
    button_event = new button_event_t;

    if (button != NULL && button_event != NULL)
    {
        buttonQueue = xQueueCreate(1, sizeof(button_event_t));
        button->gpio_pin = pin;
        pinMode(button->gpio_pin, INPUT);
        attachInterrupt(button->gpio_pin, buttonISR, CHANGE);

        button_timer = timerBegin(hw_timer_id, 80, true);
        timerAttachInterrupt(button_timer, buttonTimerISR, false);
        timerAlarmWrite(button_timer, MULTIPLE_CLICK_INTERVAL * 1000, false);
        timerAlarmEnable(button_timer);

        button->state = RELEASED;
        button->click_count = 0;
        button->last_update_time = 0;

        button_event->state = RELEASED;
        button_event->click_count = 0;

        xTaskCreatePinnedToCore(buttonTask, "buttonTask", 65535, NULL, 1, &buttonTaskHandler, 0);
    }
}

/* private functions */
void IRAM_ATTR buttonISR()
{
    BaseType_t xYieldRequired;

    if (button != NULL)
    {
        uint8_t _curr_state = digitalRead(button->gpio_pin);
        uint64_t _curr_time = millis();
        uint64_t _elapsed_time = _curr_time - button->last_update_time;

        switch (button->state)
        {
        case RELEASED:
        default:
            if (_curr_state == PRESS && _elapsed_time > DEBOUNCE_INTERVAL)
            {
                ++button->click_count;
                button->state = PRESSED;
                button->last_update_time = _curr_time;
                timerWrite(button_timer, 0); // reset timer
                timerAlarmWrite(button_timer, MULTIPLE_CLICK_INTERVAL * 1000, false); // reset alarm
                timerAlarmEnable(button_timer);
                xYieldRequired = xTaskResumeFromISR(buttonTaskHandler);
            }
            break;
        
        case PRESSED:
            if (_curr_state == RELEASE && _elapsed_time > DEBOUNCE_INTERVAL)
            {
                button->state = RELEASED;
                button->last_update_time = _curr_time;
                xYieldRequired = xTaskResumeFromISR(buttonTaskHandler);
            }
            break;
        }

        portYIELD_FROM_ISR(xYieldRequired);
    }
}

void IRAM_ATTR buttonTimerISR()
{
    BaseType_t xYieldRequired;

    xYieldRequired = xTaskResumeFromISR(buttonTaskHandler);
    portYIELD_FROM_ISR(xYieldRequired);
}

void buttonTask(void *pvParameters)
{
    if (button != NULL && button_event != NULL)
    {
        bool _pause = false;

        log_v("Start");
        for(;;)
        {
            switch (button->state)
            {
                case PRESSED:
                    if (button_event->state != PRESSED)
                    {
                        log_d("Detect press");
                        button_event->state = PRESSED;
                        button_event->click_count = button->click_count;
                    }
                    else if (timerRead(button_timer) >= (MULTIPLE_CLICK_INTERVAL * 1000) && timerAlarmRead(button_timer) != (LONG_PRESS_INTERVAL * 1000))
                    {
                        // Multiple click timer is over, we reload for long press detection
                        log_d("Start long press detection");
                        timerAlarmWrite(button_timer, LONG_PRESS_INTERVAL * 1000, false);
                        timerAlarmEnable(button_timer);
                    }
                    else if (timerRead(button_timer) >= (LONG_PRESS_INTERVAL * 1000))
                    {
                        log_d("Detect long press\r\n");
                        button_event->state = LONG_PRESSED;
                        button_event->click_count = 1;
                        button->click_count = 0;
                        xQueueSend(buttonQueue, button_event, 0);
                    }
                    break;

                case RELEASED:
                default:
                    if (button_event->state != RELEASED && timerAlarmRead(button_timer) != (LONG_PRESS_INTERVAL * 1000))
                    {
                        log_d("Detect release");
                        button_event->state = RELEASED;
                        button_event->click_count = button->click_count;
                    }
                    else if (timerRead(button_timer) >= (MULTIPLE_CLICK_INTERVAL * 1000) && timerAlarmRead(button_timer) != (LONG_PRESS_INTERVAL * 1000))
                    {
                        // Multiple click timer is over and long press timer has not been armed so we send message
                        log_d("Detect full release");
                        button->click_count = 0;
                        xQueueSend(buttonQueue, button_event, 0);
                    }
                    else
                    {
                        // Long press timer has been armed so we stop it
                        log_d("Stop long press detection");
                        timerAlarmDisable(button_timer);
                        button_event->state = RELEASED;
                        button_event->click_count = button->click_count;
                        button->click_count = 0;
                        xQueueSend(buttonQueue, button_event, 0);
                    }
                    break;
            }
            log_v("Paused");
            vTaskSuspend(NULL);
        }
    }
    vTaskDelete(NULL);
}