#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Servo.h"
#include "Key.h"
#include "filt.h"
#include "AD.h"
#include "mw.h"
#include "Timer.h"
#include "Serial.h"
#include "NRF24L01.h"
uint8_t KeyNum;
uint8_t fl1,fl2,fl3;
char i,letter,letter1;
int main(void)
{
	//Timer_Init();
	OLED_Init();
	AD_Init();
	//NRF24L01_Init();
	Serial_Init();
//	j=1;
//	b=2;
//	c=1;
	while (1)
 { 	
	 
	 Flag();//获取标志位
     //sennum();
	if (AD_Value[9] > 4000)     //译码盲文   
	 {
		   Delay_ms(400);

			 
			 
			 if (f0==1||f1==1||f2==1||f3==1||f4==1||f5==1)//如果存在盲文编码则执行编码字符输入
			 {
				       i=Read_BrailleKeys();//获取盲文点位
			         letter = Braille_To_Char(i);
	             letter1=letter ;
				       Serial_Printf(&letter1);
	             //NRF24L01_SendString(&letter1);   
			 }
			 else//无盲文位置激活则直接输出enter
			 {
				       Serial_Printf("0x0A");
				 
			 }
			 clear();//清除mangwen.c中的判断标志

		}

	 
	 if (AD_Value[6] > 4000)	//输入.功能
	{
		Delay_ms(400);

		    Serial_Printf(".");
		    clear();			
    }
	
	if (AD_Value[7] > 4000)	//输入空格功能
	{
		  Delay_ms(400);

			Serial_Printf(" ");
		clear();
	}
	 

	
	if (AD_Value[8] > 4000)	//删除功能
	{
		
		  Delay_ms(400);		

			Serial_Printf("0x08");//
		  clear();

		
   }

}
 
}
