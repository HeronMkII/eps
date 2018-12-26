/*
IMU (Inertial Measurement Unit) library
BNO080

Based on SparkFun BNO080 Arduino library:
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/blob/master/src/SparkFun_BNO080_Arduino_Library.cpp
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/blob/master/src/SparkFun_BNO080_Arduino_Library.h

The IMU contains sensors including an accelerometer and gyroscope. We use it to
collect ADCS data about the positioning and orientation of the satellite.

The IMU is located on the EPS PCB and collected to the EPS microcontroller
because it should be located in the centre of the satellite along the long axis,
which is not possible on OBC.

Most of the information is in the main datasheet, but there are supplementary
documents with necessary informataion (indicated in the references of the main
datasheet).

#0 (BNO080 Datasheet): https://cdn.sparkfun.com/assets/1/3/4/5/9/BNO080_Datasheet_v1.3.pdf
    This is the main datasheet for the specific part itself
#1 (SH-2 Reference Manual): https://cdn.sparkfun.com/assets/4/d/9/3/8/SH-2-Reference-Manual-v1.2.pdf
    This is a supplementary document for the part itself, describing all the sensors and their message/report formats
#2 (Sensor Hub Transport Protocol): https://cdn.sparkfun.com/assets/7/6/9/3/c/Sensor-Hub-Transport-Protocol-v1.7.pdf
    This document described the more general SHTP protocol, not specific to this part

Reference Implementation: https://github.com/hcrest/bno080-driver

SPI Interface - #0 p. 17-19
CPOL = 1, CPHA = 1 (#0 p. 19)

sensor triggering (#1 p. 34) - most sensors are continuous
Report IDs (#1 p. 36) - byte 0
Sensor reports (#1 p. 53)
Get feature request (DATA!!) - (#1 p. 55)
Request raw acceleration - send Get Feature Request, will respond in the future with a Get Feature Response
Get Feature Reponse messages can also be triggered regularly using the period set for the sensor
Specific sensor data starts on (#1 p. 57)

SHTP header (#2 p. 4)
Each channel has its own sequence number (#2 p. 4)
At startup, hub sends it advertisement message? (#2 p. 5)

Must delay a minimum of 100us between bytes written (#2 p. 12)
Channel usage (#2 p. 15)

Configure sensor to "normal" instead of "wakeup" (#1 p. 33)


Currently used sensors (TODO):
- accelerometer

Example of getting accelerometer data (#0 p. 43-44)

The PS1 port is permanently tied to VCC (1). The PS0/WAKE port is tied to a GPIO pin.

NOTE: Protocol sends LSB first, MSB after

TODO - H_INTN, PS0/WAKE (#0 p. 18)
TODO - check wake from sleep (#0 p. 19)
TODO - sensor calibration
TODO - sensor metadata - FRS read operation (#1 p. 29)

TODO - when does the first interrupt occur?
*/

#include "imu.h"

// CLKSEL0
pin_info_t imu_clksel0 = {
    .ddr = &DDRD,
    .port = &PORTD,
    .pin = PD7
};

// BOOTn
pin_info_t imu_boot = {
    .ddr = &DDRC,
    .port = &PORTC,
    .pin = PC4
};

// PS0/WAKE
pin_info_t imu_ps0_wake = {
    .ddr = &DDRB,
    .port = &PORTB,
    .pin = PB6
};

// RSTn
pin_info_t imu_rst = {
    .ddr = &DDRB,
    .port = &PORTB,
    .pin = PB3
};

// CS
pin_info_t imu_cs = {
    .ddr = &DDRD,
    .port = &PORTD,
    .pin = PD0
};

// INTn
// PB5 -> INT2
pin_info_t imu_int = {
    .ddr = &DDRB,
    .port = &PORTB,
    .pin = PB5
};




// Count the number of messages on each SHTP channel (in SHTP header) (#2 p. 4)
uint8_t imu_seq_nums[6] = { 0 };

