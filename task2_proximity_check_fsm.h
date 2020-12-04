#ifndef TASK2_PROXIMITY_CHECK_FSM_H_
#define TASK2_PROXIMITY_CHECK_FSM_H_

#include "proximity_sensor.h"

uint8_t start_proximity_check = 0;
uint8_t object_detected_status = 0;
uint8_t last_object_detected_status = 0;

enum SM2_States { SM2_wait, SM2_proximity_check_on, SM2_proximity_check_validate, SM2_proximity_change};
int SMTick2(int state) {

    //State machine transitions
    switch (state) {
        case SM2_wait:
            state = SM2_proximity_check_on;
        break;

        case SM2_proximity_check_on:
            state = SM2_proximity_check_validate;
        break;

        case SM2_proximity_check_validate:
            if (object_detected_status != last_object_detected_status)
                state = SM2_proximity_change;
            else
                state = SM2_proximity_check_on;
        break;
        
        case SM2_proximity_change:
        state = SM2_proximity_check_on;
        break;

        default:
        state = SM2_wait; // default: Initial state
        break;
    }

    //State machine actions
    switch(state) {
        case SM2_wait:
        break;

        case SM2_proximity_check_on:
            start_proximity_check = 1;
            last_object_detected_status = object_detected_status;
        break;
        
        case SM2_proximity_check_validate:
        if (emitter_detected_count >= EMITTER_VALIDATION_COUNT)
        {
            if (!blink_on)
            {
                PORTA_OUTCLR = GREEN_LED;
                PORTA_OUTSET = RED_LED;
            }
            else
                PORTA_OUTCLR = RED_LED;
         
            object_detected_status = 1;
            emitter_detected_count = 0;
        }
        else
        {
            if (!blink_on)
            {
                 PORTA_OUTCLR = RED_LED;
                 PORTA_OUTSET = GREEN_LED;
            }
            else
                PORTA_OUTCLR = RED_LED;
            
            object_detected_status = 0;
            emitter_detected_count = 0;
        }
        break;
        
        case SM2_proximity_change:
        PORTA_OUTSET = INTERRUPT_OUT;
        PORTA_OUTCLR = INTERRUPT_OUT;

        default:
        break;
    }

    return state;
}

#endif /* TASK2_PROXIMITY_CHECK_FSM_H_ */