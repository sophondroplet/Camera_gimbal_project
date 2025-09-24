/*
 * MPU6050.c
 *
 *  Created on: Feb 3, 2025
 *      Author: Lek
 */

#include "MPU6050.h"
#include <math.h>

uint8_t rx_data[14];
int16_t gyro[3];
int16_t temp;
int16_t acc[3];
uint8_t rx_flag;

float gyro_rps[3];
float gyropos[3];
float accpos[3];
float acc_mps2[3];
float gyro_drift[3];
extern float dt;

float roll;
float pitch;
float yaw;



void MPU6050_calc_drift(I2C_HandleTypeDef* handle){

	int16_t gyro_drift_sum [3];

	gyro_drift_sum[0] = 0;
	gyro_drift_sum[1] = 0;
	gyro_drift_sum[2] = 0;

	float gyro_drift_sum_rps [3];

	gyro_drift_sum_rps[0] = 0;
	gyro_drift_sum_rps[1] = 0;
	gyro_drift_sum_rps[2] = 0;


	for (int i = 0; i < 100; i++){

		HAL_I2C_Mem_Read(handle, MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, rx_data, 14,100);
		gyro_drift_sum[0] = ( (((int16_t) rx_data[8]) << 8) | ((int16_t) rx_data[9]) );
		gyro_drift_sum[1] = ( (((int16_t) rx_data[10]) << 8) | ((int16_t) rx_data[11]) );
		gyro_drift_sum[2] = ( (((int16_t) rx_data[12]) << 8) | ((int16_t) rx_data[13]) );

		gyro_drift_sum_rps[0] += (gyro_drift_sum[0]*RAW_TO_RPS);
		gyro_drift_sum_rps[1] += (gyro_drift_sum[1]*RAW_TO_RPS);
		gyro_drift_sum_rps[2] += (gyro_drift_sum[2]*RAW_TO_RPS);

		HAL_Delay(10);

	}

	gyro_drift[0] = gyro_drift_sum_rps[0]/100.0f;
	gyro_drift[1] = gyro_drift_sum_rps[1]/100.0f;
	gyro_drift[2] = gyro_drift_sum_rps[2]/100.0f;
}

HAL_StatusTypeDef MPU6050_init(I2C_HandleTypeDef* handle){
	HAL_StatusTypeDef status;
	uint8_t data = 0x01;
	status = HAL_I2C_Mem_Write(handle, MPU6050_I2C_ADDR, 0x6B, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	HAL_Delay(10);

	data = 0b00000001; /* DLPF = 1 (sample rate 1kHz), 188Hz BW Gyro */
	status = HAL_I2C_Mem_Write(handle, MPU6050_I2C_ADDR, MPU6050_REG_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	HAL_Delay(10);

	data = 0b00000001; /* DLPF = 1 (sample rate 1kHz), 188Hz BW Gyro */
	status = HAL_I2C_Mem_Write(handle, MPU6050_I2C_ADDR, MPU6050_REG_GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	HAL_Delay(10);

	data = 0b00001000; /* full scale range +/- 4g */
	status = HAL_I2C_Mem_Write(handle,  MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);


	data = 0b00110000; /* latch interrupt, clear on read */
	status = HAL_I2C_Mem_Write(handle,  MPU6050_I2C_ADDR, MPU6050_REG_INT_PIN_CFG, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	HAL_Delay(10);

	data = 0b00000001; /* data ready interrupt */
	status = HAL_I2C_Mem_Write(handle,  MPU6050_I2C_ADDR, MPU6050_REG_INT_ENABLE, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	HAL_Delay(10);

	data = 0b00000000; /* DLPF = 1 (sample rate 1kHz), 188Hz BW Gyro */
		status = HAL_I2C_Mem_Write(handle, MPU6050_I2C_ADDR, MPU6050_REG_SMPRT_DIV, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

	HAL_Delay(100);

	MPU6050_calc_drift(handle);

	HAL_Delay(100);

	return status;
}

HAL_StatusTypeDef MPU6050_read_DMA_data (I2C_HandleTypeDef *handle){
	HAL_StatusTypeDef status;
	status = HAL_I2C_Mem_Read_DMA(handle, MPU6050_I2C_ADDR, MPU6050_REG_ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, rx_data, 14);
	rx_flag = (status == HAL_OK);
	return status;
}

void MPU6050_process_data(){
	//shift buffer data into variables
	//raw data to rotation per second (131 LSB/deg/s)*(pi/180)

	acc[0]= ( (((int16_t) rx_data[0]) << 8) | ((int16_t) rx_data[1]) );
	acc[1] = ( (((int16_t) rx_data[2]) << 8) | ((int16_t) rx_data[3]) );
	acc[2] = ( (((int16_t) rx_data[4]) << 8) | ((int16_t) rx_data[5]) );

	temp = (rx_data[6]<<8)|rx_data[7];

	gyro[0] = ( (((int16_t) rx_data[8]) << 8) | ((int16_t) rx_data[9]) );
	gyro[1] = ( (((int16_t) rx_data[10]) << 8) | ((int16_t) rx_data[11]) );
	gyro[2] = ( (((int16_t) rx_data[12]) << 8) | ((int16_t) rx_data[13]) );


	gyro_rps[0] = (gyro[0]*RAW_TO_RPS) - gyro_drift[0];
	gyro_rps[1] = (gyro[1]*RAW_TO_RPS) - gyro_drift[1];
	gyro_rps[2] = (gyro[2]*RAW_TO_RPS) - gyro_drift[2];

	acc_mps2[0] = acc[0]*RAW_TO_MPS2;
	acc_mps2[1] = acc[1]*RAW_TO_MPS2;
	acc_mps2[2] = acc[2]*RAW_TO_MPS2;

//integrate over time
	float mpudt = 1.0f/1000.0f;

//	gyropos[0]+= gyro_rps[0]*mpudt;
//	gyropos[1]+= gyro_rps[1]*mpudt;
//	gyropos[2]+= gyro_rps[2]*mpudt;

//integrate over time and convert to global frame

	gyropos[0] = gyropos[0] + (gyro_rps[0] + tanf(gyropos[1])*((gyro_rps[1])*sinf(gyropos[0])+(gyro_rps[2])*cosf(gyropos[0])))*mpudt;
	gyropos[1] = gyropos[1] + ((gyro_rps[1])*cosf(gyropos[0])-(gyro_rps[2])*sinf(gyropos[0]))*mpudt;
	gyropos[2]+= gyro_rps[2]*mpudt;

//	//roll
	accpos[0] = atan2((acc[1] ),(sqrt(acc[0]*acc[0] + acc[2]*acc[2])));
	//pitch
	accpos[1] = atan2((-1 * acc[0] ),(sqrt(acc[1]*acc[1] + acc[2]*acc[2])));

	roll = 0.96*gyropos[0] + 0.04*accpos[0];
	pitch = 0.96*gyropos[1] + 0.04*accpos[1];
	yaw = 0.96*gyropos[2] + 0.4*accpos[2];


}
