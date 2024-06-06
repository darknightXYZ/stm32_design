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

extern uint16_t w_digital_table[];

uint8_t b_decoder_out = 0; // 当前译码器低电平的输出端

//uint16_t add_count = 0;
//uint16_t dec_count = 0;

uint8_t temp_status = 0; // 临时状态

uint8_t b_pause_flag = 0;


void DisplayDigitValue(void);
void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int red_delay_delta, int green_delay_delta);

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
        DisplayDigitValue();
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
        // 处于暂停状态
        b_pause_flag = 1;
        TIM_Cmd(TIM2, DISABLE); // 关闭交通灯定时变化，数码管显示暂停
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    
    // 增加
    if (EXTI_GetITStatus(EXTI_Line6) == SET)
    {
        // 增加时长，修改副本
        ModifyTrafficLightDelay(&tagEWDirect_cache, &tagNSDirect_cache, 1, 1);
        temp_status = 1;
        EXTI_ClearITPendingBit(EXTI_Line6);
    }  
    
    // 减少
    if (EXTI_GetITStatus(EXTI_Line7) == SET)
    {
        // 减少时长，修改副本
        ModifyTrafficLightDelay(&tagEWDirect_cache, &tagNSDirect_cache, -1, -1);
        temp_status = 1;
        EXTI_ClearITPendingBit(EXTI_Line7);        
    }    
}



// 显示八个数码管倒计时
void DisplayDigitValue(void)
{
    uint8_t b_value = 0;
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, (b_decoder_out & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, (b_decoder_out & 0x02) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, (b_decoder_out & 0x04) ? Bit_SET : Bit_RESET);
    
    if (!b_pause_flag) {
        // TIM2重新运行后
        b_value = GetDigitValue(b_decoder_out, w_EW_CountDown, w_NS_CountDown);
    } else {
        // 获取副本值
        b_value = GetDigitValue(b_decoder_out, tagEWDirect_cache.w_red_delay, tagNSDirect_cache.w_green_delay);
    }
    
    Digit_Refresh_Show(b_value); // 显示
    
    b_decoder_out++;
    if (b_decoder_out == 8) {
        b_decoder_out = 0;
    }
}

// 修改交通灯延时
void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int red_delay_delta, int green_delay_delta)
{
    // 限制灯控时间范围
    if (pEWDirect->w_red_delay + red_delay_delta < MIN_RED_DELAY) {
        pEWDirect->w_red_delay = MIN_RED_DELAY;
        return;
    } else if (pEWDirect->w_red_delay + green_delay_delta > MAX_RED_DELAY) {
        pEWDirect->w_red_delay = MAX_RED_DELAY;
        return;
    }
    
    if (pNSDirect->w_green_delay + green_delay_delta < MIN_GREEN_DELAY) {
        pNSDirect->w_green_delay = MIN_GREEN_DELAY;
        return;
    } else if (pNSDirect->w_green_delay + red_delay_delta > MAX_GREEN_DELAY) {
        pNSDirect->w_green_delay = MAX_GREEN_DELAY;
        return;
    }
    
    // 修改东西方向红灯时长/南北方向绿灯时长
    pEWDirect->w_red_delay += red_delay_delta;
    pNSDirect->w_green_delay += green_delay_delta;

}

