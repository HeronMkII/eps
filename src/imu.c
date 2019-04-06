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

One transmission to or from the IMU is referred to as a "packet", which is split up into the "header" and the "data".

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
uint8_t imu_data_len = 0;

/*
Initializes the IMU (#0 p. 43).
*/
void init_imu(void) {
    // The protocol selection and boot pins are sampled during startup, so we
    // need to set them before reset

    // CLKSEL0 = 0 (for crystal, #0 p.11)
    init_output_pin(imu_clksel0.pin, imu_clksel0.ddr, 0);
    // BOOTn = 1 (not bootloader mode, #0 p.9)
    init_output_pin(imu_boot.pin, imu_boot.ddr, 1);
    // PS0 = 1 (#0 p.9) - PS1 = 1 already in hardware
    init_output_pin(imu_ps0_wake.pin, imu_ps0_wake.ddr, 1);
    // RSTn = 1 (#0 p.10)
    init_output_pin(imu_rst.pin, imu_rst.ddr, 1);
    // CSn = 1
    init_cs(imu_cs.pin, imu_cs.ddr);
    // Interrupt input
    init_input_pin(imu_int.pin, imu_int.ddr);

    // Enable interrupts
    // set behaviour of INT2 to trigger on falling edge (p.84)
    EICRA |= _BV(ISC21);
    EICRA &= ~_BV(ISC20);
    // enable external interrupt 2
    EIMSK |= _BV(INT2);
    // enable global interrupts
    sei();

    // Reset with the appropriate GPIO pin settings
    print("Resetting IMU...\n");
    reset_imu();
    print("Done reset\n");
    // TODO
    _delay_ms(200);

    // Wait for INT to be asserted after reset (#0 p. 43)
    wait_for_imu_int();
    // _delay_ms(200);

    // "A read from the BNO080 will return the initial SHTP advertisement
    // packet" (#0 p.43)
    // "On system startup, the SHTP control application will send its
    // full advertisement response, unsolicited, to the host." (#2 p.16)
    receive_and_discard_imu_packet();
    // _delay_ms(200);

    // Initialize response
    // "An unsolicited response is also generated after startup." (#1 p.48)
    receive_and_discard_imu_packet();
    // _delay_ms(200);

    // receive_and_discard_imu_packet();
    // // _delay_ms(200);
    // receive_and_discard_imu_packet();
    // receive_and_discard_imu_packet();
    // receive_and_discard_imu_packet();


    // Request product ID (#0 p.23)
    imu_data[0] = IMU_PRODUCT_ID_REQ;
    imu_data[1] = 0x00; // reserved
    imu_data_len = 2;
    send_imu_packet(IMU_CONTROL);
    // Get response
    receive_and_discard_imu_packet();

    // TODO - in SPI library, log all sent and received bytes
}

void reset_imu(void) {
    // Assert then deassert active low reset
    // TODO - what delay time?
    set_pin_low(imu_rst.pin, imu_rst.port);
    _delay_ms(10);
    set_pin_high(imu_rst.pin, imu_rst.port);
}

void populate_imu_header(uint8_t channel, uint8_t seq_num, uint16_t length) {
    imu_header[0] = length & 0xFF;
    imu_header[1] = (length >> 8) & 0xFF;
    imu_header[2] = channel;
    imu_header[3] = seq_num;
}

void process_imu_header(uint8_t* channel, uint8_t* seq_num, uint16_t* length) {
    // Concatenate length
    *length = (((uint16_t) imu_header[1]) << 8) | ((uint16_t) imu_header[0]);
    *channel = imu_header[2];
    *seq_num = imu_header[3];
}

