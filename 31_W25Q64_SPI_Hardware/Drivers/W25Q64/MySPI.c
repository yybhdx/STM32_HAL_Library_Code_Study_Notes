#include "gpio.h"
#include "stdint.h"

/*
(GPIO_PinState)BitValue 是什么？

  这是 C语言的强制类型转换（cast），把 BitValue（uint8_t 类型）转换成 GPIO_PinState（枚举类型）。
  先看 HAL 库中 GPIO_PinState 的定义（stm32f1xx_hal_gpio.h:65-68）：
  typedef enum {
      GPIO_PIN_RESET = 0u,  // 低电平
      GPIO_PIN_SET          // 高电平，值为 1
  } GPIO_PinState;

  再看 HAL_GPIO_WritePin 的函数签名：

  void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);

  第三个参数要求的是 GPIO_PinState 枚举类型，而你的 BitValue 是
  uint8_t。编译器对枚举类型检查较严格（可能产生警告），所以用 (GPIO_PinState)BitValue 强制转换一下，消除警告。

  当 BitValue 为 0 或 1 时有什么区别？

  效果完全等价：

  ┌──────────┬──────────────────┬────────────────┬────────────────┐
  │ BitValue │      转换后      │     等价于     │      效果      │
  ├──────────┼──────────────────┼────────────────┼────────────────┤
  │ 0        │ (GPIO_PinState)0 │ GPIO_PIN_RESET │ 引脚输出低电平 │
  ├──────────┼──────────────────┼────────────────┼────────────────┤
  │ 1        │ (GPIO_PinState)1 │ GPIO_PIN_SET   │ 引脚输出高电平 │
  └──────────┴──────────────────┴────────────────┴────────────────┘

  因为枚举的本质就是整数，GPIO_PIN_RESET = 0，GPIO_PIN_SET = 1，数值完全对应。传 0 就是拉低（片选有效、时钟拉低等），传
  1 就是拉高（片选释放、时钟拉高等）。

  简单总结：这个 cast 只是为了满足函数签名中枚举类型的要求，编译器的类型检查更严格而已，运行时没有任何区别。
*/

/*引脚配置层*/

/**
 * 函    数：SPI写SS引脚电平
 * 参    数：BitValue 0=低电平 1=高电平
 * 返 回 值：无
 */
void MySPI_W_SS(uint8_t BitValue)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (GPIO_PinState)BitValue);
}

/**
 * 函    数：SPI写SCK引脚电平
 */
void MySPI_W_SCK(uint8_t BitValue)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (GPIO_PinState)BitValue);
}

/**
 * 函    数：SPI写MOSI引脚电平
 */
void MySPI_W_MOSI(uint8_t BitValue)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (GPIO_PinState)BitValue);
}

/**
 * 函    数：SPI读MISO引脚电平
 */
uint8_t MySPI_R_MISO(void)
{
  return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
}

/**
 * 函    数：SPI初始化（软件SPI，GPIO模拟）
 */
void MySPI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 开启GPIOA时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // PA4/5/7 推挽输出
  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // PA6 上拉输入 MISO线(主机输入，从机输出)
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // 默认电平
  MySPI_W_SS(1);  // SS高电平，未选中
  MySPI_W_SCK(0); // SCK初始为低电平
}

/*协议层*/

/**
 * 函    数：SPI起始
 */
void MySPI_Start(void)
{
  MySPI_W_SS(0);
}

/**
 * 函    数：SPI终止
 */
void MySPI_Stop(void)
{
  MySPI_W_SS(1);
}

/**
 * 函    数：SPI交换一个字节（模式0）
 * 参    数：要发送的字节
 * 返回值：接收到的字节
 */
uint8_t MySPI_SwapByte(uint8_t ByteSend)
{
  uint8_t i, ByteReceive = 0x00;

  for (i = 0; i < 8; i++)
  {
    // 发送高位
    MySPI_W_MOSI(!!(ByteSend & (0x80 >> i)));

    // 上升沿读
    MySPI_W_SCK(1);

    // 读取MISO
    if (MySPI_R_MISO())
    {
      ByteReceive |= (0x80 >> i);
    }

    // 下降沿
    MySPI_W_SCK(0);
  }

  return ByteReceive;
}
