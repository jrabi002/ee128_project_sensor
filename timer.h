#ifndef TIMER_H_
#define TIMER_H_

#include <avr/interrupt.h>
//#include "globals.h"

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
volatile unsigned char TimerFlag = 0;
volatile uint16_t timeout_cnt;

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn(void)
{
    //1ms (TOF) @ 10MHz Clock
    TCB0.CCMP = 10000;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
    TCB0.CTRLB = 0x00;
}

void TimerOff(void)
{
    TCB0.CTRLA &= ~TCB_ENABLE_bm;
}

void TimerISR(void)
{
    TimerFlag = 1;
}

ISR(TCB0_INT_vect)
{
    _avr_timer_cntcurr--;           // Count down to 0 rather than up to TOP
    if (_avr_timer_cntcurr == 0) {  // results in a more efficient compare
        TimerISR();                 // Call the ISR that the user uses
        _avr_timer_cntcurr = _avr_timer_M;
    }
    TCB0.INTFLAGS = 1;
}

void RTC_init(void)
{
    uint8_t temp;
    /* Initialize 32.768kHz Oscillator: */
    /* Disable oscillator: */
    temp =CLKCTRL.OSC32KCTRLA;
    temp &= ~CLKCTRL_RUNSTDBY_bm;
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.OSC32KCTRLA = temp;

    /* Enable oscillator: */
    temp = CLKCTRL.OSC32KCTRLA;
    temp |= CLKCTRL_RUNSTDBY_bm;
    /* Enable writing to protected register */
    CPU_CCP = CCP_IOREG_gc;
    CLKCTRL.OSC32KCTRLA = temp;

    //Wait for RTC stabilization
    while (RTC.STATUS > 0);
}

void RTC_timer_on(void)
{
    RTC.PER = 1;
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    RTC.PITINTCTRL = RTC_PI_bm; //Enable RTC Periodic Interrupts
    RTC.PITCTRLA = RTC_PERIOD_CYC32_gc | RTC_PITEN_bm;
}

//RTC periodic Interrupt ~1ms
ISR(RTC_PIT_vect)
{
    timeout_cnt++;                      // increment timeout counter
    RTC.PITINTFLAGS = RTC_PI_bm;        // clear interrupt flag
}

#endif /* TIMER_H_ */