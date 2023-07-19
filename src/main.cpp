#include "bsp.h"

#include "button/button.h"
#include "steppermotor/steppermotor.h"

#define BUTTON_HW_TIMER 0
#define BUTTON_PIN      0

#define MOTOR_HW_TIMER  1
#define MOTOR_COIL1_PIN 21
#define MOTOR_COIL2_PIN 13
#define MOTOR_COIL3_PIN 12
#define MOTOR_COIL4_PIN 25

button_event_t click_event;

/**
 * Initialization
 */
void setup()
{
  // logger
  if (CORE_DEBUG_LEVEL > 0)
  {
    Serial.begin(115200);
  }

  // init
  buttonInit(BUTTON_PIN, BUTTON_HW_TIMER);
  stepperMotorInit(MOTOR_COIL1_PIN, MOTOR_COIL2_PIN, MOTOR_COIL3_PIN, MOTOR_COIL4_PIN, MOTOR_HW_TIMER);

  // task core 0
  // task core 1
}

/**
 * Processing loop
 */
void loop()
{
  if (buttonQueue != 0)
  {
    if (xQueueReceive(buttonQueue, &click_event, portMAX_DELAY))
    {
      Serial.printf("button %d %d\r\n", click_event.state, click_event.click_count);
      if (click_event.state == RELEASED)
        stepperMotorMove(-1024);
      else if (click_event.state == LONG_PRESSED)
      {
        stepperMotorMove(1024);
      }
    }
  }
}