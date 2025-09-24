/*
 * MPU6050.c
 *
 *  Created on: Sep 1, 2024
 *      Author: David
 */

#include "MPU6050_D.h"

uint8_t MPU6050_Initialize(MPU6050 *dev, I2C_HandleTypeDef *i2cHandle)
{
	dev-> i2cHandle = i2cHandle;

	dev->acc_mps2[0] = 0.0f;
	dev->acc_mps2[1] = 0.0f;
	dev->acc_mps2[2] = 0.0f;

	dev->tmp_c = 0.0f;

	dev->gyr_rps[0] = 0.0f;
	dev->gyr_rps[1] = 0.0f;
	dev->gyr_rps[2] = 0.0f;

	for (int n = 0; n < 14; n++)
	{
		dev->rxData[n] = 0;
	}

	dev->acc_offset[0] = 0;
	dev->acc_offset[1] = 0;
	dev->acc_offset[2] = 0;

	dev->gyr_offset[0] = 0;
	dev->gyr_offset[1] = 0;
	dev->gyr_offset[2] = 0;

	dev->newData = 0;

	dev->rxFlag = 0;

	dev->gotData = 0;

	uint8_t errNum = 0;
	HAL_StatusTypeDef status;

	uint8_t regData;

	status = MPU6050_Read_Register(dev, MPU6050_REG_WHO_AM_I, &regData);
	errNum += (status != HAL_OK);

	if (regData != (MPU6050_I2C_ADDR >> 1))
	{
		return 255;
	}

	regData = 0b00000001; /* wake up, set clock source to X gyro*/
	status = MPU6050_Write_Register(dev, MPU6050_REG_PWR_MGMT_1, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(10);

	regData = 0b00000000; /* sample rate division, 1 kHz/1 = 1000 Hz */
	status = MPU6050_Write_Register(dev, MPU6050_REG_SMPRT_DIV, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(10);

	regData = 0b00000001; /* DLPF = 1 (sample rate 1kHz), 188Hz BW Gyro */
	status = MPU6050_Write_Register(dev, MPU6050_REG_CONFIG, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(10);

	regData = 0b00000000; /* full scale range 250 degrees/s */
	status = MPU6050_Write_Register(dev, MPU6050_REG_GYRO_CONFIG, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(10);

	regData = 0b00001000; /* full scale range +/- 4g */
	status = MPU6050_Write_Register(dev, MPU6050_REG_ACCEL_CONFIG, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(10);

	regData = 0b00110000; /* latch interrupt, clear on read */
	status = MPU6050_Write_Register(dev, MPU6050_REG_INT_PIN_CFG, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(10);

	regData = 0b00000001; /* data ready interrupt */
	status = MPU6050_Write_Register(dev, MPU6050_REG_INT_ENABLE, &regData);
	errNum += (status != HAL_OK);

	HAL_Delay(100);

	return errNum;
}

HAL_StatusTypeDef MPU6050_Read_DMA(MPU6050 *dev)
{
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read_DMA(dev->i2cHandle, MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, dev->rxData, 14);

	dev->rxFlag = (status == HAL_OK);
	dev->newData = 0;

	return status;
}

void MPU6050_Process_DMA_Data(MPU6050 *dev)
{
	int16_t accRaw[3];
	accRaw[0] = ( (((int16_t) dev->rxData[0]) << 8) | ((int16_t) dev->rxData[1]) );
	accRaw[1] = ( (((int16_t) dev->rxData[2]) << 8) | ((int16_t) dev->rxData[3]) );
	accRaw[2] = ( (((int16_t) dev->rxData[4]) << 8) | ((int16_t) dev->rxData[5]) );

	int16_t tmpRaw = ( (((int16_t) dev->rxData[6]) << 8) | ((int16_t) dev->rxData[7]) );

	int16_t gyrRaw[3];
	gyrRaw[0] = ( (((int16_t) dev->rxData[8]) << 8) | ((int16_t) dev->rxData[9]) );
	gyrRaw[1] = ( (((int16_t) dev->rxData[10]) << 8) | ((int16_t) dev->rxData[11]) );
	gyrRaw[2] = ( (((int16_t) dev->rxData[12]) << 8) | ((int16_t) dev->rxData[13]) );

	/* convert to m/s^2 using +/- 4g setting */
	dev->acc_mps2[0] = RAW_TO_MPS2 * (accRaw[0] - dev->acc_offset[0]);
	dev->acc_mps2[1] = RAW_TO_MPS2 * (accRaw[1] - dev->acc_offset[1]);
	dev->acc_mps2[2] = RAW_TO_MPS2 * (accRaw[2] - dev->acc_offset[2]);

	dev->tmp_c = (float) tmpRaw / 340.0f + 36.53f;

	/* convert to rad/s using +/- 250 degrees/s setting */
	dev->gyr_rps[0] = RAW_TO_RPS * (gyrRaw[0] - dev->gyr_offset[0]);
	dev->gyr_rps[1] = RAW_TO_RPS * (gyrRaw[1] - dev->gyr_offset[1]);
	dev->gyr_rps[2] = RAW_TO_RPS * (gyrRaw[2] - dev->gyr_offset[2]);
}

HAL_StatusTypeDef MPU6050_Read_Temperature(MPU6050 *dev)
{
	uint8_t regData[2];

	HAL_StatusTypeDef status = MPU6050_Read_Registers(dev, MPU6050_REG_TEMP_OUT_H, regData, 2);

	int16_t tempRaw = ( (((int16_t) regData[0]) << 8) | ((int16_t) regData[1]) );
	dev->tmp_c = (float) tempRaw / 340.0f + 36.53f;

	return status;
}

HAL_StatusTypeDef MPU6050_Read_Accelerometer(MPU6050 *dev)
{
	uint8_t regData[6];

	HAL_StatusTypeDef status = MPU6050_Read_Registers(dev, MPU6050_REG_ACCEL_XOUT_H, regData, 6);

	int16_t accRaw[3];
	accRaw[0] = ( (((int16_t) regData[0]) << 8) | ((int16_t) regData[1]) );
	accRaw[1] = ( (((int16_t) regData[2]) << 8) | ((int16_t) regData[3]) );
	accRaw[2] = ( (((int16_t) regData[4]) << 8) | ((int16_t) regData[5]) );

	/* convert to m/s^2 using +/- 4g setting */
	dev->acc_mps2[0] = RAW_TO_MPS2 * (accRaw[0] - dev->acc_offset[0]);
	dev->acc_mps2[1] = RAW_TO_MPS2 * (accRaw[1] - dev->acc_offset[1]);
	dev->acc_mps2[2] = RAW_TO_MPS2 * (accRaw[2] - dev->acc_offset[2]);

	return status;
}

HAL_StatusTypeDef MPU6050_Read_Gyroscope(MPU6050 *dev)
{
	uint8_t regData[6];

	HAL_StatusTypeDef status = MPU6050_Read_Registers(dev, MPU6050_REG_GYRO_XOUT_H, regData, 6);

	int16_t gyrRaw[3];
	gyrRaw[0] = ( (((int16_t) regData[0]) << 8) | ((int16_t) regData[1]) );
	gyrRaw[1] = ( (((int16_t) regData[2]) << 8) | ((int16_t) regData[3]) );
	gyrRaw[2] = ( (((int16_t) regData[4]) << 8) | ((int16_t) regData[5]) );

	/* convert to rad/s using +/- 250 degrees/s setting */
	dev->gyr_rps[0] = RAW_TO_RPS * (gyrRaw[0] - dev->gyr_offset[0]);
	dev->gyr_rps[1] = RAW_TO_RPS * (gyrRaw[1] - dev->gyr_offset[1]);
	dev->gyr_rps[2] = RAW_TO_RPS * (gyrRaw[2] - dev->gyr_offset[2]);

	return status;
}

HAL_StatusTypeDef MPU6050_Read_Register(MPU6050 *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, MPU6050_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPU6050_Read_Registers(MPU6050 *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, MPU6050_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPU6050_Write_Register(MPU6050 *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, MPU6050_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}
