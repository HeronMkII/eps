/*
IMU (Inertial Measurement Unit) library
BNO080

Datasheets:
#0 (BNO080 Datasheet): https://cdn.sparkfun.com/assets/1/3/4/5/9/BNO080_Datasheet_v1.3.pdf
    This is the main datasheet for the specific part itself
#1 (SH-2 Reference Manual): https://cdn.sparkfun.com/assets/4/d/9/3/8/SH-2-Reference-Manual-v1.2.pdf
    This is a supplementary document for the part itself, describing all the sensors and their message/report formats
#2 (Sensor Hub Transport Protocol): https://cdn.sparkfun.com/assets/7/6/9/3/c/Sensor-Hub-Transport-Protocol-v1.7.pdf
    This document described the more general SHTP protocol, not specific to this part

Based on SparkFun BNO080 Arduino library:
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/blob/master/src/SparkFun_BNO080_Arduino_Library.cpp
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/blob/master/src/SparkFun_BNO080_Arduino_Library.h

Other Library/Driver Implementations:
https://github.com/hcrest/bno080-driver
https://os.mbed.com/users/MultipleMonomials/code/BNO080/
https://github.com/BBUK/Bell-Boy
https://github.com/jps2000/BNO080
http://www.itpcs.cn/github_/fm4dd/pi-bno080
https://github.com/williamg42/BNO080-Linux-Library
https://github.com/fm4dd/pi-bno080
https://github.com/josepmcgrath/imu_i2c

Miscellanous Links:
Breakout board hookup guide: https://learn.sparkfun.com/tutorials/qwiic-vr-imu-bno080-hookup-guide/all
https://www.raspberrypi.org/forums/viewtopic.php?t=203550
https://www.arduinolibraries.info/libraries/spark-fun-bno080-cortex-based-imu
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library/issues/5
https://github.com/hcrest/bno080-nucleo-demo/issues/6
https://www.raspberrypi.org/forums/viewtopic.php?t=196310
https://github.com/fm4dd/pi-bno080/blob/master/issues.md


The IMU contains sensors including an accelerometer and gyroscope. We use it to
collect ADCS data about the positioning and orientation of the satellite.

The IMU is located on the EPS PCB and collected to the EPS microcontroller
because it should be located in the centre of the satellite along the long axis,
which is not possible on OBC.

Most of the information is in the main datasheet, but there are supplementary
documents with necessary informataion (indicated in the references of the main
datasheet).

One transmission to or from the IMU is referred to as a "packet", which is split up into the "header" and the "data".

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

Cannot use one-shot trigger mode reporting, only continuous (#1 p.34-35)

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

#define PRINT_FUNC print("%s\n", __FUNCTION__);

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

// CSn
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

uint8_t imu_header[IMU_HEADER_LEN] = { 0x00 };
uint8_t imu_data[IMU_DATA_MAX_LEN] = { 0x00 };
// Number of valid bytes in `imu_data`, NOT including the header
uint16_t imu_data_len = 0;

// TODO - modify lib-common print_bytes, fix uint16_t bug for len/i
void print_hex(uint8_t* data, uint16_t len) {
    if (len == 0) {
        return;
    }
    print("%.2x", data[0]);
    for (uint16_t i = 1; i < len; i++) {
        print(":%.2x", data[i]);
    }
    print("\n");
}

/*
Initializes the IMU (#0 p. 43).
*/
void init_imu(void) {
    PRINT_FUNC

    // TODO
    set_spi_cpol_cpha(1, 1);
    // BNO080 supports up to 3MHz, use 2MHz
    set_spi_clk_freq(SPI_FOSC_4);

    // The protocol selection and boot pins are sampled during startup, so we
    // need to set them before reset
    init_imu_pins();

    // Reset with the appropriate GPIO pin settings
    reset_imu();
    print("Done reset\n");

    // TODO - soft reset??

    // while (1) {
    //     // print("PINB = %.2x\n", PINB);
    //     while (get_imu_int()) {}
    //     print("PINB = %.2x\n", PINB);
    //     start_imu_spi();
    //     for (uint16_t i = 0; i < 276; i++) {
    //         uint8_t x = send_spi(0x00);
    //         print("%.2x:", x);
    //         // put_uart_char(x);
    //     }
    //     // print("\n\n");
    //     put_uart_char('\n');
    //     put_uart_char('\n');
        
    //     end_imu_spi();
    //     print("done SPI\n");
    // }

    // Activate WAKE pin
    // wake_imu();
    // print("Done wake\n");

    // Wait for INT to be asserted after reset (#0 p. 43)
    // wait_for_imu_int();

    // "A read from the BNO080 will return the initial SHTP advertisement
    // packet [...] Following the SHTP advertisement packet, the individual applications built in to the BNO080 will send a packet indicating they have left the reset state" (#0 p.43)
    // "On system startup, the SHTP control application will send its
    // full advertisement response, unsolicited, to the host." (#2 p.16)
    receive_and_discard_imu_packet();

    // "The executable will issue a reset message on SHTP channel 1" (#0 p.43)
    receive_and_discard_imu_packet();

    // Initialize response
    // "SH-2 will issue an unsolicited initialization message on SHTP channel 2" (#0 p.43)
    // "An unsolicited response is also generated after startup." (#1 p.48)
    receive_and_discard_imu_packet();
}

