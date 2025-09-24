/*
 * MPU6050.h
 *
 *  Created on: Sep 1, 2024
 *      Author: David
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "stm32g4xx_hal.h" /* For I2C */

#define MPU6050_I2C_ADDR (0x68 << 1)

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

typedef struct
{
	/* I2C handle */
	I2C_HandleTypeDef *i2cHandle;

	/* Accelerometer data [X, Y, Z], (m/s^2) */
	float acc_mps2[3];

	/* Temperature data (deg C) */
	float tmp_c;

	/* Gyroscope data [X, Y, Z], (rad/s) */
	float gyr_rps[3];

	/* DMA receive buffer */
	uint8_t rxData[14];

	/* bias */
	int16_t acc_offset[3];
	int16_t gyr_offset[3];

	/* Set to 1 when IMU interrupt goes high */
	volatile uint8_t newData;

	/* Set to 1 when DMA receiving */
	volatile uint8_t rxFlag;

	/* Set to 1 when DMA done */
	volatile uint8_t gotData;

} MPU6050;

uint8_t MPU6050_Initialize(MPU6050 *dev, I2C_HandleTypeDef *i2cHandle);

HAL_StatusTypeDef MPU6050_Read_DMA(MPU6050 *dev);
void MPU6050_Process_DMA_Data(MPU6050 *dev);

HAL_StatusTypeDef MPU6050_Read_Temperature(MPU6050 *dev);
HAL_StatusTypeDef MPU6050_Read_Accelerometer(MPU6050 *dev);
HAL_StatusTypeDef MPU6050_Read_Gyroscope(MPU6050 *dev);

HAL_StatusTypeDef MPU6050_Read_Register(MPU6050 *dev, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef MPU6050_Read_Registers(MPU6050 *dev, uint8_t reg, uint8_t *data, uint8_t length);
HAL_StatusTypeDef MPU6050_Write_Register(MPU6050 *dev, uint8_t reg, uint8_t *data);

#endif /* INC_MPU6050_H_ */
