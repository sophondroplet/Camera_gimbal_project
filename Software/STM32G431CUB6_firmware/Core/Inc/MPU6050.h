/*
 * MPU6050.h
 *
 *  Created on: Feb 3, 2025
 *      Author: Lek
 */
#ifndef MPU6050_H_
#define MPU6050_H_

#include "stm32g4xx_hal.h"

#define MPU6050_I2C_ADDR (0x68 << 1)
#define M_PI 3.14159265358979323846
#define M_SQRT3_OVER_2 0.8660254037844

#define MPU6050_REG_SMPRT_DIV 		0x19
#define MPU6050_REG_CONFIG 			0x1A
#define MPU6050_REG_GYRO_CONFIG		0x1B
#define MPU6050_REG_ACCEL_CONFIG	0x1C
#define MPU6050_FIFO_EN				0x23
#define MPU6050_REG_INT_PIN_CFG		0x37
#define MPU6050_REG_INT_ENABLE		0x38
#define MPU6050_REG_ACCEL_XOUT_H	0x3B
#define MPU6050_REG_ACCEL_XOUT_L	0x3C
#define MPU6050_REG_ACCEL_YOUT_H	0x3D
#define MPU6050_REG_ACCEL_YOUT_L	0x3E
#define MPU6050_REG_ACCEL_ZOUT_H	0x3F
#define MPU6050_REG_ACCEL_ZOUT_L	0x40
#define MPU6050_REG_TEMP_OUT_H		0x41
#define MPU6050_REG_TEMP_OUT_L		0x42
#define MPU6050_REG_GYRO_XOUT_H		0x43
#define MPU6050_REG_GYRO_XOUT_L		0x44
#define MPU6050_REG_GYRO_YOUT_H		0x45
#define MPU6050_REG_GYRO_YOUT_L		0x46
#define MPU6050_REG_GYRO_ZOUT_H		0x47
#define MPU6050_REG_GYRO_ZOUT_L		0x48
#define MPU6050_REG_USER_CTRL		0x6A
#define MPU6050_REG_PWR_MGMT_1		0x6B
#define MPU6050_REG_WHO_AM_I 		0x75

#define RAW_TO_RPS 		0.00013323124061f
#define RAW_TO_MPS2		0.00119750976563f

#define RPS_TO_RAW		7505.74711623f
#define MPS2_TO_RAW		835.066258916f

extern uint8_t rx_data[14];
extern int16_t gyro[3];
extern int16_t temp;
extern int16_t acc[3];

extern float gyro_rps[3];
extern float gyropos[3];
extern float accpos[3];
extern float acc_mps2[3];
extern float gyro_drift[3];
extern float dt;

HAL_StatusTypeDef MPU6050_init(I2C_HandleTypeDef *handle);
HAL_StatusTypeDef MPU6050_read_DMA_data (I2C_HandleTypeDef *handle);
void MPU6050_process_data();
void MPU6050_calc_drift(I2C_HandleTypeDef* handle);

extern uint8_t IMU_flag;

extern float roll;
extern float pitch;
extern float yaw;

extern uint8_t rx_flag;

#endif /* MPU6050_H_ */
