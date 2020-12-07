#include "I2CS.h"
//#include "globals.h"

volatile uint8_t i2c_data;        // data byte sent by slave or received from master
uint8_t  num_bytes = 0;              // number of bytes sent/received in transaction



void I2CS_init(void)                                            // initialize slave
{
    //Clear and setup SCL
    VPORTB.DIR |= (1 << SCL);
    VPORTB.OUT &= ~(1 << SCL);
    
    volatile uint8_t *port_pin_ctrl = ((uint8_t *)&PORTB + 0x10 + SCL);
    *port_pin_ctrl |= PORT_PULLUPEN_bm;
    *port_pin_ctrl = ((uint8_t *)&PORTB + 0x10 + SCL);
    *port_pin_ctrl &= ~PORT_INVEN_bm;
    *port_pin_ctrl = ((uint8_t *)&PORTB + 0x10 + SCL);
    *port_pin_ctrl = (*port_pin_ctrl & ~PORT_ISC_gm) | PORT_ISC_INTDISABLE_gc;

    //Clear and setup SDA
    VPORTB.DIR |= (1 << SDA);
    VPORTB.OUT &= ~(1 << SDA);
    *port_pin_ctrl = ((uint8_t *)&PORTB + 0x10 + SDA);
    *port_pin_ctrl |= PORT_PULLUPEN_bm;
    *port_pin_ctrl = ((uint8_t *)&PORTB + 0x10 + SDA);
    *port_pin_ctrl &= ~PORT_INVEN_bm;
    *port_pin_ctrl = ((uint8_t *)&PORTB + 0x10 + SDA);
    *port_pin_ctrl = (*port_pin_ctrl & ~PORT_ISC_gm) | PORT_ISC_INTDISABLE_gc;

    TWI0.CTRLA = 0 << TWI_FMPEN_bp                  // FM Plus Enable: disabled
                 | TWI_SDAHOLD_50NS_gc               // Typical 50ns hold time
                 | TWI_SDASETUP_8CYC_gc;             // SDA setup time is 8 clock cycles

    //TWI0.DBGCTRL = TWI_DBGRUN_bm; //Debug Run: disabled

    TWI0.SADDR = SLAVE_ADDRESS << 1                 // Slave Address (8 bit address, i. e. bit 0 = 0, will be substituted by R/W bit)
                 | 0 << TWI_ADDREN_bp;               // General Call Recognition Enable: disabled

    TWI0.SCTRLA = 1 << TWI_APIEN_bp                 // Address/Stop Interrupt Enable: enabled
                  | 1 << TWI_DIEN_bp                // Data Interrupt Enable: enabled
                  | 1 << TWI_ENABLE_bp              // Enable TWI Slave: enabled
                  | 1 << TWI_PIEN_bp                // Stop Interrupt Enable: enabled
                  | 0 << TWI_PMEN_bp                // Promiscuous Mode Enable: disabled
                  | 0 << TWI_SMEN_bp;               // Smart Mode Enable: disabled
}

void I2C_error_handler()                            // error handler, should reset I2C slave internal state
{
    TWI0.SSTATUS |=
        TWI_APIF_bm
        | TWI_DIF_bm;                          // clear interrupt flags

    VPORTB.DIR &= ~(1 << SCL);
    VPORTB.DIR &= ~(1 << SDA);

    TWI0.SCTRLA = 0;                                // disable slave
    _delay_us(10);                                  

    VPORTB.DIR |= (1 << SCL);
    VPORTB.DIR |= (1 << SDA);
    // re-enable slave
    TWI0.SCTRLA = 1 << TWI_APIEN_bp                 // Address/Stop Interrupt Enable: enabled
                  | 1 << TWI_DIEN_bp                // Data Interrupt Enable: enabled
                  | 1 << TWI_ENABLE_bp              // Enable TWI Slave: enabled
                  | 1 << TWI_PIEN_bp                // Stop Interrupt Enable: enabled
                  | 0 << TWI_PMEN_bp                // Promiscuous Mode Enable: disabled
                  | 0 << TWI_SMEN_bp;               // Smart Mode Enable: disabled
    TWI0.SCTRLB = TWI_ACKACT_NACK_gc
                  | TWI_SCMD_NOACT_gc;              // set NACK
}

