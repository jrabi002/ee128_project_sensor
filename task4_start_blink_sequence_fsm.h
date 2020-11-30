#ifndef TASK4_START_BLINK_SEQUENCE_FSM_H_
#define TASK4_START_BLINK_SEQUENCE_FSM_H_

#include "proximity_sensor.h"

uint8_t blink_on = 0;

enum SM4_States { SM4_wait, SM4_blinking_on, SM4_blinking_off};
int SMTick4(int state) {

    //State machine transitions
    switch (state) {
        case SM4_wait:
        if (set_blink_state)
            state = SM4_blinking_on;
        break;

        case SM4_blinking_on:
        state = SM4_blinking_off;
        break;

        case SM4_blinking_off:
        state = SM4_wait;
        break;

        default:
        state = SM4_wait; // default: Initial state
        break;
    }

    //State machine actions
    switch(state) {
        case SM4_wait:
        break;

        case SM4_blinking_on:
        blink_on = 1;
        break;
        
        case SM4_blinking_off:
        set_blink_state = 0;
        break;

        default:
        break;
    }

    return state;
}



#endif /* TASK4_START_BLINK_SEQUENCE_FSM_H_ */