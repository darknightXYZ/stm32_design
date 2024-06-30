#include "stm32f10x.h"                 
#include "Timer.h"
#include "traffic_light.h" 
#include "Interrupt.h"


// 初始参数设置

struct tagTraffic tagNSDirect = {6, 12, 21, 3, 0, GreenRight, 0};
struct tagTraffic tagEWDirect = {6, 12, 21, 3, 0, RedRight, 0};
struct tagTraffic tagNSDirectCache = {6, 12, 21, 3, 0, GreenRight, 0};
struct tagTraffic tagEWDirectCache = {6, 12, 21, 3, 0, RedRight, 0};

extern uint16_t wEWCountDown; // 东西方向倒计时
extern uint16_t wNSCountDown; // 南北方向倒计时

uint8_t bDecoderOut = 0;      // 当前译码器低电平的输出端
uint8_t bTempStatus = 0;      // 临时状态
uint8_t bPauseFlag = 0;       // 暂停标志
uint16_t wGPIOBOutput = 0;    // GPIOB端口的输出
uint8_t bDirectChoice = EW_DIRECTION;  // 车流量的方向选择位，默认为东西方向


// 函数声明
void DisplayDigitValue(uint8_t * bDecoderOut, uint16_t w_EW, uint16_t w_NS);
void ModifyTrafficLightDelay(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int bDelta, uint8_t bPauseFlag);
void DisplayDefault(uint8_t * bDecoderOut);
void AutoControlVolume(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int bDelta);


// 主函数
int main(void)
{
	Timer_Init();
	Traffic_Init();
    Switch_Init();

	while (1)
	{
        
	}
}


/* 定时器中断处理 */

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
            case 0: // TIM2启动，显示正常倒计时值
                DisplayDigitValue(&bDecoderOut, wEWCountDown, wNSCountDown);
                break;
            case EW_RED_RIGHT_1: // 双位数码管中显示东西方向红灯右转设定值以及南北方向绿灯左转设定值
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wRedRightDelay, tagNSDirectCache.wGreenLeftDelay);
                break;
            case EW_RED_RIGHT_2:// 双位数码管中显示东西方向红灯右转设定值以及南北方向绿灯右转设定值
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wRedRightDelay, tagNSDirectCache.wGreenRightDelay);
                break;
            case EW_GREEN_LEFT:// 双位数码管中显示东西方向绿灯左转设定值以及南北方向红灯右转设定值
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wGreenLeftDelay, tagNSDirectCache.wRedRightDelay);
                break;
            case EW_GREEN_RIGHT:// 双位数码管中显示东西方向绿灯右转设定值以及南北方向红灯右转设定值
                DisplayDigitValue(&bDecoderOut, tagEWDirectCache.wGreenRightDelay, tagNSDirectCache.wRedRightDelay);
                break;
            case EW_EVER_GREEN: // 东西方向常绿状态（南北方向常红状态）
                DisplayDefault(&bDecoderOut);
                break;
            case EW_EVER_RED:   // 东西方向常绿状态（南北方向常红状态）
                DisplayDefault(&bDecoderOut);
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}


/* 外部中断处理 */

