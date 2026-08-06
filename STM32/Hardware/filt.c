#include "stm32f10x.h"
#include "AD.h"

#define N 20
#define BAN 5

uint32_t  Getadcaverage(uint8_t c)
{
	uint32_t   ADCvalueBuf[N]={0};
    uint16_t   count=0,i=0,j=0,ADCvaluetemp=0;
	uint32_t   sum=0;
	for(count=0;count<N;count++)
	  ADCvalueBuf[count]=AD_Value[c];
      for(j=0;j<N-1;j++)    //冒泡法排序，大的四个值和小的五个值去掉，其余的求平均值
	{
		for(i=0;i<N-j;i++)
		{
			if(ADCvalueBuf[i]>ADCvalueBuf[i+1])
			{
				ADCvaluetemp=ADCvalueBuf[i];
			    ADCvalueBuf[i]=ADCvalueBuf[i+1];
				ADCvalueBuf[i+1]=ADCvaluetemp;
			
			
			}	
		}
	}

	for(count=BAN;count<N-BAN;count++)
	sum+=ADCvalueBuf[count];
	return((uint32_t)((float)sum/(N-BAN*2)));

}
