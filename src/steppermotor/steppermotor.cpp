#include "steppermotor.h"

/* local types & variables */
enum ButtonStates
{
    CLOCKWISE = 0,
    ANTICLOCKWISE
};

typedef struct Stepper
{
    int16_t nb_steps = 0;
    uint8_t curr_step = 0;
    uint8_t direction = CLOCKWISE;

    uint8_t coil1, coil2, coil3, coil4;
} stepper_t;

stepper_t *stepper_controler;
hw_timer_t *stepper_timer;

/* private functions prototypes */
void IRAM_ATTR stepperMotorTimerISR();

/* public functions */
void stepperMotorInit(uint8_t coil1_pin, uint8_t coil2_pin, uint8_t coil3_pin, uint8_t coil4_pin, uint8_t hw_timer_id)
{
    stepper_controler = new stepper_t;

    if (stepper_controler != NULL)
    {
        stepper_controler->coil1 = coil1_pin;
        pinMode(coil1_pin, OUTPUT);
        stepper_controler->coil2 = coil2_pin;
        pinMode(coil2_pin, OUTPUT);
        stepper_controler->coil3 = coil3_pin;
        pinMode(coil3_pin, OUTPUT);
        stepper_controler->coil4 = coil4_pin;
        pinMode(coil4_pin, OUTPUT);

        stepper_timer = timerBegin(hw_timer_id, 80, true);
        timerAttachInterrupt(stepper_timer, &stepperMotorTimerISR, false);
    }
}

void stepperMotorMove(int16_t nb_steps)
{
    stepper_controler->nb_steps = nb_steps;

    if (nb_steps != 0)
    {
        timerAlarmWrite(stepper_timer, 1000, true);
        timerAlarmEnable(stepper_timer);
    }
    else
    {
        timerAlarmDisable(stepper_timer);
    }
}

bool stepperMotorIsMoving()
{
    bool _moving = false;

    if (stepper_controler->nb_steps != 0)
    {
        _moving = true;
    }

    return _moving;
}

/* private functions */
void IRAM_ATTR stepperMotorTimerISR()
{
    uint32_t _interval = 100;

    if (stepper_controler->nb_steps != 0)
    {
        switch (stepper_controler->curr_step)
        {
        case 0:
        default:
            digitalWrite(stepper_controler->coil1, HIGH);
            digitalWrite(stepper_controler->coil2, HIGH);
            digitalWrite(stepper_controler->coil3, LOW);
            digitalWrite(stepper_controler->coil4, LOW);
            if (stepper_controler->nb_steps < 0)
            {
                stepper_controler->nb_steps++;
                stepper_controler->curr_step = 3;
            }
            else
            {
                stepper_controler->nb_steps--;
                stepper_controler->curr_step++;
            }
            break;
        
        case 1:
            digitalWrite(stepper_controler->coil1, LOW);
            digitalWrite(stepper_controler->coil2, HIGH);
            digitalWrite(stepper_controler->coil3, HIGH);
            digitalWrite(stepper_controler->coil4, LOW);
            if (stepper_controler->nb_steps < 0)
            {
                stepper_controler->nb_steps++;
                stepper_controler->curr_step--;
            }
            else
            {
                stepper_controler->nb_steps--;
                stepper_controler->curr_step++;
            }
            break;

        case 2:
            digitalWrite(stepper_controler->coil1, LOW);
            digitalWrite(stepper_controler->coil2, LOW);
            digitalWrite(stepper_controler->coil3, HIGH);
            digitalWrite(stepper_controler->coil4, HIGH);
            if (stepper_controler->nb_steps < 0)
            {
                stepper_controler->nb_steps++;
                stepper_controler->curr_step--;
            }
            else
            {
                stepper_controler->nb_steps--;
                stepper_controler->curr_step++;
            }
            break;

        case 3:
            digitalWrite(stepper_controler->coil1, HIGH);
            digitalWrite(stepper_controler->coil2, LOW);
            digitalWrite(stepper_controler->coil3, LOW);
            digitalWrite(stepper_controler->coil4, HIGH);
            if (stepper_controler->nb_steps < 0)
            {
                stepper_controler->nb_steps++;
                stepper_controler->curr_step--;
            }
            else
            {
                stepper_controler->nb_steps--;
                stepper_controler->curr_step = 0;
            }
            break;
        }

        if (stepper_controler->nb_steps == 0)
        {
            timerAlarmDisable(stepper_timer);
        }
        else
        {
            timerAlarmWrite(stepper_timer, 3000, true);
        }
    }
    else
    {
        timerAlarmDisable(stepper_timer);
    }
}