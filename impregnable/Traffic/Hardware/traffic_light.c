#include "stm32f10x.h"                  // Device header
#include "traffic_light.h"

//###################################

// 双位数码管0~9对照表
const uint16_t w_digital_table[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f 
};

uint16_t wEWCountDown = 0; // 东西方向倒计时
uint16_t wNSCountDown = 0; // 南北方向倒计时
uint16_t wEWPressCount = 0; // 东西方向点按频次
uint16_t wNSPressCount = 0; // 南北方向点按频次

extern uint8_t bTempStatus;

//###################################

void Traffic_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 开启GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
    // 信号灯初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_6); // 测试使用 012红黄绿 345绿黄红
    
    // 数码管初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 未点击开始按钮时数码管显示 - 
    GPIO_SetBits(GPIOA, GPIO_Pin_6);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
//    GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2);
}


uint16_t GetMinimum(uint16_t flag) {
    switch(flag) {
        case EW_RED_LEFT:
            return MIN_RED_LEFT_DALAY;
        case EW_RED_RIGHT:
            return MIN_RED_RIGHT_DELAY;
        case EW_GREEN_LEFT:
            return MIN_GREEN_LEFT_DELAY;
        case EW_GREEN_RIGHT:
            return MIN_GREEN_RIGHT_DELAY;
        default:
            return 0;
    }
}


uint16_t GetMaximum(uint16_t flag) {
    switch(flag) {
        case EW_RED_LEFT:
            return MAX_RED_LEFT_DALAY;
        case EW_RED_RIGHT:
            return MAX_RED_RIGHT_DELAY;
        case EW_GREEN_LEFT:
            return MAX_GREEN_LEFT_DELAY;
        case EW_GREEN_RIGHT:
            return MAX_GREEN_RIGHT_DELAY;
        default:
            return 0;
    }
}


// 交通灯定时运行
void Traffic_Run(struct tagTraffic * tagEW, struct tagTraffic * tagNS, struct tagTraffic * tagEWCache, struct tagTraffic * tagNSCache)
{
    wEWCountDown = Light_Show(tagEW, EW_DIRECTION, tagEWCache);
    wNSCountDown = Light_Show(tagNS, NS_DIRECTION, tagNSCache);
    
    if(bTempStatus && (wEWCountDown==0 || wNSCountDown==0))
    {
        *tagEW = *tagEWCache;
        *tagNS = *tagNSCache;
        bTempStatus = 0;
    }
}

// 获取数码管特定位的显示数字
uint8_t GetDigitValue(uint8_t bDecoderOut, uint16_t w_EW, uint16_t w_NS)
{
    uint8_t b_temp = 0;
    switch(bDecoderOut)
    {
        case 0:
            b_temp = w_EW % 10;
            break;
        case 1:
            b_temp = w_EW / 10;
            break;
        case 2:
            b_temp = w_EW % 10;
            break;
        case 3:
            b_temp = w_EW / 10;
            break;
        case 4:
            b_temp = w_NS % 10;
            break;
        case 5:
            b_temp = w_NS / 10;
            break;
        case 6:
            b_temp = w_NS % 10;
            break;
        case 7:
            b_temp = w_NS / 10;
            break;        
    }
    return b_temp;
}

// 刷新数码管显示
void Digit_Refresh_Show(uint8_t b_value)
{
    GPIOA->ODR = w_digital_table[b_value];
}

// 改变左右方向箭头
void ChangeLED(void)
{
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_6) == Bit_SET)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_6); 
    }else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_6); 
    }
    
    if(GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_7) == Bit_SET)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_7); 
    }else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_7); 
    }        
}


