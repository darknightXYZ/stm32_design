#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Timer.h"
#include "traffic_light.h" 
#include "Interrupt.h"


// 初始参数设置
struct tagTraffic tagEWDirect = {8, 11, 3, 0, 0, 0};
struct tagTraffic tagNSDirect = {8, 11, 3, 0, 1, 0};

struct tagTraffic tagEWDirect_cache = {8, 11, 3, 0, 0, 0};
struct tagTraffic tagNSDirect_cache = {8, 11, 3, 0, 1, 0};

extern uint16_t w_EW_CountDown; // 东西方向倒计时
extern uint16_t w_NS_CountDown; // 南北方向倒计时

uint8_t b_decoder_out = 0; // 当前译码器低电平的输出端
uint8_t temp_status = 0; // 临时状态
uint8_t b_pause_flag = 0; // 暂停标志


void DisplayDigitValue(uint8_t * b_decoder_out, uint16_t w_EW, uint16_t w_NS);
void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int red_delay_delta, int green_delay_delta, uint8_t b_Direction);

int main(void)
{
	Timer_Init();
	Traffic_Init();
    switch_Init();

	while (1)
	{
        
	}
}


// 定时器2中断,控制交通灯的变化
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) // 如果更新中断标志位为1
	{
        Traffic_Run(&tagEWDirect, &tagNSDirect, &tagEWDirect_cache, &tagNSDirect_cache);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志位
	}
}

// 定时器3中断,刷新数码管的显示
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) 
    {
        if(!b_pause_flag) // TIM2重新运行后
        {
            DisplayDigitValue(&b_decoder_out, w_EW_CountDown, w_NS_CountDown);
        }else if(b_pause_flag == 1) // 展示东西方向红灯值(南北方向绿灯值)
        {
            DisplayDigitValue(&b_decoder_out, tagEWDirect_cache.w_red_delay, tagNSDirect_cache.w_green_delay);
        }else if(b_pause_flag == 2) // 展示东西方向绿灯值(南北方向红灯值)
        {
            DisplayDigitValue(&b_decoder_out, tagEWDirect_cache.w_green_delay, tagNSDirect_cache.w_red_delay);
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

// 外部中断

// 开始运行/重新运行
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) == SET)
    {
        b_pause_flag = 0;
        TIM_Cmd(TIM2, ENABLE); // 开启交通灯定时变化
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
    
}


void EXTI9_5_IRQHandler(void)
{
    // 暂停
    if (EXTI_GetITStatus(EXTI_Line5) == SET)
    {
        // 切换暂停状态
        if (b_pause_flag == 1)
        {
            b_pause_flag = 2;
        }
        else
        {
            b_pause_flag = 1;
        }

        TIM_Cmd(TIM2, DISABLE); // 关闭交通灯定时变化，数码管显示暂停

        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    
    // 增加
    if (EXTI_GetITStatus(EXTI_Line6) == SET)
    {
        // 增加时长，修改副本
        ModifyTrafficLightDelay(&tagEWDirect_cache, &tagNSDirect_cache, 1, 1, b_pause_flag);
        temp_status = 1;
        EXTI_ClearITPendingBit(EXTI_Line6);
    }  
    
    // 减少
    if (EXTI_GetITStatus(EXTI_Line7) == SET)
    {
        // 减少时长，修改副本
        ModifyTrafficLightDelay(&tagEWDirect_cache, &tagNSDirect_cache, -1, -1, b_pause_flag);
        temp_status = 1;
        EXTI_ClearITPendingBit(EXTI_Line7);        
    }    
}



// 显示八个数码管倒计时
void DisplayDigitValue(uint8_t *b_decoder_out, uint16_t w_EW, uint16_t w_NS)
{
    uint8_t b_value = 0;
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, (*b_decoder_out & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, (*b_decoder_out & 0x02) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, (*b_decoder_out & 0x04) ? Bit_SET : Bit_RESET);

    b_value = GetDigitValue(*b_decoder_out, w_EW, w_NS);

    Digit_Refresh_Show(b_value); // 显示
    
    *b_decoder_out = *b_decoder_out + 1;
    if (*b_decoder_out == 8) {
        *b_decoder_out = 0;
    }
}


void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int red_delay_delta, int green_delay_delta, uint8_t b_pause_flag)
{
    struct tagTraffic *pRedDirect, *pGreenDirect;
    
    // 根据标志选择红灯和绿灯方向
    if (b_pause_flag == 1) {
        pRedDirect = pEWDirect;
        pGreenDirect = pNSDirect;
    } else if (b_pause_flag == 2) {
        pRedDirect = pNSDirect;
        pGreenDirect = pEWDirect;
    } else {
        return; // 非法标志，直接返回
    }
    
    // 限制灯控时间范围
    if (pRedDirect->w_red_delay + red_delay_delta < MIN_RED_DELAY) {
        pRedDirect->w_red_delay = MIN_RED_DELAY;
    } else if (pRedDirect->w_red_delay + red_delay_delta > MAX_RED_DELAY) {
        pRedDirect->w_red_delay = MAX_RED_DELAY;
    } else {
        pRedDirect->w_red_delay += red_delay_delta;
    }
    
    if (pGreenDirect->w_green_delay + green_delay_delta < MIN_GREEN_DELAY) {
        pGreenDirect->w_green_delay = MIN_GREEN_DELAY;
    } else if (pGreenDirect->w_green_delay + green_delay_delta > MAX_GREEN_DELAY) {
        pGreenDirect->w_green_delay = MAX_GREEN_DELAY;
    } else {
        pGreenDirect->w_green_delay += green_delay_delta;
    }
}