void init_imu_pins(void) {
    // CLKSEL0 = 0 (for crystal, #0 p.11)
    init_output_pin(imu_clksel0.pin, imu_clksel0.ddr, 0);
    // BOOTn = 1 (not bootloader mode, #0 p.9)
    init_output_pin(imu_boot.pin, imu_boot.ddr, 1);
    // CSn = 1
    init_cs(imu_cs.pin, imu_cs.ddr);
    set_cs_high(imu_cs.pin, imu_cs.port);
    // PS0 = 1 (#0 p.9) - not using WAKE yet
    // in hardware, PS1 = 1
    init_output_pin(imu_ps0_wake.pin, imu_ps0_wake.ddr, 1);
    // Interrupt input
    init_input_pin(imu_int.pin, imu_int.ddr);
    // TODO - does it need input pullup?
    // configure input pullup resistor (14.2.1, p.95)
    PORTB |= _BV(5);
    print("PORTB = %.2x\n", PORTB);
    // RSTn = 1 (#0 p.10)
    init_output_pin(imu_rst.pin, imu_rst.ddr, 1);
    
    // Enable interrupts
    // set behaviour of INT2 to trigger on any logical change (falling or rising edge) (p.84)
    EICRA &= ~_BV(ISC21);
    EICRA |= _BV(ISC20);
    // enable external interrupt 2
    EIMSK |= _BV(INT2);
    // enable global interrupts
    sei();
}

void wake_imu(void) {
    // Set wake low then high (#0 p.19)
    set_pin_low(imu_ps0_wake.pin, imu_ps0_wake.port);
    wait_for_imu_int();
    set_pin_high(imu_ps0_wake.pin, imu_ps0_wake.port);
}

void reset_imu(void) {
    PRINT_FUNC
    // Assert then deassert active low reset
    // TODO - what delay time?
    set_pin_low(imu_rst.pin, imu_rst.port);
    _delay_ms(2);
    set_pin_high(imu_rst.pin, imu_rst.port);
}

void inf_loop_imu_receive(void) {
    while (1) {
        receive_and_discard_imu_packet();
    }
}

uint8_t get_imu_prod_id(void) {
    for (uint8_t i = 0; i < IMU_PACKET_CHECK_COUNT; i++) {
        // Request product ID (#0 p.23)
        imu_data[0] = IMU_PRODUCT_ID_REQ;
        imu_data[1] = 0x00; // reserved
        imu_data_len = 2;
        send_imu_packet(IMU_CONTROL);

        // Get response
        if (!receive_imu_packet()) {
            continue;
        }
        // TODO - is this correct?
        if (imu_data_len >= 16 && imu_data[0] == IMU_PRODUCT_ID_RESP) {
            return 1;
        }
    }

    return 0;
}



