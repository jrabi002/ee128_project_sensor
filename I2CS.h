#define F_CPU 10000000UL

#ifndef I2CS_H_
#define I2CS_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "sensor_protocol.h"
#include "proximity_sensor.h"

#define NOP() asm volatile(" nop \r\n")         
#define SLAVE_ADDRESS   0x1A
//#define SLAVE_ADDRESS 0x1B    
//#define SLAVE_ADDRESS 0x1C            
#define MAX_TRANSACTION 15      

//I2C PORTB
#define SCL 0
#define SDA 1

void I2CS_init(void);   // initialize I2C slave
void I2C_error_handler(void);

#endif /* I2CS_H_ */