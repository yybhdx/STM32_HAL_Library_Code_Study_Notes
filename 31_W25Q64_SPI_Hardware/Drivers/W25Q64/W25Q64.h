/**
  ******************************************************************************
  * @file           : W25Q64.h
  * @brief          : W25Q64 Flash 存储器驱动头文件
  ******************************************************************************
  */

#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f1xx_hal.h"

/* 外部句柄声明 ------------------------------------------------------------*/
extern SPI_HandleTypeDef hspi1; 

/* 引脚宏定义 --------------------------------------------------------------*/
#define W25Q64_CS_GPIO_PORT    GPIOA
#define W25Q64_CS_PIN          GPIO_PIN_4

/* 函数原型声明 ------------------------------------------------------------*/

/**
  * @brief  初始化W25Q64
  * @param  无
  * @retval 无
  */
void W25Q64_Init(void);

/**
  * @brief  读取W25Q64的ID号
  * @param  MID: 输出参数，返回厂家ID (Manufacturer ID)
  * @param  DID: 输出参数，返回设备ID (Device ID)
  * @retval 无
  */
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID);

/**
  * @brief  W25Q64页编程
  * @param  Address: 起始地址，范围：0x000000 ~ 0x7FFFFF
  * @param  DataArray: 要写入数据的数组首地址
  * @param  Count: 要写入数据的数量，范围：0 ~ 256
  * @note   写入地址不能跨页（256字节为一个扇区页面）
  * @retval 无
  */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);

/**
  * @brief  W25Q64扇区擦除 (4KB)
  * @param  Address: 指定扇区的地址，范围：0x000000 ~ 0x7FFFFF
  * @retval 无
  */
void W25Q64_SectorErase(uint32_t Address);

/**
  * @brief  W25Q64读取数据
  * @param  Address: 读取数据的起始地址，范围：0x000000 ~ 0x7FFFFF
  * @param  DataArray: 用于接收数据的数组首地址
  * @param  Count: 要读取数据的数量
  * @retval 无
  */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

#endif