// 根据方向和当前状态更新交通灯
void ChangeDigit(uint8_t bDirection, enum eType eState)
{
    if(bDirection == EW_DIRECTION) // 东西方向
    {
        switch(eState)
        {
            case RedLeft:
                GPIO_ResetBits(GPIOB, GPIO_Pin_0);
                GPIO_SetBits(GPIOB, GPIO_Pin_2);
                break;
            case GreenRight:
                GPIO_ResetBits(GPIOB, GPIO_Pin_2);
                GPIO_SetBits(GPIOB, GPIO_Pin_1); 
                break;
            case Yellow:
                GPIO_ResetBits(GPIOB, GPIO_Pin_1);
                GPIO_SetBits(GPIOB, GPIO_Pin_0);
                break;  
            default:
                return;
        }
    }
    if(bDirection == NS_DIRECTION) // 南北方向
    {
        switch(eState)
        {
            case RedLeft:
                GPIO_ResetBits(GPIOB, GPIO_Pin_5);
                GPIO_SetBits(GPIOB, GPIO_Pin_3);
                break;
            case GreenRight:
                GPIO_ResetBits(GPIOB, GPIO_Pin_3);
                GPIO_SetBits(GPIOB, GPIO_Pin_4);  
                break;
            case Yellow:  
                GPIO_ResetBits(GPIOB, GPIO_Pin_4);
                GPIO_SetBits(GPIOB, GPIO_Pin_5);    
                break;
            default:
                return;
        }                
    }
}


// 依照定时，更新交通灯状态
// 修改原始值和副本值的 w_Count 和 w_Current_state
uint16_t Light_Show(struct tagTraffic * ptagTra, uint8_t bDirection, struct tagTraffic * ptagTraCache)
{
    ptagTra->wCount = ptagTra->wCount + 1;
    ptagTraCache->wCount = ptagTra->wCount;
    
    switch(ptagTra->eCurrentState)
    {
        // 红灯及右转
        case RedRight:
        {
            if(ptagTra->wCount == ptagTra->wRedRightDelay)
            {
                ChangeLED();
                ptagTra->eCurrentState = RedLeft;
                ptagTraCache->eCurrentState = RedLeft;
            }
            return ptagTra->wRedLeftDelay + ptagTra->wRedRightDelay - ptagTra->wCount;
        }
        
        // 红灯及左转
        case RedLeft:
        {
            if(ptagTra->wCount == ptagTra->wRedLeftDelay + ptagTra->wRedRightDelay)
            {
                ChangeLED();
                // 当手动更新计时值时，重新启动后的临时状态
//                if(bTempStatus)
//                {
//                    ptagTra->wException = 1;
//                    ptagTraCache->wException = 1;
//                }
                ChangeDigit(bDirection, RedLeft); // 红灯结束，绿灯开启
                ptagTra->wCount = 0;
                ptagTraCache->wCount = 0;
                ptagTra->eCurrentState = GreenLeft;
                ptagTraCache->eCurrentState = GreenLeft;
                return 0;
            }
            return ptagTra->wRedLeftDelay + ptagTra->wRedRightDelay - ptagTra->wCount;
        }
        
        // 绿灯及左转
        case GreenLeft:
        {
            if(ptagTra->wCount == ptagTra->wGreenLeftDelay)
            {
                ChangeLED();
                ptagTra->eCurrentState = GreenRight;
                ptagTraCache->eCurrentState = GreenRight;                
            }
            return ptagTra->wGreenLeftDelay + ptagTra->wGreenRightDelay - ptagTra->wCount;
        }
        
        // 绿灯及右转
        case GreenRight:
        {
            if(ptagTra->wCount == ptagTra->wGreenLeftDelay + ptagTra->wGreenRightDelay)
            {
                ChangeLED();
                ChangeDigit(bDirection, GreenRight); // 绿灯结束，黄灯开启
                ptagTra->wCount = 0;
                ptagTraCache->wCount = 0;
                ptagTra->eCurrentState = Yellow; 
                ptagTraCache->eCurrentState = Yellow; 
                return 0;
            }
            return ptagTra->wGreenLeftDelay + ptagTra->wGreenRightDelay - ptagTra->wCount;
        }

        // 黄灯    
        case Yellow:
        {
            if(ptagTra->wCount == ptagTra->wYellowDelay)
            {
                ChangeLED();
                ChangeDigit(bDirection, Yellow); // 黄灯结束，红灯开启
                ptagTra->wCount = 0;
                ptagTraCache->wCount = 0;
                ptagTra->eCurrentState = RedRight;
                ptagTraCache->eCurrentState = RedRight; 
                return 0;
            }
            return ptagTra->wYellowDelay - ptagTra->wCount;
        }
    } // end switch()
}
