#include "gpio.h"
#include "led.h"

/*多彩LED状态机，三个灯，一共8种不同的状态 */
uint16_t Led_Stage = 0;

void Led_Color(void)
{
	//蓝色
	if(Led_Stage==0|Led_Stage ==2|Led_Stage ==4|Led_Stage ==6)
	{
		// PA0设置为高电平 蓝色亮
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	}
	
	// 绿色
	if(Led_Stage==1|Led_Stage ==2|Led_Stage ==5|Led_Stage ==6)
	{
		// PA1设置为高电平 绿色亮
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	}
	
	// 红色
	if(Led_Stage>2)
	{
		// PA2设置为高电平 红色亮
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
	}
	
	// 延时500ms
	HAL_Delay(500);
	
	Led_Stage++;
	
	if(Led_Stage > 6)
	{
		Led_Stage = 0;
	}
}


