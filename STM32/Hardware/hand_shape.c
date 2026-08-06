#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "Servo.h"
#include "Key.h"

uint8_t Key;
float Angle;

void shape1(void)
{
	            Servo_SetAnglePA6(180);
				Servo_SetAnglePA7(180);
		        Servo_SetAnglePB0(0);
				Servo_SetAnglePB1(0);
				Servo_SetAnglePB6(0);

}
	
void shape2(void)
{
	           Servo_SetAnglePA6(180);
				Servo_SetAnglePA7(180);
		        Servo_SetAnglePB0(180);
				Servo_SetAnglePB1(0);
				Servo_SetAnglePB6(0);

}

void shape3(void)
{
	           Servo_SetAnglePA6(180);
				Servo_SetAnglePA7(0);
		        Servo_SetAnglePB0(180);
				Servo_SetAnglePB1(180);
				Servo_SetAnglePB6(180);

}

void shape4(void)
{
	            Servo_SetAnglePA6(180);
				Servo_SetAnglePA7(180);
		        Servo_SetAnglePB0(180);
				Servo_SetAnglePB1(180);
				Servo_SetAnglePB6(180);

}

void shape5(void)
{
	           Servo_SetAnglePA6(0);
				Servo_SetAnglePA7(180);
		        Servo_SetAnglePB0(180);
				Servo_SetAnglePB1(180);
				Servo_SetAnglePB6(180);
}

void shape_fist(void)
{
	            Servo_SetAnglePA6(180);
				Servo_SetAnglePA7(0);
		        Servo_SetAnglePB0(0);
				Servo_SetAnglePB1(0);
				Servo_SetAnglePB6(0);

}

void handshape(void)
{
	Key = Key_GetNum();
		if (Key == 1)
		{
			Angle += 1;
			if(Angle==1)
			{
				shape1();
			
			}
			if(Angle==2)
			{
				shape2();
			
			}
			if(Angle==3)
			{
				shape3();
			
			}
			if(Angle==4)
			{
				shape4();
			
			}
			if(Angle==5)
			{
				shape5();
			
			}
			
			if (Angle > 5)
			{
				Angle = 0;
			}
			if (Angle == 0)
			{
				shape_fist();
			}
		}

}
