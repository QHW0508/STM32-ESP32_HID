#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//定时器4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// PA6 PA7配置成推挽输出
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PB0 PB1 PB6配置成推挽输出
	GPIO_InitTypeDef GPIO_InitStructureB;
	GPIO_InitStructureB.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructureB.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6;
	GPIO_InitStructureB.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructureB);
	
	TIM_InternalClockConfig(TIM3);
	TIM_InternalClockConfig(TIM4);
	//定时器2配置
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 20000 - 1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	//结构体配置到定时器3
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);
	//定时器4的配置
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	//PWM配置
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;		//CCR
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);//PB6 定时器4的通道1
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);//PA6
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);//PA7
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);//PB0
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);// PB1
	TIM_Cmd(TIM3, ENABLE);//使能定时器3
	TIM_Cmd(TIM4, ENABLE);//使能定时器4
}

void PWM_SetCompare2(uint16_t Compare)//定时器3的通道 PA6
{
	TIM_SetCompare1(TIM3, Compare);
}

void PWM_SetCompare3(uint16_t Compare)//定时器3的通道2 PA7
{
	TIM_SetCompare2(TIM3, Compare);
}

void PWM_SetCompare4(uint16_t Compare)//定时器3的通道3 PBO
{
	TIM_SetCompare3(TIM3, Compare);
}

void PWM_SetCompare5(uint16_t Compare)//定时器3的通道4 PB1
{
	TIM_SetCompare4(TIM3, Compare);
}

void PWM_SetCompare6(uint16_t Compare)//定时器4的通道1 PB6
{
	TIM_SetCompare1(TIM4, Compare);
}
