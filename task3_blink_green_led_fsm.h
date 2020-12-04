#ifndef TASK3_BLINK_GREEN_LED_FSM_H_
#define TASK3_BLINK_GREEN_LED_FSM_H_

#include "proximity_sensor.h"


uint8_t blink_status = 0;

enum SM3_States { SM3_wait, SM3_LED_on, SM3_LED_off, SM3_reset};
int SMTick3(int state) {

    //State machine transitions
    switch (state) {
        case SM3_wait:
        if (blink_on)
            state = SM3_LED_on;
        else
            state = SM3_wait;
        break;

        case SM3_LED_on:
        if (blink_on)
            state = SM3_LED_off;
        else
            state = SM3_reset;
        break;

        case SM3_LED_off:
        if (blink_on)
            state = SM3_LED_on;
        else
            state = SM3_reset;
        break;
        
        case SM3_reset:
        state = SM3_wait;
        break;
        
        default:
        state = SM3_wait; // default: Initial state
        break;
    }

    //State machine actions
    switch(state) {
        case SM3_wait:
        break;

        case SM3_LED_on:
        blink_status = 1;
        PORTA_OUTCLR = RED_LED;
        PORTA_OUTSET = GREEN_LED;
        break;
        
        case SM3_LED_off:
        PORTA_OUTCLR = GREEN_LED;
        PORTA_OUTCLR = RED_LED;
        break;
        
        case SM3_reset:
        blink_status = 0;
        break;
        
        default:
        break;
    }

    return state;
}




#endif /* TASK3_BLINK_GREEN_LED_FSM_H_ */