/**
  ******************************************************************************
  * @file           : W25Q64.c
  * @brief          : W25Q64 Flash 存储器驱动源文件（基于HAL库）
  ******************************************************************************
  */

#include "W25Q64.h"
#include "W25Q64_Ins.h"

/* 私有函数原型 ------------------------------------------------------------*/
static void W25Q64_CS_Select(void);
static void W25Q64_CS_Deselect(void);
static uint8_t W25Q64_SPI_SwapByte(uint8_t byte);
static void W25Q64_WriteEnable(void);
static void W25Q64_WaitBusy(void);

/**
  * @brief  片选使能（拉低CS）
  */
static void W25Q64_CS_Select(void) {
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_PORT, W25Q64_CS_PIN, GPIO_PIN_RESET);
}

/**
  * @brief  片选禁止（拉高CS）
  */
static void W25Q64_CS_Deselect(void) {
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_PORT, W25Q64_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  SPI底层单字节交换函数
  * @param  byte: 准备发送的一个字节
  * @retval 接收到的一个字节
  */
static uint8_t W25Q64_SPI_SwapByte(uint8_t byte) {
    uint8_t receiveByte;
    // 使用HAL库同步收发，超时设定为1000ms
    if (HAL_SPI_TransmitReceive(&hspi1, &byte, &receiveByte, 1, 1000) != HAL_OK) {
        return 0xFF; 
    }
    return receiveByte;
}

/**
  * @brief  W25Q64驱动初始化
  */
void W25Q64_Init(void) {
    W25Q64_CS_Deselect(); // 默认状态拉高片选，停止通信
}

/**
  * @brief  读取厂家ID和设备ID
  */
void W25Q64_ReadID(uint8_t *MID, uint16_t *DID) {
    W25Q64_CS_Select();
    W25Q64_SPI_SwapByte(W25Q64_JEDEC_ID); // 发送指令 0x9F
    *MID = W25Q64_SPI_SwapByte(W25Q64_DUMMY_BYTE); // 读取MID
    *DID = W25Q64_SPI_SwapByte(W25Q64_DUMMY_BYTE); // 读取DID高8位
    *DID <<= 8;
    *DID |= W25Q64_SPI_SwapByte(W25Q64_DUMMY_BYTE); // 读取DID低8位
    W25Q64_CS_Deselect();
}

/**
  * @brief  写使能指令
  * @note   在Page Program, Sector Erase等写操作前必须调用
  */
static void W25Q64_WriteEnable(void) {
    W25Q64_CS_Select();
    W25Q64_SPI_SwapByte(W25Q64_WRITE_ENABLE);
    W25Q64_CS_Deselect();
}

/**
  * @brief  等待Flash操作完成（检测忙状态）
  * @note   循环读取状态寄存器1的BUSY位，直至其为0
  */
static void W25Q64_WaitBusy(void) {
    uint32_t Timeout = 100000;
    W25Q64_CS_Select();
    W25Q64_SPI_SwapByte(W25Q64_READ_STATUS_REGISTER_1);
    while (Timeout > 0) {
        // 读取状态寄存器，BUSY位在最低位
        if ((W25Q64_SPI_SwapByte(W25Q64_DUMMY_BYTE) & 0x01) == 0x00) {
            break; 
        }
        Timeout--;
    }
    W25Q64_CS_Deselect();
}

/**
  * @brief  页编程（向Flash写入数据）
  */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count) {
    W25Q64_WriteEnable();
    W25Q64_CS_Select();
    
    W25Q64_SPI_SwapByte(W25Q64_PAGE_PROGRAM); // 发送页编程指令
    W25Q64_SPI_SwapByte(Address >> 16);        // 发送地址高位
    W25Q64_SPI_SwapByte(Address >> 8);
    W25Q64_SPI_SwapByte(Address);
    
    // 使用HAL库批量发送数据，提高传输速度
    HAL_SPI_Transmit(&hspi1, DataArray, Count, 1000);
    
    W25Q64_CS_Deselect();
    W25Q64_WaitBusy(); // 等待Flash写入完成
}

/**
  * @brief  扇区擦除（4KB）
  */
void W25Q64_SectorErase(uint32_t Address) {
    W25Q64_WriteEnable();
    W25Q64_CS_Select();
    W25Q64_SPI_SwapByte(W25Q64_SECTOR_ERASE_4KB);
    W25Q64_SPI_SwapByte(Address >> 16);
    W25Q64_SPI_SwapByte(Address >> 8);
    W25Q64_SPI_SwapByte(Address);
    W25Q64_CS_Deselect();
    W25Q64_WaitBusy(); // 擦除操作比较耗时，必须等待忙结束
}

/**
  * @brief  连续读取Flash数据
  */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count) {
    W25Q64_CS_Select();
    
    W25Q64_SPI_SwapByte(W25Q64_READ_DATA); // 发送读取指令
    W25Q64_SPI_SwapByte(Address >> 16);    // 发送起始地址
    W25Q64_SPI_SwapByte(Address >> 8);
    W25Q64_SPI_SwapByte(Address);
    
    // 使用HAL库批量接收数据
    HAL_SPI_Receive(&hspi1, DataArray, Count, 1000);
    
    W25Q64_CS_Deselect();
}