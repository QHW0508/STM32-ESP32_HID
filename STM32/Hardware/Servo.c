#include "stm32f10x.h"                  // Device header
#include "PWM.h"


void Servo_Init(void)
{
	PWM_Init();
}

void Servo_SetAnglePA6(float Angle)
{
	PWM_SetCompare2(Angle / 180 * 2000 + 500);
}

void Servo_SetAnglePA7(float Angle)
{
	PWM_SetCompare3(Angle / 180 * 2000 + 500);
}

void Servo_SetAnglePB0(float Angle)
{
	PWM_SetCompare4(Angle / 180 * 2000 + 500);
}
void Servo_SetAnglePB1(float Angle)
{
	PWM_SetCompare5(Angle / 180 * 2000 + 500);
}
void Servo_SetAnglePB6(float Angle)
{
	PWM_SetCompare6(Angle / 180 * 2000 + 500);
}


void Servo_SetAngle(uint8_t a)
{
	
	
		   if(a==0)
			{
				Servo_SetAnglePA6(180);
				
			
			}
			if(a==1)
			{
				
				Servo_SetAnglePA7(180);
		        
			
			}
			if(a==2)
			{
				
		        Servo_SetAnglePB0(180);
				
			
			}
			if(a==3)
			{
				
				Servo_SetAnglePB1(180);
				
			
			}
			if(a==4)
			{
				
				Servo_SetAnglePB6(180);
			
			}
			
			
		
		
	
	
	




}	


