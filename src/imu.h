#ifndef IMU_H
#define IMU_H

#include <stdint.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>

// 4 bytes in all headers
#define IMU_HEADER_LEN 4
// Max number of bytes to save in data buffer (not including header)
#define IMU_DATA_MAX_LEN 30

// Channels (#0 p.22)
#define IMU_CHANNEL_COUNT   6 // total number of channels
#define IMU_COMMAND         0 // general SHTP commands (e.g. advertisement, error list, #2 p.15)
#define IMU_EXECUTABLE      1
#define IMU_CONTROL         2 // "SH-2 control" channel
#define IMU_NON_WAKE_INPUT  3
#define IMU_WAKE_INPUT      4
#define IMU_GYRO_ROT        5

// Report IDs
// Request product ID (#1 p.36)
#define IMU_GET_FEAT_REQ        0xFE
#define IMU_SET_FEAT_CMD        0xFD
#define IMU_GET_FEAT_RESP       0xFC
#define IMU_BASE_TIMESTAMP_REF  0xFB
#define IMU_PRODUCT_ID_REQ      0xF9
#define IMU_PRODUCT_ID_RESP     0xF8
#define IMU_CMD_RESP            0xF1

// Input reports
#define IMU_ACCEL       0x01
#define IMU_CAL_GYRO    0x02
#define IMU_MAG         0x03
#define IMU_UNCAL_GYRO  0x07
#define IMU_RAW_GYRO    0x15

// Q points
#define IMU_ACCEL_Q         8
#define IMU_UNCAL_GYRO_Q    9
#define IMU_CAL_GYRO_Q      9

// Default report inteval (60ms, in microseconds)
#define IMU_DEF_REPORT_INTERVAL 0x0000EA60

// Number of packets to receive for checking a response from the IMU
#define IMU_PACKET_CHECK_COUNT 20


extern uint8_t imu_seq_nums[];

void init_imu(void);
void init_imu_pins(void);
void reset_imu(void);
void wake_imu(void);
uint8_t get_imu_int(void);
uint8_t wait_for_imu_int(void);
void start_imu_spi(void);
void end_imu_spi(void);
void process_imu_header(uint8_t* channel, uint8_t* seq_num, uint16_t* length);
uint8_t receive_imu_packet(void);
void populate_imu_header(uint8_t channel, uint8_t seq_num, uint16_t length);
uint8_t send_imu_packet(uint8_t channel);
uint8_t get_imu_prod_id(void);
uint8_t send_imu_set_feat_cmd(uint8_t feat_report_id, uint32_t report_interval);
uint8_t enable_imu_feat(uint8_t feat_report_id);
uint8_t disable_imu_feat(uint8_t feat_report_id);
double imu_raw_data_to_double(int16_t raw_data, uint8_t q_point);
uint8_t get_imu_accel(int16_t* x, int16_t* y, int16_t* z);
uint8_t get_imu_uncal_gyro(int16_t* x, int16_t* y, int16_t* z, int16_t* bias_x, 
    int16_t* bias_y, int16_t* bias_z);
uint8_t get_imu_cal_gyro(int16_t* x, int16_t* y, int16_t* z);

#endif
