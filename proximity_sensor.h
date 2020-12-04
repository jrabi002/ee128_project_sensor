#ifndef PROXIMITY_SENSOR_H_
#define PROXIMITY_SENSOR_H_

//Timers (in increments of 1ms)
#define TIMER_EMITTER_ONOFF 5
#define TIMER_EMITTER_START_PULSE 250
#define TIMER_BLINK_GREEN_LED_ONOFF 500
#define TIMER_BLINK_SEQ 100

//Countdown Timer
#define BLINK_SEQ_TURNOFF 50

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

//The number of times the emitter should be pulsed every pulse sequence
//This define is used to set the value of 'pulse_count'
#define NUMBER_OF_EMITTER_PULSES 3

//The number of times a valid detection must occur each pulse sequence
//To consider an object detection valid
#define EMITTER_VALIDATION_COUNT (NUMBER_OF_EMITTER_PULSES - 1)

// ***** GLOBAL VARIABLES ********

//Incremented by RTC ISR (real time clock) in Timer.h
//Cleared in I2CS.c
//This variable is used to detect an I2C bus hung state, and will cause a reset on the bus at a defined  value
extern volatile uint16_t timeout_cnt;

//Set and cleared by Task2 FSM
//This variable is used to communicate the state of object detection by the sensor to the I2C Master
extern uint8_t object_detected_status;

//Set in main.c ISR
//Cleared in Task4 FSM
//This variable is requested to be set by the I2C Master, resulting in the green LED to be blinked 
//to denote the 'closest parking space'
extern volatile uint8_t set_blink_state;

//Set and cleared in Task3 FSM
//This variable is used to communicate the state of blinking LED to the I2C Master
extern uint8_t blink_status;

//Set and cleared in Task4 FSM
//This variable is used by Task3 FSM to change states and blink the green LED
extern uint8_t blink_on;

//Set, decremented, and reset by Task1 FSM
//This variable determines how many times the emitter will be pulsed every time a pulse sequence is requested by Task2 FSM
extern uint8_t pulse_count;

//Set by Task2 FSM
//Cleared by Task1 FSM
//This variable tells the Task1 FSM to begin pulsing the IR emitter up to the specified 'pulse_count'
extern uint8_t start_proximity_check;

//Set and cleared by Task1 FSM
//This variable is used with 'pulse_count' to facilitate state changes in the TASK1 FSM
extern uint8_t emitter_on;

//Incremented in main.c in analog comparator ISR
//Cleared in Task2 FSM
//This variable tracks the number of positive object detections when the emitter is pulsed
//If this variable is >= EMITTER_VALIDATION_COUNT then an object is considered detected
extern volatile uint8_t emitter_detected_count;

#endif /* PROXIMITY_SENSOR_H_ */