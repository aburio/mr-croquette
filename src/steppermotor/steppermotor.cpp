#include "steppermotor.h"

#include <AccelStepper.h>

/* local types & variables */
AccelStepper *stepper;
hw_timer_t *stepper_timer;

/* private functions prototypes */
void IRAM_ATTR stepperMotorTimerISR();

/* public functions */
void stepperMotorInit(uint8_t coil1_pin, uint8_t coil2_pin, uint8_t coil3_pin, uint8_t coil4_pin, uint8_t hw_timer_id)
{
    stepper = new AccelStepper(AccelStepper::FULL4WIRE, coil1_pin, coil2_pin, coil3_pin, coil4_pin, false);

    if (stepper != NULL)
    {
        stepper->setMaxSpeed(300);
        stepper->setAcceleration(400);

        stepper_timer = timerBegin(hw_timer_id, 80, true);
        timerAttachInterrupt(stepper_timer, &stepperMotorTimerISR, false);
    }
}

void stepperMotorMove(uint16_t nb_steps)
{
    stepper->move(nb_steps);
    stepper->enableOutputs();

    timerAlarmWrite(stepper_timer, 1000, true);
    timerAlarmEnable(stepper_timer);
}

/* private functions */
void IRAM_ATTR stepperMotorTimerISR()
{
    float _speed = 0.0;
    uint32_t _interval = 100;

    if (stepper->distanceToGo() > 0)
    {
        stepper->run();

        _speed = stepper->speed();
        if (_speed == 0)
        {
            stepper->disableOutputs();
            timerAlarmDisable(stepper_timer);
        }
        else
        {
            // compute timer next interval
            _interval = uint32_t(abs(1000 / _speed) / 2);
            timerAlarmWrite(stepper_timer, _interval * 1000, true);
        }
    }
    else
    {
        stepper->disableOutputs();
        timerAlarmDisable(stepper_timer);
    }
}