void populate_imu_header(uint8_t channel, uint8_t seq_num, uint16_t length) {
    PRINT_FUNC
    imu_header[0] = length & 0xFF;
    imu_header[1] = (length >> 8) & 0xFF;
    imu_header[2] = channel;
    imu_header[3] = seq_num;
}

void process_imu_header(uint8_t* channel, uint8_t* seq_num, uint16_t* length) {
    PRINT_FUNC
    // Concatenate length
    *length = (((uint16_t) imu_header[1]) << 8) | ((uint16_t) imu_header[0]);
    *channel = imu_header[2];
    *seq_num = imu_header[3];
    print("length = %u, channel = %u, seq_num = %u\n", *length, *channel, *seq_num);
}

/*
Populate `imu_data` before calling this; this function will take care of populating and sending the header.
channel - 0 to 5
Returns - 1 for success, 0 for failure
*/
uint8_t send_imu_packet(uint8_t channel) {
    PRINT_FUNC
    if (channel >= IMU_CHANNEL_COUNT) {
        return 0;
    }
    // TODO ?
    if (!wait_for_imu_int()) {
        return 0;
    }

    populate_imu_header(channel, imu_seq_nums[channel], IMU_HEADER_LEN + imu_data_len);

    print("Sending IMU SPI:\n");
    print("Header: ");
    print_hex(imu_header, IMU_HEADER_LEN);
    print("Data: ");
    print_hex(imu_data, imu_data_len);

    // Set feature command (#1 p.55-56)
    start_imu_spi();
    for (uint16_t i = 0; i < IMU_HEADER_LEN; i++) {
        send_spi(imu_header[i]);
    }
    for (uint16_t i = 0; i < imu_data_len; i++) {
        send_spi(imu_data[i]);
    }
    end_imu_spi();

    // Increment the sequence number for that channel
    imu_seq_nums[channel]++;

    return 1;
}

/*
This function will populate `imu_header` and `imu_data`
Returns - 1 for success, 0 for failure (either no interrupt or too long message for buffer)
*/
uint8_t receive_imu_packet(void) {
    PRINT_FUNC
    if (!wait_for_imu_int()) {
        return 0;
    }

    print("Received IMU SPI:\n");

    start_imu_spi();

    // Get header
    // Add this header length (should be length of cargo + header)
    // Note LSB first
    for (uint16_t i = 0; i < IMU_HEADER_LEN; i++) {
        imu_header[i] = send_spi(0x00);
    }
    
    uint8_t channel = 0;
    uint8_t seq_num = 0;
    uint16_t length = 0;
    process_imu_header(&channel, &seq_num, &length);
    // TODO - check if seq_num is correct

    // "A length of 65535 is an error. The remaining header and cargo bytes are ignored. This type of error may occur if there is a failure in the SPI or I2C peripheral." (#2 p.4-5)
    if (length == 0xFFFF) {
        print("Error: length is 0xFFFF\n");
        end_imu_spi();
        return 0;
    }

    // MSB (bit 15) is used to indicate if the transfer is a continuation of the
    // previous transfer (not applicable for us)
    length &= ~_BV(15);

    // Check for a null header (#2 p.5)
    if (length < IMU_HEADER_LEN) {
        print("Error: length too short\n");
        end_imu_spi();
        return 0;
    }
    
    // According to the reference library, we don't increment our sequence number when receiving packets

    // Subtract 4 bytes get length of data (without header)
    uint16_t data_len = length - IMU_HEADER_LEN;
    print("data_len = %u\n", data_len);

    // Read and store data
    imu_data_len = 0;
    for (uint16_t i = 0; i < data_len; i++) {
        // Sending 0xFF, not sure why but the reference library does this in receivePacket()
        uint8_t byte = send_spi(0xFF);
        // Only store received data within the size of our buffer
        if (i < IMU_DATA_MAX_LEN) {
            imu_data[i] = byte;
            imu_data_len++;
        }
    }

    end_imu_spi();

    print("Header: ");
    print_hex(imu_header, IMU_HEADER_LEN);
    print("Data: ");
    print_hex(imu_data, imu_data_len);

    // Print data as string
    for (uint16_t i = 0; i < imu_data_len; i++) {
        put_uart_char(imu_data[i]);
    }
    put_uart_char('\n');

    if (data_len > IMU_DATA_MAX_LEN) {
        print("Did not save entire packet\n");
    }

    return 1;
}