/*
Initializes the IMU (#0 p. 43).
*/
void init_imu(void) {
    init_output_pin(imu_rst.pin, imu_rst.ddr, 1);
    init_input_pin(imu_int.pin, imu_int.ddr);
    // The protocol selection and boot pins are sampled at reset, so we need to
    // set PS0 = 1 before reset
    init_output_pin(imu_ps0_wake.pin, imu_ps0_wake.ddr, 1);
    init_cs(imu_ps0_wake.pin, imu_ps0_wake.ddr);




    // Enabling interrupts...

    // set behaviour of INT2 to trigger on falling edge (p.84)
    EICRA |= _BV(ISC21);
    EICRA &= ~_BV(ISC20);
    // enable external interrupt 2
    EIMSK |= _BV(INT2);
    // enable global interrupts
    sei();




    reset_imu();

    // INT is asserted after reset (#0 p. 43)
    wait_for_imu_int();

    // "A read from the BNO080 will return the initial SHTP advertisement packet"

    // TODO - on channel 0
    // TODO - in SPI library, log all sent and received bytes

    // TODO - add packet header stuff (e.g. length)

    start_imu_spi();
    // 2 bytes cargo, channel 0
    send_imu_shtp_header(2, 0);
    // "Command 0 is the advertise command." (#2 p. 15)
    send_spi(0x00);
    // Parameter 0 - 0 for just SHTP (#2 p. 15)
    send_spi(0x00);
    end_imu_spi();

    // "Response 0 is the advertise response." (#2 p. 16)
    // TODO - "On system startup, the SHTP control application will send its
    // full advertisement response, unsolicited, to the host." ???
    // TODO - Do we not need to send the command before?

    wait_for_imu_int();

    uint16_t length = 0;
    uint8_t channel = 0;

    start_imu_spi();
    receive_imu_shtp_header(&length, &channel);
    print("length = %u, channel = %u\n", length, channel);
    print("Received SPI:");
    for (uint16_t i = 0; i < length; i++) {
        uint8_t data = send_spi(0x00);
        print(" %02X", data);
    }
    print("\n");
    end_imu_spi();
}

void reset_imu(void) {
    // Assert then deassert active low reset
    // TODO - what delay time?
    set_pin_low(imu_rst.pin, imu_rst.port);
    _delay_us(1);
    set_pin_low(imu_rst.pin, imu_rst.port);
}

/*
Waits until the interrupt pin goes low.
TODO - return timeout and success value
TODO - how long does this take?
*/
void wait_for_imu_int(void) {
    uint16_t timeout;
    for (timeout = UINT16_MAX; get_pin_val(imu_int.pin, imu_int.port) != 0 && timeout > 0; timeout--) {}
}

void start_imu_spi(void) {
    set_spi_cpol_cpha(1, 1);
    set_cs_low(imu_cs.pin, imu_cs.port);
}

void end_imu_spi(void) {
    set_cs_high(imu_cs.pin, imu_cs.port);
    reset_spi_cpol_cpha();
}

/*
Sends the 4-byte SHTP header and increments the sequence number (#2 p. 4).
length - should NOT include the header length (this function will add the header length)
*/
void send_imu_shtp_header(uint16_t length, uint8_t channel) {
    // Add this header length (should be length of cargo + header)
    length += 4;

    send_spi(length & 0xFF);
    send_spi((length >> 8) & 0xFF);
    send_spi(channel);
    send_spi(imu_seq_nums[channel]);

    // Increment the sequence number for that channel
    imu_seq_nums[channel]++;
}

/*
Receives the 4-byte SHTP header and increments the sequence number (#2 p. 4).
length - will NOT include the header length (this function will subtract the header length)
*/
void receive_imu_shtp_header(uint16_t *length, uint8_t *channel) {
    // Add this header length (should be length of cargo + header)
    uint8_t length_lsb = send_spi(0x00);
    uint8_t length_msb = send_spi(0x00);
    *channel = send_spi(0x00);
    // TODO - check if seq_num is correct
    send_spi(0x00);

    *length = (((uint16_t) length_msb) << 8) | ((uint16_t) length_lsb);
    *length -= 4;

    // Increment the sequence number for that channel
    imu_seq_nums[*channel]++;
}

// INT2 interrupt from INTn pin
ISR(INT2_vect) {
    print("IMU INT: pin = %u\n", get_pin_val(imu_int.pin, imu_int.port));
}