// 开始运行/重新运行
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) == SET)
    {
        bPauseFlag = 0; // 清除标志位
        if(wGPIOBOutput) // 当wGPIOBOutput不为0，表示按下过常绿/常红开关，因此需要恢复当前GPIOB口的输出
        {
            GPIOB->ODR = wGPIOBOutput;
        }
        TIM_Cmd(TIM2, ENABLE); // 开启交通灯定时变化
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

// 处理常红或常绿
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) == SET)
    {
        wGPIOBOutput = GPIOB->ODR; // 保存当前GPIOB口输出状态，便于恢复

        TIM_Cmd(TIM2, DISABLE);
        switch(bPauseFlag)
        {
            case 0: // 在系统正常运行时按下，东西方向常绿（允许通行），南北方向常红（不许通行）
                bPauseFlag = EW_EVER_GREEN;
                GPIOB->ODR = 0x0164;
                break;
            case EW_EVER_GREEN: // 第二次按下，南北方向常绿（允许通行），东西方向常红（不许通行）
                bPauseFlag = EW_EVER_RED;
                GPIOB->ODR = 0x0149;
                break;
            case EW_EVER_RED: // 东西方向常绿（允许通行），南北方向常红（不许通行）
                bPauseFlag = EW_EVER_GREEN;
                GPIOB->ODR = 0x0164;
                break;
            default:
                return;
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
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
                bPauseFlag = EW_RED_RIGHT_1;
                break;
            case EW_RED_RIGHT_1:
                bPauseFlag = EW_RED_RIGHT_2;
                break;            
            case EW_RED_RIGHT_2:
                bPauseFlag = EW_GREEN_LEFT;
                break;
            case EW_GREEN_LEFT:
                bPauseFlag = EW_GREEN_RIGHT;
                break;
            case EW_GREEN_RIGHT:
                bPauseFlag = EW_RED_RIGHT_1;
                break;
            default:
                return;
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
    
    // 选择方向
    if (EXTI_GetITStatus(EXTI_Line8) == SET)
    {
        if(bDirectChoice == EW_DIRECTION)
        {
            bDirectChoice = NS_DIRECTION;
        }
        else if(bDirectChoice == NS_DIRECTION)
        {
            bDirectChoice = EW_DIRECTION;
        }
        else
        {
            return;
        }
        EXTI_ClearITPendingBit(EXTI_Line8);        
    }    
}


// 显示八个数码管倒计时
void DisplayDigitValue(uint8_t * bDecoderOut, uint16_t w_EW, uint16_t w_NS)
{
    // GPIOC的0 1 2口循环输出000~111
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

// 显示默认值 --
void DisplayDefault(uint8_t * bDecoderOut)
{
    GPIO_WriteBit(GPIOC, GPIO_Pin_0, (*bDecoderOut & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_1, (*bDecoderOut & 0x02) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(GPIOC, GPIO_Pin_2, (*bDecoderOut & 0x04) ? Bit_SET : Bit_RESET);
    GPIOA->ODR = 0x40;   
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
        case EW_RED_RIGHT_1: // 东西方向右红，南北方向左绿
            pEWvalue = &pEWDirect->wRedRightDelay;
            pNSvalue = &pNSDirect->wGreenLeftDelay;
            break;
        case EW_RED_RIGHT_2: // 东西方向右红，南北方向右绿
            pEWvalue = &pEWDirect->wRedRightDelay;
            pNSvalue = &pNSDirect->wGreenRightDelay;
            break;        
        case EW_GREEN_LEFT:  // 东西方向绿左，南北方向右红
            pEWvalue = &pEWDirect->wGreenLeftDelay;
            pNSvalue = &pNSDirect->wRedRightDelay;
            break;
        case EW_GREEN_RIGHT: // 东西方向绿右，南北方向红灯
            pEWvalue = &pEWDirect->wGreenRightDelay;
            pNSvalue = &pNSDirect->wRedRightDelay;
            break;
        default:             // 在暂停键并未按下时，表示动态调整车流量大小
            AutoControlVolume(pEWDirect, pNSDirect, bDelta);
            return;
    }
    
    // 限制灯控时间范围
    if((*pEWvalue + bDelta <= 1) || (*pNSvalue + bDelta <= 1))
    {
        return;
    }else if((*pEWvalue + bDelta > 60) || (*pNSvalue + bDelta > 60))
    {
        return;
    }else
    {
        *pEWvalue += bDelta;
        *pNSvalue += bDelta;
    }
}


// 根据车流量调整交通信号灯时长
void AutoControlVolume(struct tagTraffic *pEWDirect, struct tagTraffic *pNSDirect, int bDelta)
{
    struct tagTraffic *pPrimary, *pSecondary;

    if (bDirectChoice == EW_DIRECTION) // 东西方向车流量
    {
        pPrimary = pEWDirect;
        pSecondary = pNSDirect;
    }
    else if (bDirectChoice == NS_DIRECTION) // 南北方向车流量
    {
        pPrimary = pNSDirect;
        pSecondary = pEWDirect;
    }
    else
    {
        return; // 无效的方向选择
    }

    // 计算并检查延迟
    int redDelay = pSecondary->wRedRightDelay + 2 * bDelta;
    int greenLeftDelay = pPrimary->wGreenLeftDelay + bDelta;
    int greenRightDelay = pPrimary->wGreenRightDelay + bDelta;


    if (redDelay <= 1 || redDelay > 60 || greenLeftDelay <= 1 || 
        greenLeftDelay > 60 || greenRightDelay <= 1 || greenRightDelay > 60)
    {
        return; // 不进行更新
    }

    // 更新延迟
    pSecondary->wRedRightDelay = redDelay;
    pPrimary->wGreenLeftDelay = greenLeftDelay;
    pPrimary->wGreenRightDelay = greenRightDelay;
}

