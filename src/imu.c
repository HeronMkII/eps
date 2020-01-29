/*
IMU (Inertial Measurement Unit) library
BNO080

The IMU contains sensors including an accelerometer, gyroscope, and magnetometer.
We use it to collect ADCS data about the positioning and orientation of the
satellite.

Datasheets:
#0 (BNO080 Datasheet): https://cdn.sparkfun.com/assets/1/3/4/5/9/BNO080_Datasheet_v1.3.pdf
    This is the main datasheet for the BNO080 component.
#1 (SH-2 Reference Manual): https://cdn.sparkfun.com/assets/4/d/9/3/8/SH-2-Reference-Manual-v1.2.pdf
    This is a supplementary document for the BNO080 component, describing all the
    sensors and their message/report formats in detail.
#2 (Sensor Hub Transport Protocol): https://cdn.sparkfun.com/assets/7/6/9/3/c/Sensor-Hub-Transport-Protocol-v1.7.pdf
    This document describes the more general SHTP protocol, not specific to the BNO080 component.

Based on SparkFun BNO080 Arduino library:
https://github.com/sparkfun/SparkFun_BNO080_Arduino_Library

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

Most of the information is in the main datasheet, but there are supplementary
documents with necessary informataion (indicated in the references of the main
datasheet).

The IMU is located on the EPS PCB and collected to the EPS microcontroller
because it should be located in the centre of the satellite along the long axis,
which is not possible on OBC.

SHTP Protocol:
- One transmission to or from the IMU is referred to as a "packet", which is split
up into the "header" and the "cargo".
- Host (MCU) to hub (BNO080) - write
- Hub (BNO080) to host (MCU) - read
- SHTP header (#2 p. 4)
- Each channel has its own sequence number (#2 p. 4)
- NOTE: Protocol always sends LSB first, MSB last
- "HINT may be deasserted at any time after the read begins, including after the transaction is complete." (#2 p.6)

Communication:
- SPI Interface - CPOL = 1, CPHA = 1 (#0 p.17-19)
- Channel usage (#2 p. 15)
- Report IDs (#1 p. 36) - byte 0
- Sensor triggering (#1 p. 34) - most sensors are continuous
- Sensor reports (#1 p. 53)
- Cannot use one-shot trigger mode reporting, only continuous (#1 p.34-35) - the sensor keeps sending data at regular intervals while enabled
- Configure sensor to "normal" instead of "wakeup" (#1 p. 33)
- Example of getting accelerometer data (#0 p. 43-44)
- Specific sensor data starts on (#1 p. 57)
- Get feature request (#1 p. 55)
- "Get feature response" just tells the configuration of the sensor, not the actual read data
- "Sensor feature reports are used to control and configure sensors, and to retrieve sensor configuration. Sensor input reports are used to send sensor data to the host." (#1 p.53)
- "Sensor operating rate is controlled through the report interval field. When set to zero the sensor is off. When set to a non-zero value the sensor generates reports at that interval or more frequently." (#1 p.33)
- "Input reports may also be requested by the host at any time." (#1 p.32-33) -  not sure what this is supposed to mean, just ignored it
- Set feature command -> get feature response (should be R instead of W) (#1 p.36)
- "Note that the BNO080 also provides a timebase reference report with sensor reports:" (#0 p.28)
- For input report packets, the received packet begins with the 5-byte timebase reference section (0xFB) (#0 p.44, #1 p.79)

Request data:
- Send Set Feature Request
- IMU responds with Get Feature Response
- IMU sends Input Report after specified interval (with actual sensor data)
- Send Set Feature Request with time interval = 0 (to disable sensor, stop receiving input reports)
- IMU responds with Get Feature Response

Hardware Configuration:
- The PS1 port is permanently tied to VCC (1).
- The PS0/WAKE port is tied to a GPIO pin.

Gyro/angular velocity measurements (#0 p.31)
Gyroscope drift:
https://www.analog.com/en/analog-dialogue/raqs/raq-issue-139.html
https://motsai.com/handling-gyroscopes-bias-drift/
https://stemrobotics.cs.pdx.edu/sites/default/files/Gyro.pdf
https://base.xsens.com/hc/en-us/articles/209611089-Understanding-Sensor-Bias-offset-
https://stackoverflow.com/questions/14210206/gyroscope-drift-on-mobile-phones
https://en.wikipedia.org/wiki/Inertial_navigation_system#Error
*/

#include "imu.h"

// Useful for debugging
// #define PRINT_FUNC print("%s\n", __FUNCTION__);

// Comment out this line to disable debugging print statements (sent/received packets)
// #define IMU_DEBUG

