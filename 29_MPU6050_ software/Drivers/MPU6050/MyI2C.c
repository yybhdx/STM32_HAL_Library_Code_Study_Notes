#include "stm32f1xx_hal.h"  // 注意：如果您使用的不是F1系列，请修改为对应的头文件，例如 stm32f4xx_hal.h 或直接包含 "main.h"
#include "Delay.h"          // 确保您的工程中已经实现了 Delay_us 延时函数

/*引脚配置层*/

/**
  * 函    数：I2C写SCL引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SCL的电平，范围0~1
  * 返 回 值：无
  * 注意事项：当BitValue为0时，置SCL为低电平，当BitValue为1时，置SCL为高电平
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
    // HAL库使用 GPIO_PIN_SET 和 GPIO_PIN_RESET 设置引脚状态
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (BitValue == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET); 
    Delay_us(10);                                               // 延时10us，防止时序频率超过要求
}

/**
  * 函    数：I2C写SDA引脚电平
  * 参    数：BitValue 协议层传入的当前需要写入SDA的电平，范围0~1
  * 返 回 值：无
  * 注意事项：当BitValue为0时，置SDA为低电平，当BitValue为1时，置SDA为高电平
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, (BitValue == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET); 
    Delay_us(10);                                               // 延时10us，防止时序频率超过要求
}

/**
  * 函    数：I2C读SDA引脚电平
  * 参    数：无
  * 返 回 值：协议层需要得到的当前SDA的电平，范围0~1
  * 注意事项：当前SDA为低电平时，返回0，当前SDA为高电平时，返回1
  */
uint8_t MyI2C_R_SDA(void)
{
    uint8_t BitValue;
    // 读取引脚状态，如果是 GPIO_PIN_SET 则返回1，否则返回0
    BitValue = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) ? 1 : 0; 
    Delay_us(10);                                               // 延时10us，防止时序频率超过要求
    return BitValue;                                            // 返回SDA电平
}

/**
  * 函    数：I2C初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：实现SCL和SDA引脚的初始化 (PB10, PB11 开漏输出)
  */
void MyI2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 开启GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE(); 
    
    /* 设置默认电平 */
    // 初始化前先将引脚电平拉高，保证初始化后默认处于释放总线状态，防止产生错误电平跳变
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_SET); 

    /* GPIO初始化配置 */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;         // 开漏输出模式 (Open Drain)
    GPIO_InitStruct.Pull = GPIO_NOPULL;                 // 外部一般有上拉电阻，此处设为不带内部上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;       // 对应标准库的50MHz高速模式
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/*协议层*/

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void MyI2C_Start(void)
{
    MyI2C_W_SDA(1);                         // 释放SDA，确保SDA为高电平
    MyI2C_W_SCL(1);                         // 释放SCL，确保SCL为高电平
    MyI2C_W_SDA(0);                         // 在SCL高电平期间，拉低SDA，产生起始信号
    MyI2C_W_SCL(0);                         // 起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void MyI2C_Stop(void)
{
    MyI2C_W_SDA(0);                         // 拉低SDA，确保SDA为低电平
    MyI2C_W_SCL(1);                         // 释放SCL，使SCL呈现高电平
    MyI2C_W_SDA(1);                         // 在SCL高电平期间，释放SDA，产生终止信号
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i ++)                // 循环8次，主机依次发送数据的每一位
    {
        /* 两个!可以对数据进行两次逻辑取反，作用是把非0值统一转换为1，即：!!(0) = 0，!!(非0) = 1 */
        MyI2C_W_SDA(!!(Byte & (0x80 >> i)));// 使用掩码的方式取出Byte的指定一位数据并写入到SDA线
        MyI2C_W_SCL(1);                     // 释放SCL，从机在SCL高电平期间读取SDA
        MyI2C_W_SCL(0);                     // 拉低SCL，主机开始发送下一位数据
    }
}

/**
  * 函    数：I2C接收一个字节
  * 参    数：无
  * 返 回 值：接收到的一个字节数据，范围：0x00~0xFF
  */
uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;                 // 定义接收的数据，并赋初值0x00，此处必须赋初值0x00，后面会用到
    MyI2C_W_SDA(1);                         // 接收前，主机先确保释放SDA，避免干扰从机的数据发送
    for (i = 0; i < 8; i ++)                // 循环8次，主机依次接收数据的每一位
    {
        MyI2C_W_SCL(1);                     // 释放SCL，主机在SCL高电平期间读取SDA
        if (MyI2C_R_SDA()){Byte |= (0x80 >> i);}    // 读取SDA数据，并存储到Byte变量
                                                    // 当SDA为1时，置变量指定位为1，当SDA为0时，不做处理，指定位为默认的初值0
        MyI2C_W_SCL(0);                     // 拉低SCL，从机在SCL低电平期间准备下一位数据
    }
    return Byte;                            // 返回接收到的一个字节数据
}

/**
  * 函    数：I2C发送应答位
  * 参    数：AckBit 要发送的应答位，范围：0~1，0表示应答，1表示非应答
  * 返 回 值：无
  */
void MyI2C_SendAck(uint8_t AckBit)
{
    MyI2C_W_SDA(AckBit);                    // 主机把应答位数据放到SDA线
    MyI2C_W_SCL(1);                         // 释放SCL，从机在SCL高电平期间，读取应答位
    MyI2C_W_SCL(0);                         // 拉低SCL，开始下一个时序模块
}

/**
  * 函    数：I2C接收应答位
  * 参    数：无
  * 返 回 值：接收到的应答位，范围：0~1，0表示应答，1表示非应答
  */
uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t AckBit;                         // 定义应答位变量
    MyI2C_W_SDA(1);                         // 接收前，主机先确保释放SDA，避免干扰从机的数据发送
    MyI2C_W_SCL(1);                         // 释放SCL，主机在SCL高电平期间读取SDA
    AckBit = MyI2C_R_SDA();                 // 将应答位存储到变量里
    MyI2C_W_SCL(0);                         // 拉低SCL，开始下一个时序模块
    return AckBit;                          // 返回应答位变量
}