/*
Set feature command (#1 p.55-56)
*/
uint8_t set_imu_feature(uint8_t feat_report_id) {
    PRINT_FUNC
    imu_data[0] = IMU_SET_FEAT_CMD;
    imu_data[1] = feat_report_id;
    imu_data[2] = 0x00;
    imu_data[3] = 0x00;
    imu_data[4] = 0x00;
    // TODO - for now, 60,000us report interval (this is in microseconds)
    imu_data[5] = 0x60;
    imu_data[6] = 0xEA;
    imu_data[7] = 0x00;
    imu_data[8] = 0x00;
    imu_data[9] = 0x00;
    imu_data[10] = 0x00;
    imu_data[11] = 0x00;
    imu_data[12] = 0x00;
    imu_data[13] = 0x00;
    imu_data[14] = 0x00;
    imu_data[15] = 0x00;
    imu_data[16] = 0x00;
    imu_data_len = 17;
    if (!send_imu_packet(IMU_CONTROL)) {
        return 0;
    }

    // TODO - get feature response?
    // receive_and_discard_imu_packet();

    return 1;
}

uint8_t get_imu_accel(uint16_t* x, uint16_t* y, uint16_t* z) {
    PRINT_FUNC
    // TODO - Q point?
    // Q point - number of fractional digits after (to the right of) the decimal point, i.e. higher Q point means smaller/more precise number (#1 p.22)
    // https://en.wikipedia.org/wiki/Q_(number_format)
    // TODO - m/s^2? change precision?

    if (!set_imu_feature(IMU_ACCEL)) {
        return 0;
    }

    // Try multiple packets
    for (uint8_t i = 0; i < IMU_PACKET_CHECK_COUNT; i++) {
        if (!receive_imu_packet()) {
            continue;
        }
        // TODO - does this contain the timebase reference? does that only apply to batching? are we using batching? (#0 p.44, #1 p.79)
        if (imu_data_len < 10) {
            continue;
        }
        if (imu_data[0] != IMU_ACCEL) {
            continue;
        }

        if (x != NULL) {
            *x = (((uint16_t) imu_data[5]) << 8) | ((uint16_t) imu_data[4]);
        }
        if (y != NULL) {
            *y = (((uint16_t) imu_data[7]) << 8) | ((uint16_t) imu_data[6]);
        }
        if (z != NULL) {
            *z = (((uint16_t) imu_data[9]) << 8) | ((uint16_t) imu_data[8]);
        }

        return 1;
    }

    return 0;
}

// TODO - calibrate gyroscope CAN/trans command?
// Gyro/angular velocity measurements (#0 p.31)
// Gyroscope drift:
// https://www.analog.com/en/analog-dialogue/raqs/raq-issue-139.html
// https://motsai.com/handling-gyroscopes-bias-drift/
// https://stemrobotics.cs.pdx.edu/sites/default/files/Gyro.pdf
// https://base.xsens.com/hc/en-us/articles/209611089-Understanding-Sensor-Bias-offset-
// https://stackoverflow.com/questions/14210206/gyroscope-drift-on-mobile-phones
// https://en.wikipedia.org/wiki/Inertial_navigation_system#Error

/*
Raw uncalibrated gyroscope (ADC units) (#1 p.59)
*/
uint8_t get_imu_raw_gyro(uint16_t* x, uint16_t* y, uint16_t* z) {
    PRINT_FUNC

    if (!set_imu_feature(IMU_RAW_GYRO)) {
        return 0;
    }

    // Try 10 packets
    // TODO - what timeout/number?
    for (uint8_t i = 0; i < 10; i++) {
        _delay_ms(100);

        if (!receive_imu_packet()) {
            continue;
        }
        // TODO - does this contain the timebase reference? does that only apply to batching? are we using batching? (#0 p.44, #1 p.79)
        if (imu_data_len != 16) {
            continue;
        }
        if (imu_data[0] != IMU_RAW_GYRO) {
            continue;
        }

        if (x != NULL) {
            *x = (((uint16_t) imu_data[5]) << 8) | ((uint16_t) imu_data[4]);
        }
        if (y != NULL) {
            *y = (((uint16_t) imu_data[7]) << 8) | ((uint16_t) imu_data[6]);
        }
        if (z != NULL) {
            *z = (((uint16_t) imu_data[9]) << 8) | ((uint16_t) imu_data[8]);
        }

        return 1;
    }

    return 0;
}