// More verbose logging
// #define IMU_VERBOSE

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


/*
Initializes the IMU (#0 p. 43).
*/
void init_imu(void) {
    // The protocol selection and boot pins are sampled during startup, so we
    // need to set them before reset
    init_imu_pins();

    // Reset with the appropriate GPIO pin settings
    reset_imu();

    // "A read from the BNO080 will return the initial SHTP advertisement
    // packet [...] Following the SHTP advertisement packet, the individual applications built in to the BNO080 will send a packet indicating they have left the reset state" (#0 p.43)
    // "On system startup, the SHTP control application will send its
    // full advertisement response, unsolicited, to the host." (#2 p.16)
    // At startup, hub sends it advertisement message (#2 p.5)
    receive_imu_packet();

    // "The executable will issue a reset message on SHTP channel 1" (#0 p.43)
    receive_imu_packet();

    // Initialize response
    // "SH-2 will issue an unsolicited initialization message on SHTP channel 2" (#0 p.43)
    // "An unsolicited response is also generated after startup." (#1 p.48)
    receive_imu_packet();
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
    // configure input pullup resistor (14.2.1, p.95)
    set_pin_pullup(imu_int.pin, imu_int.port, 1);
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

#ifdef IMU_DEBUG
    print("DDRB: 0x%.2x\n", DDRB);
    print("PORTB: 0x%.2x\n", PORTB);
    print("PINB: 0x%.2x\n", PINB);
#endif
}

void reset_imu(void) {
    // Assert then deassert active low reset
    // Not in datasheet, but use 2ms to match the reference library
    set_pin_low(imu_rst.pin, imu_rst.port);
    _delay_ms(2);
    set_pin_high(imu_rst.pin, imu_rst.port);
}

void wake_imu(void) {
    // Set wake low then high (#0 p.19)
    set_pin_low(imu_ps0_wake.pin, imu_ps0_wake.port);
    wait_for_imu_int();
    set_pin_high(imu_ps0_wake.pin, imu_ps0_wake.port);
}

uint8_t get_imu_int(void) {
    return get_pin_val(imu_int.pin, imu_int.port);
}

/*
Waits until the interrupt pin goes low.
Returns - 1 for success (got INT), 0 for failure (no INT)
*/
uint8_t wait_for_imu_int(void) {
    // Wait up to 255ms (can take up to 104ms after hardware reset, see reference library)
    uint8_t timeout = UINT8_MAX;
    while (get_imu_int() != 0 && timeout > 0) {
        _delay_ms(1);
        timeout--;
    }

    if (timeout == 0) {
#ifdef IMU_DEBUG
        print("Failed INT\n");
#endif
        return 0;
    }

#ifdef IMU_DEBUG
    print("Successful INT: timeout = %u\n", timeout);
#endif

    return 1;
}

void start_imu_spi(void) {
    // Need to use SPI mode 3 (CPOL = 1, CPHA = 1)
    set_spi_cpol_cpha(1, 1);
    // BNO080 supports up to 3MHz, our clock division only allows 2MHz
    set_spi_clk_freq(SPI_FOSC_4);
    set_cs_low(imu_cs.pin, imu_cs.port);
}

void end_imu_spi(void) {
    set_cs_high(imu_cs.pin, imu_cs.port);
    reset_spi_clk_freq();
    reset_spi_cpol_cpha();
}

void process_imu_header(uint8_t* channel, uint8_t* seq_num, uint16_t* length) {
    // Concatenate length
    *length = (((uint16_t) imu_header[1]) << 8) | ((uint16_t) imu_header[0]);
    *channel = imu_header[2];
    *seq_num = imu_header[3];
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
    for (uint16_t i = 0; i < IMU_HEADER_LEN; i++) {
        imu_header[i] = send_spi(0x00);
    }
    
    uint8_t channel = 0;
    uint8_t seq_num = 0;
    uint16_t length = 0;
    process_imu_header(&channel, &seq_num, &length);

#ifdef IMU_DEBUG
    print("\nReceived IMU SPI:\n");
    print("length = %u, channel = %u, seq_num = %u\n", length, channel, seq_num);
#endif

    // "A length of 65535 is an error. The remaining header and cargo bytes are ignored. This type of error may occur if there is a failure in the SPI or I2C peripheral." (#2 p.4-5)
    if (length == 0xFFFF) {
        end_imu_spi();
#ifdef IMU_DEBUG
        print("Error: length is 0xFFFF\n");
#endif
        return 0;
    }

    // MSB (bit 15) is used to indicate if the transfer is a continuation of the
    // previous transfer (not applicable for us)
    length &= ~_BV(15);

    // Check for a null header (#2 p.5)
    if (length < IMU_HEADER_LEN) {
        end_imu_spi();
#ifdef IMU_DEBUG
        print("Error: null header\n");
#endif
        return 0;
    }
    
    // According to the reference library, we don't increment our sequence number when receiving packets

    // Subtract 4 bytes to get length of data (without header)
    uint16_t data_len = length - IMU_HEADER_LEN;
    
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

#ifdef IMU_DEBUG
    print("data_len = %u\n", data_len);
    print("Header: ");
    print_bytes(imu_header, IMU_HEADER_LEN);
    print("Data: ");
    print_bytes(imu_data, imu_data_len);

    // Print data as string
    // for (uint16_t i = 0; i < imu_data_len; i++) {
    //     put_uart_char(imu_data[i]);
    // }
    // put_uart_char('\n');

    if (data_len > IMU_DATA_MAX_LEN) {
        print("Did not save entire packet\n");
    }
#endif

    return 1;
}

