/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// 添加OLED显示屏的头文件
#include "OLED.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
char arr[] = {'B'};
char arr1[] = "Hello World!"; // �Զ����\0

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t cnt = 0;
uint32_t IR_temp = 0;
uint8_t IR_buf[4] = {0}; // 用于存储接收到的红外信号数据，初始值为0
/*
 * 功能:通过测量红外接收头引脚上两个下降沿之间的时间差，来解码NEC协议的红外遥控信号。
 * 参数htim：指向TIM_HandleTypeDef结构体的指针，表示发生输入捕获事件的定时器句柄。
 * 返回值：无
 */

/*
为什么用右移？
因为NEC协议发送数据是 低位在前（LSB First）。最先收到的bit应该放在最低位。
每次收到新的bit时，先将之前的数据整体向右移动一位，然后把新收到的状态（0或1）填补在最高位（0x80是二进制的1000 0000）。
移满8次后，最先收到的bit就刚好被推到了最低位（bit 0）。
*/
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  // 判断是哪个定时器发生了输入捕获事件
  if (htim->Instance == TIM3)
  {
    uint32_t time_us = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_3); // 读取当前捕获到的时间值（即距离上一次下降沿过去了多少微秒）

    // 根据捕获的时间值判断是否为有效的红外信号脉冲
    if (time_us > 12000 && time_us < 15000) /// 9ms+4.5ms = 13500us(说明是NEC协议的起始脉冲)
    {
      cnt = 0;     // 接收到起始信号，清空位数计数器
      IR_temp = 0; // 清空临时数据寄存器
    }
    else if (time_us > 900 && time_us < 1300) // 接收到bit0的脉冲，0.56ms+0.56ms = 1120us
    {
      cnt++;
      IR_temp >>= 1;            // 右移一位，准备接收下一个bit
      IR_temp = IR_temp | 0x00; // 最高位置0（这句其实不起实质作用，因为右移默认补0，但写上逻辑更清晰）
    }
    else if (time_us > 2000 && time_us < 2500) // 接收到bit1的脉冲，0.56ms+1.69ms = 2250us
    {
      cnt++;
      IR_temp >>= 1; // 右移一位，准备接收下一个bit

      // 虽然 IR_temp 是个 32 位变量，但当代码执行 IR_temp = IR_temp | 0x80; 时，它强制写入“1”的位置仅仅是第 7 位（从0开始算），也就是最低那一个字节的最高位，而不是 32 位变量真正的最高位（第31位）。
      /*
      当 cnt == 9 时，又来了一个新 bit 填进第 7 位。同时 IR_temp >>= 1。
      此时，第 1 个字节的最低位（bit 0）去哪了？它被移出了范围，直接被丢弃了！
      等到 cnt == 16 移满第 16 次时，第 1 个字节的 8 个 bit 已经全部被挤出去了，此时 IR_temp 里面装的完完全全是第 2 个字节的全新数据。
      */
      IR_temp = IR_temp | 0x80; // 将最高位（bit 7）置为1
    }
    else
    {
      // 如果时间不在上述任何一个范围内，说明遇到了噪声干扰或者接收错误，直接放弃当前接收，状态复位。
      cnt = 0;
      IR_temp = 0;
    }

    if (cnt == 8) // 当接收到8个bit后，说明已经接收完整的一个字节数据
    {
      IR_buf[0] = IR_temp; // 将接收到的字节数据存储到IR_buf数组中
    }
    else if (cnt == 16) // 当接收到16个bit后，说明已经接收完整的两个字节数据
    {
      IR_buf[1] = IR_temp; // 将接收到的字节数据存储到IR_buf数组中
    }
    else if (cnt == 24) // 当接收到24个bit后，说明已经接收完整的三个字节数据
    {
      IR_buf[2] = IR_temp; // 将接收到的字节数据存储到IR_buf数组中
    }
    else if (cnt == 32) // 当接收到32个bit后，说明已经接收完整的四个字节数据
    {
      IR_buf[3] = IR_temp; // 将接收到的字节数据存储到IR_buf数组中
    }

    __HAL_TIM_SET_COUNTER(&htim3, 0); // 将TIM3的计数器清零，准备接收下一个红外信号脉冲
  }
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // OLED显示屏初始化
  OLED_Init();
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_3);                                             // 启动TIM3的输入捕获功能，使用中断方式
  __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING); // 设置捕获极性为下降沿
  __HAL_TIM_SET_COUNTER(&htim3, 0);                                                       // 将TIM3的计数器清零 == TIM3->CNT = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (cnt == 32) // 当接收到完整的32个bit数据后，说明已经接收完整的一个红外信号帧，可以进行处理和显示
    {
      cnt = 0;                          // 接收完成后将计数器清零，准备接收下一个红外信号
      OLED_ShowNum(1, 1, IR_buf[0], 4); // 地址码在OLED显示屏的第一行第一列显示接收到的第一个字节数据，长度为4位
      OLED_ShowNum(2, 1, IR_buf[1], 4); // 地址反码在OLED显示屏的第二行第一列显示接收到的第二个字节数据，长度为4位
      OLED_ShowNum(3, 1, IR_buf[2], 4); // 命令码在OLED显示屏的第三行第一列显示接收到的第三个字节数据，长度为4位
      OLED_ShowNum(4, 1, IR_buf[3], 4); // 命令反码在OLED显示屏的第四行第一列显示接收到的第四个字节数据，长度为4位

      if (IR_buf[0] == (uint8_t)~IR_buf[1] && IR_buf[2] == (uint8_t)~IR_buf[3]) // 如果地址码和地址反码互补，命令码和命令反码互补，说明接收到的红外信号数据是有效的，可以进行进一步处理
      {
        switch (IR_buf[2])
        {
        case 0X45: // 如果命令码是0X45，说明按下了遥控器上的数字键1，可以执行相应的操作
          /* code */
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // 红灯开
          break;
        case 0x46:
          /* code */
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET); // 绿灯开
          break;
        case 0x47:
          /* code */
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET); // 蓝灯开
          break;
        case 0x44:
          /* code */
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // 红灯关
          break;
        case 0x40:
          /* code */
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET); // 绿灯关
          break;
        case 0x43:
          /* code */
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET); // 蓝灯关
          break;

        default:
          break;
        }
      }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
