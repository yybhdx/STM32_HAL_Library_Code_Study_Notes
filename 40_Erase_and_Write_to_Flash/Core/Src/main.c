/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/*
  * @brief  FLASH读取一个32位的字(字就是32位)
  * @param  Address: 要读取的地址
  * @retval 指定地址下的数据

*/
uint32_t MyFLASH_ReadWord(uint32_t Address)
{

  /*
  将Address(无符号整数)转换为指针类型
  指定该指针为volatile uint32_t*(即指向32位无符号整数的易变指针)
  通过*运算符解引用该指针，获取该地址下的4字节数据，并返回
  __IO就是volatile的别名，表示该地址的数据可能会被硬件或其他程序修改，所以编译器在优化时会考虑到这一点，不会对该地址的数据进行缓存或优化，以确保每次访问都能获取到最新的数据
  */
  return *(__IO uint32_t *)Address; // 将地址转换为指针，并通过解引用获取该地址下的数据
}

/*
 * @brief  FLASH读取一个16位的半字(半字就是16位)
 * @param  Address: 要读取的地址
 * @retval 指定地址下的数据
 */
uint16_t MyFLASH_ReadHalfWord(uint32_t Address)
{
  return *(__IO uint16_t *)Address; // 将地址转换为指针，并通过解引用获取该地址下的数据
}

/*
 * @brief  FLASH读取一个8位的字节(字节就是8位)
 * @param  Address: 要读取的地址
 * @retval 指定地址下的数据
 */
uint8_t MyFLASH_ReadByte(uint32_t Address)
{
  return *(__IO uint8_t *)Address; // 将地址转换为指针，并通过解引用获取该地址下的数据
}

/*
 * @brief  FLASH擦除一个页
 * @param  PageAddress: 要擦除的页的起始地址(必须是页的起始地址)
 * @retval 操作状态，HAL_OK表示成功，其他值表示失败
 */
HAL_StatusTypeDef FlashPageErase(uint32_t PageAddress)
{
  /*定义一个变量来存储函数的返回状态，初始值为HAL_OK，表示操作成功*/
  HAL_StatusTypeDef status = HAL_OK;

  /*定义一个FLASH_EraseInitTypeDef结构体变量，用于配置擦除操作的参数*/
  FLASH_EraseInitTypeDef eraseInit;

  /*设置擦除操作的类型为页擦除*/
  uint32_t sectorError = 0;

  // 解锁FLASH
  HAL_FLASH_Unlock();

  /*配置擦除参数*/
  eraseInit.TypeErase = FLASH_TYPEERASE_PAGES; // 设置擦除类型为页擦除(有些芯片支持 Mass Erase 全擦)
  eraseInit.PageAddress = PageAddress;         // 设置要擦除的页的起始地址 (必须是页的起始地址)
  eraseInit.NbPages = 1;                       // 设置要擦除的页数为1

  /*
  &eraseInit:指向一个配置结构体，决定了你要擦哪里、擦多少
  &sectorError:指向一个变量，用于存储擦除过程中发生的错误页地址，如果擦除成功，该变量的值将被设置为0，如果擦除失败，该变量会记录下出错的那一页地址。
  */
  status = HAL_FLASHEx_Erase(&eraseInit, &sectorError);

  /*锁定FLASH*/
  HAL_FLASH_Lock();

  /*返回操作状态*/
  if (status != HAL_OK)
  {
    // 错误处理
    return status;
  }

  return HAL_OK;
}

/*
 * @brief  FLASH写入数据
 * @param  StartAddress: 要写入的起始地址(必须是页的起始地址)
 * @param  pData: 指向要写入的数据的指针
 * @param  DataSize: 要写入的数据的大小，单位为字节
 * @retval 操作状态，HAL_OK表示成功，其他值表示失败
 */
HAL_StatusTypeDef FlashWrite(uint32_t StartAddress, uint8_t *pData, uint32_t DataSize)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t address = StartAddress; // 当前写入的地址，初始值为起始地址
  uint32_t dataIndex;              // 数据索引，用于遍历要写入的数据
  uint32_t wordData = 0;           // 用于32位写数据的数据缓存

  /*解锁Flash控制器*/
  status = HAL_FLASH_Unlock();
  if (status != HAL_OK)
  {
    return status; // 解锁失败，返回错误状态
  }

  /*循环写入数据(按32位操作)*/
  while (dataIndex < DataSize)
  {
    /*构建32位数据*/
    wordData = 0; // 清空数据缓存

    // 将要写入的数据按字节拼接成一个32位的整数，注意处理最后不足4字节的情况
    for (uint32_t i = 0; i < 4 && (dataIndex + i) < DataSize; i++)
    {
      wordData |= ((uint32_t)pData[dataIndex + i]) << (i * 8); // 将8位数据拼接成32位数据
    }

    /*执行32位写入*/
    // FLASH_TYPEPROGRAM_WORD表示按32位(4字节)写入数据，address是要写入的具体物理地址，wordData是要写入的数据
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, wordData);
    if (status != HAL_OK)
    {
      break;
    }

    /*更新地址和索引*/
    address += 4;   // 每次写入4字节
    dataIndex += 4; // 更新数据索引
  }

  /*锁定Flash控制器*/
  HAL_FLASH_Lock();

  return status; // 返回操作状态
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
  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();

  // OLED_ShowHexNum(1, 1, MyFLASH_ReadWord(0x08000000), 8);     // 读取地址0x08000000下的32位数据，并以16进制形式显示在OLED上，显示长度为8个字符
  // OLED_ShowHexNum(3, 1, MyFLASH_ReadHalfWord(0x08000000), 4); // 读取地址0x08000000下的16位数据，并以16进制形式显示在OLED上，显示长度为4个字符
  // OLED_ShowHexNum(5, 1, MyFLASH_ReadByte(0x08000000), 2);     // 读取地址0x08000000下的8位数据，并以16进制形式显示在OLED上，显示长度为2个字符

  // FlashPageErase(0X08000400); // 擦除地址0x08004000所在的页
  // FlashPageErase(0X08000000); // 擦除地址0x08000000所在的页
  FlashPageErase(0X08000800); // 擦除地址0x08008000所在的页

  /*创建一个四字节的数组，也就是一个字*/
  uint8_t pData[] = {0x12, 0x34, 0x56, 0x78};   // 要写入的数据，可以根据需要修改
  FlashWrite(0x08001000, pData, sizeof(pData)); // 将数据写入地址0x08001000
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
