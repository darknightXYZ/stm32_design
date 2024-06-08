#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Timer.h"
#include "traffic_light.h" 
#include "Interrupt.h"


// 初始参数设置
struct tagTraffic tagNSDirect = {10, 12, 16, 8, 3, 0, GreenLeft, 0};
struct tagTraffic tagEWDirect = {8, 13, 15, 10, 3, 0, RedRight, 0};
//struct tagTraffic tagNSDirectCache = {5, 6, 11, 3, 3, 0, GreenLeft, 0};
//struct tagTraffic tagEWDirectCache = {3, 8, 9, 5, 3, 0, RedRight, 0};
struct tagTraffic tagNSDirectCache = {10, 12, 16, 8, 3, 0, GreenLeft, 0};
struct tagTraffic tagEWDirectCache = {8, 13, 15, 10, 3, 0, RedRight, 0};

extern uint16_t wEWCountDown; // 东西方向倒计时
extern uint16_t wNSCountDown; // 南北方向倒计时

uint8_t bDecoderOut = 0; // 当前译码器低电平的输出端
uint8_t bTempStatus = 0; // 临时状态
uint8_t bPauseFlag = 0; // 暂停标志


void DisplayDigitValue(uint8_t * bDecoderOut, uint16_t w_EW, uint16_t w_NS);
void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int bDelta, uint8_t bPauseFlag);

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
        Traffic_Run(&tagEWDirect, &tagNSDirect, &tagEWDirectCache, &tagNSDirectCache);
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志位
	}
}

// 定时器3中断,刷新数码管的显示
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) 
    {
        switch(bPauseFlag)
        {
            case 0: // TIM2启动
                DisplayDigitValue(&bDecoderOut, wEWCountDown, wNSCountDown);
                break;
            case EW_RED_LEFT:
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wRedLeftDelay, tagNSDirectCache.wGreenRightDelay);
                break;
            case EW_RED_RIGHT:
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wRedRightDelay, tagNSDirectCache.wGreenLeftDelay);
                break;
            case EW_GREEN_LEFT:
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wGreenLeftDelay, tagNSDirectCache.wRedRightDelay);
                break;
            case EW_GREEN_RIGHT:
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wGreenRightDelay, tagNSDirectCache.wRedLeftDelay);
                break;
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
        bPauseFlag = 0;
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
        switch(bPauseFlag)
        {
            case 0:
                bPauseFlag = EW_RED_LEFT;
                break;
            case EW_RED_LEFT:
                bPauseFlag = EW_RED_RIGHT;
                break;
            case EW_RED_RIGHT:
                bPauseFlag = EW_GREEN_LEFT;
                break;
            case EW_GREEN_LEFT:
                bPauseFlag = EW_GREEN_RIGHT;
                break;
            case EW_GREEN_RIGHT:
                bPauseFlag = EW_RED_LEFT;
                break;
        }

        TIM_Cmd(TIM2, DISABLE); // 关闭交通灯定时变化，数码管显示暂停

        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    
    // 增加
    if (EXTI_GetITStatus(EXTI_Line6) == SET)
    {
        // 增加时长，修改副本
        ModifyTrafficLightDelay(&tagEWDirectCache, &tagNSDirectCache, 1, bPauseFlag);
        bTempStatus = 1;
        EXTI_ClearITPendingBit(EXTI_Line6);
    }  
    
    // 减少
    if (EXTI_GetITStatus(EXTI_Line7) == SET)
    {
        // 减少时长，修改副本
        ModifyTrafficLightDelay(&tagEWDirectCache, &tagNSDirectCache, -1, bPauseFlag);
        bTempStatus = 1;
        EXTI_ClearITPendingBit(EXTI_Line7);        
    }    
}



// 显示八个数码管倒计时
void DisplayDigitValue(uint8_t * bDecoderOut, uint16_t w_EW, uint16_t w_NS)
{
    uint8_t bDigitValue = 0;
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, (*bDecoderOut & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, (*bDecoderOut & 0x02) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, (*bDecoderOut & 0x04) ? Bit_SET : Bit_RESET);

    bDigitValue = GetDigitValue(*bDecoderOut, w_EW, w_NS);

    Digit_Refresh_Show(bDigitValue); // 显示
    
    *bDecoderOut = *bDecoderOut + 1;
    if (*bDecoderOut == 8) {
        *bDecoderOut = 0;
    }
}


// 修改交通灯时长
void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int bDelta, uint8_t bPauseFlag)
{   
    uint16_t * pEWvalue;
    uint16_t * pNSvalue;
    
    switch(bPauseFlag)
    {
        case EW_RED_LEFT: // 东西方向左红，南北方向右绿
            pEWvalue = &pEWDirect->wRedLeftDelay;
            pNSvalue = &pNSDirect->wGreenRightDelay;
            break;
        case EW_RED_RIGHT: // 东西方向右红，南北方向左绿
            pEWvalue = &pEWDirect->wRedRightDelay;
            pNSvalue = &pNSDirect->wGreenLeftDelay;
            break;
        case EW_GREEN_LEFT:
            pEWvalue = &pEWDirect->wGreenLeftDelay;
            pNSvalue = &pNSDirect->wRedRightDelay;
            break;
        case EW_GREEN_RIGHT:
            pEWvalue = &pEWDirect->wGreenRightDelay;
            pNSvalue = &pNSDirect->wRedLeftDelay;
            break;
        default:
            return;
    }
    
    // 限制灯控时间范围
    if(*pEWvalue + bDelta < GetMinimum(bPauseFlag))
    {
        *pEWvalue = GetMinimum(bPauseFlag);
    }else if(*pEWvalue + bDelta > GetMaximum(bPauseFlag))
    {
        *pEWvalue = GetMaximum(bPauseFlag);
    }else
    {
        *pEWvalue += bDelta;
        *pNSvalue += bDelta;
    }
    
    
//    if (pRedDirect->w_red_delay + red_delay_delta < MIN_RED_DELAY) {
//        pRedDirect->w_red_delay = MIN_RED_DELAY;
//    } else if (pRedDirect->w_red_delay + red_delay_delta > MAX_RED_DELAY) {
//        pRedDirect->w_red_delay = MAX_RED_DELAY;
//    } else {
//        pRedDirect->w_red_delay += red_delay_delta;
//    }
//    
//    if (pGreenDirect->w_green_delay + green_delay_delta < MIN_GREEN_DELAY) {
//        pGreenDirect->w_green_delay = MIN_GREEN_DELAY;
//    } else if (pGreenDirect->w_green_delay + green_delay_delta > MAX_GREEN_DELAY) {
//        pGreenDirect->w_green_delay = MAX_GREEN_DELAY;
//    } else {
//        pGreenDirect->w_green_delay += green_delay_delta;
//    }
    
}
