#include <string.h>
#include "sensor_protocol.h"

volatile uint8_t sensor_status;

void clear_sensor_packet(void)
{
    sensor_packet.reg = 0;
    sensor_packet.cmd = 0;
    memset(sensor_packet.buffer, 0, 5);
    sensor_packet.buffer_count = 0;
    sensor_packet.read_data = 0;
    sensor_packet.complete = 0;
    sensor_packet.parsed = 0;
}

void parse_sensor_packet(void)
{
    if (sensor_packet.complete)
    {
        sensor_packet.reg = sensor_packet.buffer[0];
        if (sensor_packet.reg == REG_BLINK_CMD)
            sensor_packet.cmd = sensor_packet.buffer[1];
        //This should only happen if master didn't send I2C read
        else if (sensor_packet.reg == REG_SENSOR_STATUS)
            sensor_packet.read_data = object_detected_status;
        else if (sensor_packet.reg == REG_BLINK_STATUS)
            sensor_packet.read_data = blink_status;
        else
            sensor_packet.read_data = 0xFF;
        sensor_packet.parsed = 1;
    }
}

void start_parse(void)
{
    parse_sensor_packet();
    if (sensor_packet.reg == REG_BLINK_CMD)
    {
        if (sensor_packet.cmd == CMD_BLINK_ON)
            set_blink_state = 1;
        else if (sensor_packet.cmd == CMD_BLINK_OFF)
            set_blink_state = 0;
    }
    clear_sensor_packet();
}