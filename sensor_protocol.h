#ifndef SENSOR_PROTOCOL_H_
#define SENSOR_PROTOCOL_H_

#include <inttypes.h>
#include "proximity_sensor.h"

typedef enum
{
    REG_SENSOR_STATUS = 0x01,
    REG_BLINK_STATUS = 0x02,
    REG_BLINK_CMD = 0x03,
} sensorReg_t;

typedef enum
{
    CMD_BLINK_OFF = 0x00,
    CMD_BLINK_ON = 0x01,    
} sensorCmd_t;

struct sensor_packet_struct
{
    sensorReg_t reg;
    sensorCmd_t cmd;
    uint8_t buffer[5];
    uint8_t buffer_count;
    uint8_t read_data;
    uint8_t complete;
    uint8_t parsed;
}sensor_packet;

void clear_sensor_packet(void);
void parse_sensor_packet(void);


#endif /* SENSOR_PROTOCOL_H_ */