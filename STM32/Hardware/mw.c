#include "stm32f10x.h"
#include "AD.h"
#include "OLED.h"
#include "filt.h"
#include "Serial.h"
uint8_t f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,num;
uint32_t ADValue[10],ADValuelast[10];//数组的大小为6
#define  a1 45;

void Flag(void)
{
	for(num=0;num<10;num++)//读取ADC的值
	{
		//ADValue[num]= Getadcaverage(num);
		ADValue[num]= AD_Value[num];//没有滤波
	}
	  if ( ADValue[0] > 4000)    f0=1;  
    if ( ADValue[1] > 4000)    f1=1; 
	if ( ADValue[2] > 4000)    f2=1; 
	if ( ADValue[3] > 4000)    f3=1;
	if ( ADValue[4] > 4000)    f4=1;
	if ( ADValue[5] > 4000)    f5=1;
	
	
	
	OLED_ShowNum(1, 7, f0, 1);//显示出标志位
	OLED_ShowNum(2, 7, f1, 1);
	OLED_ShowNum(3, 7, f2, 1);
	OLED_ShowNum(1, 10, f3,1);
	OLED_ShowNum(2, 10, f4,1);
	OLED_ShowNum(3, 10, f5,1);

	
}

void sennum(void)
{
	for(num=0;num<10;num++)
	{
		Serial_SendNumber(ADValue[num], 4);
		if (num<9)    
			Serial_Printf(",");
	}
	 Serial_Printf("\r\n");
}
void clear(void)
{
	f0=0;
	f1=0;
	f2=0;
	f3=0;
	f4=0;
	f5=0;
	f5=0;

}

uint8_t Read_BrailleKeys(void)   //读取每个按键状态，按位或
{
    uint8_t braille = 0;

    if (f0==1)  braille |= 0x01; // 点1     //* *//     六点盲文系统示意图
    if (f1==1)  braille |= 0x02; // 点2     //* *//
    if (f2==1)  braille |= 0x04; // 点3     //* *//
    if (f3==1)  braille |= 0x08; // 点4     
    if (f4==1)  braille |= 0x10; // 点5
    if (f5==1)  braille |= 0x20; // 点6

    return braille;
}
	char Braille_To_Char(uint8_t braillechar) //译码函数
{
    // 按盲文点位的组合返回字母
    switch(braillechar)
	{
//        case 0x01: return 'A'; case 0x03: return 'B'; case 0x09: return 'C';
//        case 0x19: return 'D'; case 0x11: return 'E'; case 0x0B: return 'F';
//        case 0x1B: return 'G'; case 0x13: return 'H'; case 0x0A: return 'I';
//        case 0x1A: return 'J'; case 0x05: return 'K'; case 0x07: return 'L';
//        case 0x0D: return 'M'; case 0x1D: return 'N'; case 0x15: return 'O';
//        case 0x0F: return 'P'; case 0x1F: return 'Q'; case 0x17: return 'R';
//        case 0x0E: return 'S'; case 0x1E: return 'T'; case 0x25: return 'U';
//		case 0x27: return 'V'; case 0x3A: return 'W'; case 0x2D: return 'X';
//		case 0x3D: return 'Y'; case 0x35: return 'Z';
		case 0x01: return 'a'; case 0x03: return 'b'; case 0x09: return 'c';
        case 0x19: return 'd'; case 0x11: return 'e'; case 0x0B: return 'f';
        case 0x1B: return 'g'; case 0x13: return 'h'; case 0x0A: return 'i';
        case 0x1A: return 'j'; case 0x05: return 'k'; case 0x07: return 'l';
        case 0x0D: return 'm'; case 0x1D: return 'n'; case 0x15: return 'o';
        case 0x0F: return 'p'; case 0x1F: return 'q'; case 0x17: return 'r';
        case 0x0E: return 's'; case 0x1E: return 't'; case 0x25: return 'u';
		case 0x27: return 'v'; case 0x3A: return 'w'; case 0x2D: return 'x';
		case 0x3D: return 'y'; case 0x35: return 'z';
        default: return '?';
    }
}
