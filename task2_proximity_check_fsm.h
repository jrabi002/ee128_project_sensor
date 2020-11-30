#ifndef TASK2_PROXIMITY_CHECK_FSM_H_
#define TASK2_PROXIMITY_CHECK_FSM_H_

#include "proximity_sensor.h"

uint8_t start_proximity_check = 0;
uint8_t object_detected_status = 0;

enum SM2_States { SM2_wait, SM2_proximity_check_on, SM2_proximity_check_validate};
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
        break;
        
        // ***** NEED TO ADD CHECK FOR BLINKING LED FROM MASTER ******
        case SM2_proximity_check_validate:
        if (emitter_detected_count >= EMITTER_VALIDATION_COUNT)
        {
            PORTA_OUTCLR = GREEN_LED;
            PORTA_OUTSET = RED_LED;
            object_detected_status = 1;
            emitter_detected_count = 0;
        }
        else
        {
            PORTA_OUTCLR = RED_LED;
            PORTA_OUTSET = GREEN_LED;
            object_detected_status = 0;
            emitter_detected_count = 0;
        }
        break;

        default:
        break;
    }

    return state;
}

#endif /* TASK2_PROXIMITY_CHECK_FSM_H_ */