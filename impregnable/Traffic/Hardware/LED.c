#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9);
}

void LED_Loop(uint16_t *p_Count, uint16_t *p_reverse)
{
    uint16_t ustate = 0x03ff;
    if(*p_reverse)
    {
        *p_Count = *p_Count - 1;
    }
    else
    {
        *p_Count = *p_Count + 1;
    }
    if(*p_Count == 10)
    {
        *p_reverse = 1;
    }
    if(*p_Count == 0)
    {
        *p_reverse = 0;
    }
    ustate = ustate << *p_Count;
    GPIO_Write(GPIOB, ustate);
}

void LED_shift(uint16_t num)
{
	uint16_t x = 0x03FF;
	x = x << num;
	GPIO_Write(GPIOB, x);
}

uint16_t State_shift(uint16_t num,uint16_t flag)
{
	if(flag==1)
		num++;
	if(flag==0)
		num--;
	return num;
}

uint16_t State(uint16_t num,uint16_t flag)
{
	if(num==10)
		flag=0;
	if(num ==0)
		flag=1;
	return flag;
}
