#ifndef IMU_H
#define IMU_H

#include <stdint.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>

extern uint8_t imu_seq_nums[];


void init_imu(void);
void reset_imu(void);
uint8_t wait_for_imu_int(void);
void start_imu_spi(void);
void end_imu_spi(void);
void send_imu_header(uint16_t length, uint8_t channel);
void receive_imu_header(uint16_t* length, uint8_t* channel, uint8_t* seq_num);
uint8_t receive_and_discard_imu_packet(void);

#endif
