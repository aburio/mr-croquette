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

uint8_t state;

/**
 * Initialization
 */
void setup()
{
  // logger
  if (CORE_DEBUG_LEVEL == 6)
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
    if (xQueueReceive(buttonQueue, &state, portMAX_DELAY))
    {
      Serial.printf("button %d\r\n", state);
      stepperMotorMove(1024);
    }
  }
}