void populate_imu_header(uint8_t channel, uint8_t seq_num, uint16_t length) {
    imu_header[0] = length & 0xFF;
    imu_header[1] = (length >> 8) & 0xFF;
    imu_header[2] = channel;
    imu_header[3] = seq_num;
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
    
    // Need to assert the wake signal first or else we never receive the interrupt
    wake_imu();
    if (!wait_for_imu_int()) {
        return 0;
    }

    populate_imu_header(channel, imu_seq_nums[channel], IMU_HEADER_LEN + imu_data_len);

#ifdef IMU_DEBUG
    print("\nSending IMU SPI:\n");
    print("Header: ");
    print_bytes(imu_header, IMU_HEADER_LEN);
    print("Data: ");
    print_bytes(imu_data, imu_data_len);
#endif

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
It seems that after sending the request, first we receive a 16-byte packet (assumed as the overall system ID), followed by a separate (but not continued) 48-byte packet (assumed as the subsystem IDs).
*/
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
        if (imu_data_len >= 16 && imu_data[0] == IMU_PRODUCT_ID_RESP) {
            // Receive 48-byte packet for subsystems (don't care about contents)
            receive_imu_packet();
            return 1;
        }
    }

    return 0;
}

/*
Set feature command (#1 p.55-56)
"Sensor operating rate is controlled through the report interval field. When set to zero the sensor is off." (#1 p.33)
report_interval - in microseconds
*/
uint8_t send_imu_set_feat_cmd(uint8_t feat_report_id, uint32_t report_interval) {
    imu_data[0] = IMU_SET_FEAT_CMD;
    imu_data[1] = feat_report_id;
    imu_data[2] = 0x00;
    imu_data[3] = 0x00;
    imu_data[4] = 0x00;
    imu_data[5] = report_interval & 0xFF;
    imu_data[6] = (report_interval >> 8) & 0xFF;
    imu_data[7] = (report_interval >> 16) & 0xFF;
    imu_data[8] = (report_interval >> 24) & 0xFF;
    imu_data[9] = 0x00;
    imu_data[10] = 0x00;
    imu_data[11] = 0x00;
    imu_data[12] = 0x00;
    imu_data[13] = 0x00;
    imu_data[14] = 0x00;
    imu_data[15] = 0x00;
    imu_data[16] = 0x00;
    imu_data_len = 17;

    // Send set feature command
    if (!send_imu_packet(IMU_CONTROL)) {
        return 0;
    }

    // Wait for get feature response
    for (uint8_t i = 0; i < IMU_PACKET_CHECK_COUNT; i++) {
        if (!receive_imu_packet()) {
            continue;
        }
        if (imu_data_len < 17) {
            continue;
        }
        if (imu_data[0] != IMU_GET_FEAT_RESP) {
            continue;
        }

        return 1;
    }

    return 0;
}

uint8_t enable_imu_feat(uint8_t feat_report_id) {
    return send_imu_set_feat_cmd(feat_report_id, IMU_DEF_REPORT_INTERVAL);
}

uint8_t disable_imu_feat(uint8_t feat_report_id) {
    return send_imu_set_feat_cmd(feat_report_id, 0);
}



