/*
 * mpu6050.h
 *
 *  Created on: Nov 13, 2019
 *      Author: Bulanov Konstantin
 */

#ifndef INC_GY521_H_
#define INC_GY521_H_

#endif /* INC_GY521_H_ */

#include <stdint.h>
#include "i2c.h"

// MPU6050 structure
typedef struct
{

    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;

    float Temperature;

    double KalmanAngleX;
    double KalmanAngleY;
} MPU6050_t;

// Kalman structure
typedef struct
{
    double Q_angle;
    double Q_bias;
    double R_measure;
    double angle;
    double bias;
    double P[2][2];
} Kalman_t;

// MPU6050 传感器的初始化函数，是使用传感器前必须调用的核心函数。
uint8_t MPU6050_Init(I2C_HandleTypeDef *I2Cx);

// 读取加速度计的原始数据并转换为实际物理值
void MPU6050_Read_Accel(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

// 读取陀螺仪的原始数据并转换为实际物理值
void MPU6050_Read_Gyro(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

// 读取 MPU6050 内置温度传感器的温度值
void MPU6050_Read_Temp(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

// 一次性读取加速度计、陀螺仪、温度的所有数据，并通过卡尔曼滤波计算出姿态角（横滚角 Roll、俯仰角 Pitch）。
void MPU6050_Read_All(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct);

// 卡尔曼滤波核心函数，融合 “加速度计计算的角度” 和 “陀螺仪积分的角度”，输出高精度、低漂移的姿态角。
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt);

// 读取设备的ID号
uint8_t MPU6050_Read_DeviceID(I2C_HandleTypeDef *I2Cx);