#ifndef TASK1_PULSE_EMITTER_H_
#define TASK1_PULSE_EMITTER_H_

#include "proximity_sensor.h"

uint8_t pulse_count = NUMBER_OF_EMITTER_PULSES;
uint8_t emitter_on = 0;

//SM1 - Turn IR emitter on/off
enum SM1_States { SM1_wait, SM1_emitter_on, SM1_emitter_off, SM1_reset };
int SMTick1(int state) {

    //State machine transitions
    switch (state) {
        case SM1_wait:
        if (start_proximity_check)
            state = SM1_emitter_on;
        else
            state = SM1_wait;
        break;

        case SM1_emitter_on:
        if (pulse_count > 0 && emitter_on)
            state = SM1_emitter_off;
        else
            state = SM1_wait;
        break;

        case SM1_emitter_off:
        if (pulse_count > 0 && !emitter_on)
            state = SM1_emitter_on;
        else if (pulse_count == 0)
            state = SM1_reset;
        else
            state = SM1_wait;
        break;
        
        case SM1_reset:
        state = SM1_wait;
        break;

        default:
        state = SM1_wait; // default: Initial state
        break;
    }

    //State machine actions
    switch(state) {
        case SM1_wait:
        break;

        case SM1_emitter_on:
        PORTA_OUTSET = IR_EMITTER;
        emitter_on = 1;
        break;

        case SM1_emitter_off:
        PORTA_OUTCLR = IR_EMITTER;
        emitter_on = 0;
        pulse_count--;
        break;
        
        case SM1_reset:
        pulse_count = NUMBER_OF_EMITTER_PULSES;
        start_proximity_check = 0;
        break;

        default:
        break;
    }
    return state;
}



#endif /* TASK1_PULSE_EMITTER_H_ */