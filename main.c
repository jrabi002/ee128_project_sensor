#include <avr/io.h>
#include <inttypes.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "I2CS.h"
#include "timer.h"
#include "scheduler.h"
#include "proximity_sensor.h"
#include "sensor_protocol.h"
#include "task1_pulse_emitter_fsm.h"
#include "task2_proximity_check_fsm.h"
#include "task3_blink_green_led_fsm.h"
#include "task4_start_blink_sequence_fsm.h"

volatile uint8_t emitter_detected_count = 0;
volatile uint8_t set_blink_state = 0;

//Analog Comparator
#define DACREF_VALUE    (1.25 * 256 / 1.5)
void AC_init(void);

//This analog comparator ISR is triggered when sensor input (POS input) goes below 1.25V
ISR(AC0_AC_vect)
{
    //both a voltage drop in the photo-transistor as well
    //confirmation IR emitter was just pulsed is required to confirm
    //valid interrupt triggered  
    
    if (emitter_on)
        emitter_detected_count++;

    //Clear AC ISR Flag
    AC0.STATUS = AC_CMP_bm;
}

//This PC Int is triggered after an I2C Stop condition is issued
ISR(PORTB_PORT_vect)
{
    //Reset PORTB[2] Pin inversion
    PORTB.PIN2CTRL &= ~(PORT_INVEN_bm);
    
    parse_sensor_packet();
    if (sensor_packet.reg == REG_BLINK_CMD)
    {
        if (sensor_packet.cmd == CMD_BLINK_ON)
            set_blink_state = 1;
        else if (sensor_packet.cmd == CMD_BLINK_OFF)
            set_blink_state = 0;
    }
    clear_sensor_packet();
    
    //Clear ISR Flag
    PORTB.INTFLAGS = PIN2_bm;
}

void AC_init(void)
{   
    //AC POS Input - Disable digital input buffer
    PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
    //Set VREF to 1.5V and output to DAC 
    VREF.CTRLA = VREF_DAC0REFSEL_1V5_gc;
    //Enable internal VREF to DAC
    VREF.CTRLB = VREF_DAC0REFEN_bm;
    //Set DAC output value to 1.25V
    DAC0_DATA = DACREF_VALUE;
    //Enable internal DAC output
    DAC0_CTRLA = DAC_ENABLE_bm;
    //Set AC POS Input - IR photo-transistor input
    //Set AC NEG Input - DAC output
    AC0.MUXCTRLA = AC_MUXPOS_PIN0_gc | AC_MUXNEG_DAC_gc;
    //Enable analog comparator, interrupt on negative edge, hysteresis 50mV
    AC0.CTRLA = AC_ENABLE_bm | AC_INTMODE_NEGEDGE_gc | AC_HYSMODE_50mV_gc;
    //Enable analog comparator ISR
    AC0.INTCTRL = AC_CMP_bm;
}

int main(void)
{
    //CPU 10MHz
    CPU_CCP = 0xD8;
    CLKCTRL_MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;
    
    //PORTA Enable Outputs
    PORTA.DIRSET = INTERRUPT_OUT;
    PORTA.DIRSET = GREEN_LED;
    PORTA.DIRSET = RED_LED;
    
    //PORTB Enable Outputs
    PORTB.DIRSET = IR_EMITTER;
     
    //PORTB PC ISR Setup
    PORTB.PIN2CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc; 
    
    //Initialize I2C Slave
    I2CS_init();
    //Initialize analog comparator
    AC_init();
    //Initialize real time clock (~1s TOF) for I2C bus timeouts
    RTC_init();

    /* SET UP TASKS */
    uint32_t SM1_t = TASK1TIME;
    uint32_t SM2_t = TASK2TIME;
    uint32_t SM3_t = TASK3TIME;
    uint32_t SM4_t = TASK4TIME;

    //Calculating GCD
    uint32_t tmpGCD = 1;
    tmpGCD = findGCD(SM1_t, SM2_t);
    tmpGCD = findGCD(tmpGCD, SM3_t);
    tmpGCD = findGCD(tmpGCD, SM4_t);

    //Greatest common divisor for all tasks or smallest time unit for tasks.
    uint32_t GCD = tmpGCD;

    //Recalculate GCD periods for scheduler
    uint32_t SM1_period = SM1_t/GCD;
    uint32_t SM2_period = SM2_t/GCD;
    uint32_t SM3_period = SM3_t/GCD;
    uint32_t SM4_period = SM4_t/GCD;

    //Declare an array of tasks
    static task task1, task2, task3, task4;
    task *tasks[] = { &task1, &task2, &task3, &task4};
    const uint16_t numTasks = sizeof(tasks)/sizeof(task*);

    // Task 1
    task1.state = -1;//Task initial state.
    task1.period = SM1_period;//Task Period.
    task1.elapsedTime = SM1_period;//Task current elapsed time.
    task1.TickFct = &SMTick1;//Function pointer for the tick.

    // task 2
    task2.state = -1;//task initial state.
    task2.period = SM2_period;//task period.
    task2.elapsedTime = SM2_period;//task current elapsed time.
    task2.TickFct = &SMTick2;//function pointer for the tick.

    // task 3
    task3.state = -1;//task initial state.
    task3.period = SM3_period;//task period.
    task3.elapsedTime = SM3_period;//task current elapsed time.
    task3.TickFct = &SMTick3;//function pointer for the tick.

    // task 4
    task4.state = -1;//task initial state.
    task4.period = SM4_period;//task period.
    task4.elapsedTime = SM4_period;//task current elapsed time.
    task4.TickFct = &SMTick4;//function pointer for the tick.

    TimerSet(GCD);
    TimerOn();
    RTC_timer_on();
    
    //Enable global interrupts
    sei();

    uint16_t i = 0;
    while (1) 
    {
        // Scheduler code
        for ( i = 0; i < numTasks; i++ )
        {
            // Task is ready to tick
            if ( tasks[i]->elapsedTime == tasks[i]->period )
            {
                // Setting next state for task
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                // Reset the elapsed time for next tick.
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += 1;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    /* Never leave main */
    return 0;
}

