#include "stm32f10x.h"                  // Device header
#include "Delay.h"



//按键是PB0 +  和   PB11 -
uint8_t Key_Num;

void Key_Init(void)//按键初始化，上拉输入
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)//定时器调用的函数中获取了按键状态
{
	uint8_t Temp;
	if (Key_Num)
	{
		Temp = Key_Num;
		Key_Num =   0;//清0按键判断标志位
		return Temp;
	}
	return 0;
}

uint8_t Key_GetState(void)//获取按键状态
{
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
	{
		return 1;
	}
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0)
	{
		return 2;
	}
	return 0;
}

void Key_Tick(void)//定时器中断函数调用该函数，，1ms进一次中断
{
	static uint8_t Count;//静态变量，他的值不会在每次退出该函数时被销毁
	static uint8_t CurrState, PrevState;//默认值是0
	
	Count ++;
	if (Count >= 20)
	{
		Count = 0;
		
		PrevState = CurrState;
		CurrState = Key_GetState();//         返回值：没按下0   按键1按下为1  按键11按下为2
		
		if (CurrState == 0 && PrevState != 0)
		{
			Key_Num = PrevState;//按键松手时，kenum的值即为哪个按键按下的值
		}
	}
}	