/*
Uncalibrated gyroscope (rad/s, Q point = 9) (#1 p.60)
Non-drift-compensated rotational velocity
"rotational velocity without drift compensation. An estimate of drift is also reported."
*/
uint8_t get_imu_uncal_gyro(void) {
    PRINT_FUNC
    return 1;
}

/*
Calibrated gyroscope (rad/s, Q point = 9) (#1 p.60)
Drift-compensated rotational velocity
*/
uint8_t get_imu_cal_gyro(void) {
    PRINT_FUNC
    return 1;
}


/*
Waits until the interrupt pin goes low.
TODO - return timeout and success value
TODO - how long does this take?
Returns - 1 for success (got INT), 0 for failure (no INT)
*/
uint8_t wait_for_imu_int(void) {
    PRINT_FUNC
    // while (get_imu_int() == 0) {}
    // Wait up to 255ms (can take up to 104ms after hardware reset, see reference library)
    // TODO - fix get_pin_val() in lib-common
    uint8_t timeout = UINT8_MAX;
    while (get_imu_int() != 0 && timeout > 0) {
        _delay_ms(1);
        timeout--;
    }

    if (timeout == 0) {
        print("Failed INT\n");
        return 0;
    }

    print("Successful INT: timeout = %u\n", timeout);
    return 1;
}

void start_imu_spi(void) {
    // PRINT_FUNC
    // set_spi_cpol_cpha(1, 1);
    set_cs_low(imu_cs.pin, imu_cs.port);
}

void end_imu_spi(void) {
    // PRINT_FUNC
    set_cs_high(imu_cs.pin, imu_cs.port);
    // reset_spi_cpol_cpha();
}

uint8_t receive_and_discard_imu_packet(void) {
    PRINT_FUNC
    if (!receive_imu_packet()) {
        print("Receive packet: FAIL\n");
        return 0;
    }

    print("Receive packet: SUCCESS\n");
    return 1;
}

uint8_t get_imu_int(void) {
    return (PINB & _BV(5)) ? 1 : 0;
}

// INT2 interrupt from INTn pin
ISR(INT2_vect) {
    // print("\nINT2: pin = %u (%.2x)\n", get_imu_int(), PINB);
}

// Get feature request
void get_feat_req(void) {
    PRINT_FUNC
    // Looking for "input report" for actual read sensor data
    // "Get feature response" just tells the configuration of the sensor, not the actual read data
    // "Sensor feature reports are used to control and configure sensors, and to retrieve sensor configuration. Sensor input reports are used to send sensor data to the host." (#1 p.53)
    // Input reports, output reports, feature reports (#1 p.32-33)
    // "Sensor operating rate is controlled through the report interval field. When set to zero the sensor is off. When set to a non-zero value the sensor generates reports at that interval or more frequently." (#1 p.33)
    // "Input reports may also be requested by the host at any time." (#1 p.32-33)
    // Didn't find any information about how to request an input report at any time - probably just need to do set feature request with a continuously repeating interval, wait for a single input report, then set feature request again with a period of 0 to disable the sensor
    // Set feature command -> get feature response (should be R instead of W)

}

// TODO
// "Note that the BNO080 also provides a timebase reference report with sensor reports:" (#0 p.28)

// TODO !! - check for deassert and reassert INTn
// "HINT may be deasserted at any time after the read begins, including after the transaction is complete." (#2 p.6)
// e.g. wait_for_imu_int_low() and wait_for_imu_int_high(), or wait_for_imu_int(0/1)
// e.g. do transaction, wait for INT high
// e.g. before read, wait for INT high then low
