#ifndef IMU_H
#define IMU_H

#include <stdint.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>

// 4 bytes in all headers
#define IMU_HEADER_LEN 4
// Max number of bytes to support in data (not including header)
#define IMU_DATA_MAX_LEN 20

// Channels (#0 p.22)
#define IMU_CHANNEL_COUNT 6 // total number of channels
#define IMU_CONTROL 2       // "SH-2 control" channel

// Report IDs
// Request product ID (#1 p.36)
#define IMU_PRODUCT_ID_REQ  0xF9
#define IMU_GET_FEAT_REQ    0xFE
#define IMU_SET_FEAT_CMD    0xFD
#define IMU_GET_FEAT_RESP   0xFC

// Input reports
#define IMU_ACCEL       0x01
#define IMU_RAW_GYRO    0x15
#define IMU_CAL_GYRO    0x02
#define IMU_UNCAL_GYRO  0x07
#define IMU_MAG         0x03

extern uint8_t imu_seq_nums[];

void init_imu(void);
void reset_imu(void);
void populate_imu_header(uint8_t channel, uint8_t seq_num, uint16_t length);
void process_imu_header(uint8_t* channel, uint8_t* seq_num, uint16_t* length);
uint8_t send_imu_packet(uint8_t channel);
uint8_t receive_imu_packet(void);
uint8_t get_imu_accel(uint16_t* x, uint16_t* y, uint16_t* z);
uint8_t wait_for_imu_int(void);
void start_imu_spi(void);
void end_imu_spi(void);
void send_imu_header(uint16_t length, uint8_t channel);
void receive_imu_header(uint16_t* length, uint8_t* channel, uint8_t* seq_num);
uint8_t receive_and_discard_imu_packet(void);

#endif