//I2C slave interrupt
ISR(TWI0_TWIS_vect)
{
    timeout_cnt = 0;
    
    //sleep_tracker = 0;
    
    // APIF && DIF, invalid state
    if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_DIF_bm))
    {
        I2C_error_handler();
        return;
    }

    // Collision - slave has not been able to transmit a high data or NACK bit
    if (TWI0.SSTATUS & TWI_COLL_bm)
    {
        I2C_error_handler();
        return;
    }

    // Bus Error - illegal bus condition
    if (TWI0.SSTATUS & TWI_BUSERR_bm)
    {
        I2C_error_handler();
        return;
    }

    // APIF && AP - valid address has been received
    if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_AP_bm))
    {
        num_bytes = 0;

        TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc; // send ACK
        
        //Master Read:
        //(Start->Write(addr)->Write(reg)->Repeat Start->Read(addr)->Read(NACK)
        if (TWI0.SSTATUS & TWI_DIR_bm)
        {
            _delay_us(5);                                       // TWI0.SDATA write needs minimum 5 us delay at 10 MHz clock
            timeout_cnt = 0;                                    // reset timeout counter
            while (!(TWI0.SSTATUS & TWI_CLKHOLD_bm))
            {   // wait until Clock Hold flag set
                if (timeout_cnt > 2) return;                    // timeout error, return
            }
            NOP();
            if (sensor_packet.buffer[0] == REG_SENSOR_STATUS)
                TWI0.SDATA = object_detected_status;
            else if(sensor_packet.buffer[0] == REG_BLINK_STATUS)
                TWI0.SDATA = blink_status;
            else
                TWI0.SDATA = 0xFF; //read register does not exist (error)
            
            TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;
            clear_sensor_packet();
        }
        return;
    }
    if (TWI0.SSTATUS & TWI_DIF_bm)                              // Data Interrupt Flag
    {
        if (TWI0.SSTATUS & TWI_DIR_bm)
        {   // Master read operation (unused for this implementation, no reads longer than 1 byte)
            if (!(TWI0.SSTATUS & TWI_RXACK_bm))                 // Received ACK
            {
                _delay_us(5);                                   // TWI0.SDATA write needs minimum 5 us delay at 10 MHz clock
                timeout_cnt = 0;                                // reset timeout counter
                while (!(TWI0.SSTATUS & TWI_CLKHOLD_bm))
                {   // wait until Clock Hold flag set
                    if (timeout_cnt > 2) return;                // timeout error, return
                }
                
                TWI0.SDATA = 0xFF;
                //TWI0.SDATA = i2c_data;                          // Master read operation
                //num_bytes++;                                    // increment number of bytes
                TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;
                
            }
            else
            {   // Received NACK from master
                TWI0.SSTATUS |= TWI_DIF_bm | TWI_APIF_bm;     // Reset module
                TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
                clear_sensor_packet();
            }
        }
        else
        {   //Master write operation
            i2c_data = TWI0.SDATA;
            num_bytes++;

            if (num_bytes > MAX_TRANSACTION)
                TWI0.SCTRLB = TWI_ACKACT_NACK_gc | TWI_SCMD_RESPONSE_gc; // send NACK
            else
            {
                TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc; // send ACK

            }
            sensor_packet.buffer[sensor_packet.buffer_count] = i2c_data;      //Put data byte in buffer
            sensor_packet.buffer_count++;
        }
        return;
    }
    // APIF && !AP - Stop has been received
    if ((TWI0.SSTATUS & TWI_APIF_bm) && (!(TWI0.SSTATUS & TWI_AP_bm)))
    {
        TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;

        //Trigger PC ISR if last packet received
        if (sensor_packet.buffer[sensor_packet.buffer_count - 1]  == 0xFF)
        {
            sensor_packet.complete = 1;
            start_parse();
        }
      return;
    }
}
