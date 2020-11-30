#ifndef PROXIMITY_SENSOR_H_
#define PROXIMITY_SENSOR_H_

//Timers (in increments of 1ms)
#define TIMER_EMITTER_ONOFF 50
#define TIMER_EMITTER_START_PULSE 500
#define TIMER_BLINK_GREEN_LED_ONOFF 500
#define TIMER_BLINK_SEQ 5000

//TASK Timers
#define TASK1TIME TIMER_EMITTER_ONOFF
#define TASK2TIME TIMER_EMITTER_START_PULSE
#define TASK3TIME TIMER_BLINK_GREEN_LED_ONOFF
#define TASK4TIME TIMER_BLINK_SEQ

//PORTA Defines
#define RED_LED PIN1_bm
#define GREEN_LED PIN2_bm
#define IR_EMITTER PIN3_bm
#define INTERRUPT_OUT PIN4_bm

#define NUMBER_OF_EMITTER_PULSES 3
#define EMITTER_VALIDATION_COUNT (NUMBER_OF_EMITTER_PULSES - 1)

extern volatile uint16_t timeout_cnt;

extern uint8_t object_detected_status;

extern volatile uint8_t set_blink_state;
extern uint8_t blink_status;
extern uint8_t blink_on;

extern uint8_t pulse_count;
extern uint8_t start_proximity_check;
extern uint8_t emitter_on;
extern volatile uint8_t emitter_detected_count;



#endif /* PROXIMITY_SENSOR_H_ */