/*
Populate `imu_data` before calling this; this function will take care of populating and sending the header.
channel - 0 to 5
Returns - 1 for success, 0 for failure
*/
uint8_t send_imu_packet(uint8_t channel) {
    if (channel >= IMU_CHANNEL_COUNT) {
        return 0;
    }

    populate_imu_header(channel, imu_seq_nums[channel], IMU_HEADER_LEN + imu_data_len);

    print("Sending IMU SPI:\n");
    print("Header: ");
    print_bytes(imu_header, IMU_HEADER_LEN);
    print("Data: ");
    print_bytes(imu_data, imu_data_len);

    // Set feature command (#1 p.55-56)
    start_imu_spi();
    for (uint8_t i = 0; i < IMU_HEADER_LEN; i++) {
        send_spi(imu_header[i]);
    }
    for (uint8_t i = 0; i < imu_data_len; i++) {
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
    if (!wait_for_imu_int()) {
        return 0;
    }

    start_imu_spi();
    
    // Get header
    // Add this header length (should be length of cargo + header)
    // Note LSB first
    for (uint8_t i = 0; i < IMU_HEADER_LEN; i++) {
        imu_header[i] = send_spi(0x00);
    }
    print("Received IMU SPI:\n");
    print("Header: ");
    print_bytes(imu_header, IMU_HEADER_LEN);
    uint8_t channel = 0;
    uint8_t seq_num = 0;
    uint16_t length = 0;
    process_imu_header(&channel, &seq_num, &length);
    // TODO - check if seq_num is correct

    // MSB (bit 15) is used to indicate if the transfer is a continuation of the
    // previous transfer (not applicable for us)
    length &= ~_BV(15);

    // Just in case of a malfunction, make length at least 4
    if (length < IMU_HEADER_LEN) {
        length = IMU_HEADER_LEN;
    }

    // Increment the sequence number for that channel
    if (channel < IMU_CHANNEL_COUNT) {
        imu_seq_nums[channel]++;
    }

    // Get data
    // Subtract 4 bytes of header
    uint16_t data_len = length - IMU_HEADER_LEN;
    imu_data_len = 0;
    for (uint8_t i = 0; i < data_len; i++) {
        uint8_t byte = send_spi(0x00);
        if (i < IMU_DATA_MAX_LEN) {
            imu_data[i] = byte;
            imu_data_len++;
        }
    }

    end_imu_spi();

    print("Data: ");
    print_bytes(imu_data, imu_data_len);

    if (data_len > IMU_DATA_MAX_LEN) {
        print("Received packet too long\n");
        return 0;
    }

    return 1;
}

/*
Set feature command (#1 p.55-56)
*/
uint8_t set_imu_feature(uint8_t feat_report_id) {
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
    receive_and_discard_imu_packet();

    return 1;
}

uint8_t get_imu_accel(uint16_t* x, uint16_t* y, uint16_t* z) {
    // TODO - Q point?
    // Q point - number of fractional digits after (to the right of) the decimal point, i.e. higher Q point means smaller/more precise number (#1 p.22)
    // https://en.wikipedia.org/wiki/Q_(number_format)
    // TODO - m/s^2? change precision?

    print("getting accel\n");

    if (!set_imu_feature(IMU_ACCEL)) {
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
        if (imu_data_len != 10) {
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
    print("getting raw gyro\n");

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
    return 1;
}

/*
Calibrated gyroscope (rad/s, Q point = 9) (#1 p.60)
Drift-compensated rotational velocity
*/
uint8_t get_imu_cal_gyro(void) {
    return 1;
}


/*
Waits until the interrupt pin goes low.
TODO - return timeout and success value
TODO - how long does this take?
Returns - 1 for success (got INT), 0 for failure (no INT)
*/
uint8_t wait_for_imu_int(void) {
    uint16_t timeout = UINT16_MAX;
    while (get_pin_val(imu_int.pin, imu_int.port) != 0 && timeout > 0) {
        timeout--;
    }

    if (timeout == 0) {
        print("Failed INT\n");
        return 0;
    }

    print("Successful INT\n");
    return 1;
}

void start_imu_spi(void) {
    set_spi_cpol_cpha(1, 1);
    set_cs_low(imu_cs.pin, imu_cs.port);
}

void end_imu_spi(void) {
    set_cs_high(imu_cs.pin, imu_cs.port);
    reset_spi_cpol_cpha();
}

uint8_t receive_and_discard_imu_packet(void) {
    if (!receive_imu_packet()) {
        return 0;
    }

    print("Received packet\n");
    for (uint16_t i = 0; i < imu_data_len; i++) {
        put_uart_char(imu_data[i]);
    }
    put_uart_char('\n');
    print_bytes(imu_data, imu_data_len);
    
    return 1;
}

// INT2 interrupt from INTn pin
ISR(INT2_vect) {
    print("\nIMU INT: pin = %u\n", get_pin_val(imu_int.pin, imu_int.port));
}

// Get feature request
void get_feat_req(void) {
    // Looking for "input report" for actual read sensor data
    // "Get feature response" just tells the configuration of the sensor, not the actual read data
    // "Sensor feature reports are used to control and configure sensors, and to retrieve sensor configuration. Sensor input reports are used to send sensor data to the host." (#1 p.53)
    // Input reports, output reports, feature reports (#1 p.32-33)
    // "Sensor operating rate is controlled through the report interval field. When set to zero the sensor is off. When set to a non-zero value the sensor generates reports at that interval or more frequently." (#1 p.33)
    // "Input reports may also be requested by the host at any time." (#1 p.32-33)
    // Didn't find any information about how to request an input report at any time - probably just need to do set feature request with a continuously repeating interval, wait for a single input report, then set feature request again with a period of 0 to disable the sensor
    // Set feature command -> get feature response (should be R instead of W)

}