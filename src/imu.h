#ifndef IMU_H
#define IMU_H

#include <stdint.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>

void init_imu(void);
void reset_imu(void);
void wait_for_imu_int(void);
void start_imu_spi(void);
void end_imu_spi(void);
void send_imu_shtp_header(uint16_t length, uint8_t channel);
void receive_imu_shtp_header(uint16_t *length, uint8_t *channel);

#endif