/*
Enables a feature, gets one input report, and disables the feature.
This only works for features that provide an input report of 5 bytes (timebase reference) + 10 bytes (sensor data, last 6 bytes are x/y/z)
*/
uint8_t get_imu_data(uint8_t feat_report_id, uint16_t* x, uint16_t* y, uint16_t* z) {
    // Send set feature command, receive get feature response
    if (!enable_imu_feat(feat_report_id)) {
        return 0;
    }

    // Get input report packet
    // The received packet begins with the timebase reference (0xFB) (#0 p.44, #1 p.79)
    // Ignore base delta, sequence number, delay, etc.
    for (uint8_t i = 0; i < IMU_PACKET_CHECK_COUNT; i++) {
        if (!receive_imu_packet()) {
            continue;
        }
        if (imu_data_len < 15) {
            continue;
        }
        if (imu_data[0] != IMU_BASE_TIMESTAMP_REF) {
            continue;
        }
        if (imu_data[5] != feat_report_id) {
            continue;
        }

        if (x != NULL) {
            *x = (((uint16_t) imu_data[10]) << 8) | ((uint16_t) imu_data[9]);
        }
        if (y != NULL) {
            *y = (((uint16_t) imu_data[12]) << 8) | ((uint16_t) imu_data[11]);
        }
        if (z != NULL) {
            *z = (((uint16_t) imu_data[14]) << 8) | ((uint16_t) imu_data[13]);
        }

        // After getting data from the input report, disable the sensor so we don't keep receiving input report packets every 60ms
        // Send set feature command, receive get feature response
        if (!disable_imu_feat(feat_report_id)) {
            return 0;
        }

        return 1;
    }

    return 0;
}

/*
x, y, z are signed fixed-point
"The units are m/s^2. The Q point is 8." (#1 p.58)
*/
uint8_t get_imu_accel(uint16_t* x, uint16_t* y, uint16_t* z) {
    return get_imu_data(IMU_ACCEL, x, y, z);
}

/*
Calibrated gyroscope - drift-compensated rotational velocity

"The gyroscope calibrated sensor reports drift-compensated rotational velocity. The units are rad/s. The Q point is 9. The report ID is 0x02." (#1 p.60)

x, y, z are signed fixed-point
*/
uint8_t get_imu_cal_gyro(uint16_t* x, uint16_t* y, uint16_t* z) {
    return get_imu_data(IMU_CAL_GYRO, x, y, z);
}

/*
Uncalibrated gyroscope - non-drift-compensated rotational velocity

"The gyroscope uncalibrated sensor reports rotational velocity without drift compensation. An estimate of drift is also reported. The units for both values are rad/s. The Q point for both values is 9. The report ID is 0x07." (#1 p.60)

x, y, z are signed fixed-point

This does not use `get_imu_data()` because its input report format is different.
*/
uint8_t get_imu_uncal_gyro(uint16_t* x, uint16_t* y, uint16_t* z, uint16_t* bias_x, 
    uint16_t* bias_y, uint16_t* bias_z) {
    
    // Send set feature command, receive get feature response
    if (!enable_imu_feat(IMU_UNCAL_GYRO)) {
        return 0;
    }

    // Get input report packet
    // The received packet begins with the timebase reference (0xFB) (#0 p.44, #1 p.79)
    // Ignore base delta, sequence number, delay, etc.
    for (uint8_t i = 0; i < IMU_PACKET_CHECK_COUNT; i++) {
        if (!receive_imu_packet()) {
            continue;
        }
        if (imu_data_len < 21) {
            continue;
        }
        if (imu_data[0] != IMU_BASE_TIMESTAMP_REF) {
            continue;
        }
        if (imu_data[5] != IMU_UNCAL_GYRO) {
            continue;
        }

        if (x != NULL) {
            *x = (((uint16_t) imu_data[10]) << 8) | ((uint16_t) imu_data[9]);
        }
        if (y != NULL) {
            *y = (((uint16_t) imu_data[12]) << 8) | ((uint16_t) imu_data[11]);
        }
        if (z != NULL) {
            *z = (((uint16_t) imu_data[14]) << 8) | ((uint16_t) imu_data[13]);
        }
        if (bias_x != NULL) {
            *bias_x = (((uint16_t) imu_data[16]) << 8) | ((uint16_t) imu_data[15]);
        }
        if (bias_y != NULL) {
            *bias_y = (((uint16_t) imu_data[18]) << 8) | ((uint16_t) imu_data[17]);
        }
        if (bias_z != NULL) {
            *bias_z = (((uint16_t) imu_data[20]) << 8) | ((uint16_t) imu_data[19]);
        }

        // After getting data from the input report, disable the sensor so we don't keep receiving input report packets every 60ms
        // Send set feature command, receive get feature response
        if (!disable_imu_feat(IMU_UNCAL_GYRO)) {
            return 0;
        }

        return 1;
    }

    return 0;
}


// INT2 interrupt from INTn pin
ISR(INT2_vect) {
#ifdef IMU_VERBOSE
    print("\nINT2: pin = %u (%.2x)\n", get_imu_int(), PINB);
#endif
}
