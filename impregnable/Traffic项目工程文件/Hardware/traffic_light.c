#include "stm32f10x.h"                 
#include "traffic_light.h"


// 双位数码管0~9对照表
const uint16_t aryDigitTable[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f 
};

uint16_t wEWCountDown = 0; // 东西方向倒计时
uint16_t wNSCountDown = 0; // 南北方向倒计时

extern uint8_t bTempStatus;


void Traffic_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 开启GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
    // 信号灯初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7| GPIO_Pin_8| GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_6| GPIO_Pin_8); // 012红黄绿 345绿黄红 6和8为东西方向右转和南北方向右转
    
    // 数码管初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 译码器输入端初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}


// 交通灯定时运行

/* 
    tagEW/tagNS 为传递的交通灯结构体的指针，用来更新其中的wCount计数成员变量值以及eCurrentState值
    tagEWCache/tagNSCache 为传递的副本结构体的指针，当外部修改使得副本结构体和原始结构体不同步时需要进行更新
*/
void Traffic_Run(struct tagTraffic * tagEW, struct tagTraffic * tagNS, struct tagTraffic * tagEWCache, struct tagTraffic * tagNSCache)
{
    // 依照原始结构体更新两个方向上的倒计时，原始结构体和副本结构体同步变化
    wEWCountDown = Light_Show(tagEW, EW_DIRECTION, tagEWCache);
    wNSCountDown = Light_Show(tagNS, NS_DIRECTION, tagNSCache);
    
    // 手动配置副本值延时值后，此时原始值和副本值不同步，当结构体内部计数值均为0时通过副本值更新原始结构体内容，并清除临时状态标志位
    if(bTempStatus && (tagEW->wCount == 0 && tagNS->wCount == 0))
    {
        *tagEW = *tagEWCache;
        *tagNS = *tagNSCache;
        bTempStatus = 0;
    }
}

// 获取数码管特定位的显示数字

/*
    bDecoderOut为译码器的输出引脚，根据其值去确定当前显示的是哪个位置上的数字
    w_EW和w_NS是南北方向和东西方向上需要显示的数字
*/
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

/*
    在GetDigitValue函数后调用，获得需要显示的某个数字b_value后去通过对照表获取编码并输出
*/
void Digit_Refresh_Show(uint8_t b_value)
{
    GPIOA->ODR = aryDigitTable[b_value];
}


// 根据方向和当前状态更新交通灯

/*
    bDirection当前方向
    eState该方向上当前结构体状态
*/
void ChangeDigit(uint8_t bDirection, enum eType eState)
{
    if(bDirection == EW_DIRECTION) // 东西方向
    {
        switch(eState)
        {
            case RedRight: // 红右结束
                GPIO_ResetBits(GPIOB, GPIO_Pin_0|GPIO_Pin_6);
                GPIO_SetBits(GPIOB, GPIO_Pin_2|GPIO_Pin_6);
                break;
            
            case GreenRight: // 绿右结束
                GPIO_ResetBits(GPIOB, GPIO_Pin_2|GPIO_Pin_6);
                GPIO_SetBits(GPIOB, GPIO_Pin_2|GPIO_Pin_7); 
                break;
            
            case GreenLeft: // 绿左结束
                GPIO_ResetBits(GPIOB, GPIO_Pin_2|GPIO_Pin_7);
                GPIO_SetBits(GPIOB, GPIO_Pin_1|GPIO_Pin_7);
                break;
                
            case Yellow: // 黄灯结束，进入红右
                GPIO_ResetBits(GPIOB, GPIO_Pin_1 | GPIO_Pin_7);
                GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_6);
                break;
            
            default:
                return;
        }
    }
    if(bDirection == NS_DIRECTION) // 南北方向
    {
        switch(eState)
        {
            case RedRight:
                GPIO_ResetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_8);
                GPIO_SetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_8);
                break;
            case GreenRight:
                GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_8);
                GPIO_SetBits(GPIOB, GPIO_Pin_3 |GPIO_Pin_9);  
                break;
            case GreenLeft:
                GPIO_ResetBits(GPIOB, GPIO_Pin_3 | GPIO_Pin_9);
                GPIO_SetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_9); 
                break;
            case Yellow:  
                GPIO_ResetBits(GPIOB, GPIO_Pin_4 | GPIO_Pin_9);
                GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_8); 
                break;
            default:
                return;
        }                
    }
}


// 依照定时，更新交通灯状态（修改原始值和副本值的 w_Count 和 w_Current_state）

/*
    ptagTra/ptagTraCache为指向原始结构体和副本结构体的指针
    bDirection为东西方向或南北方向
*/
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
                ChangeDigit(bDirection, RedRight); // 改变显示灯状态
                ptagTra->wCount = 0; // 计数清零
                ptagTraCache->wCount = 0;
                ptagTra->eCurrentState = GreenRight; // 红右下一阶段：绿右
                ptagTraCache->eCurrentState = GreenRight;
                return 0;
            }
            return ptagTra->wRedRightDelay - ptagTra->wCount;
        }

        // 绿灯及右转
        case GreenRight:
        {
            if(ptagTra->wCount == ptagTra->wGreenRightDelay)
            {
                ChangeDigit(bDirection, GreenRight); // 绿灯结束，黄灯开启
                ptagTra->eCurrentState = GreenLeft;
                ptagTraCache->eCurrentState = GreenLeft; 
            }
            return ptagTra->wGreenLeftDelay + ptagTra->wGreenRightDelay - ptagTra->wCount;
        }


        // 绿灯及左转
        case GreenLeft:
        {
            if(ptagTra->wCount == ptagTra->wGreenLeftDelay + ptagTra->wGreenRightDelay)
            {
                ChangeDigit(bDirection, GreenLeft);
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
                ChangeDigit(bDirection, Yellow); // 黄灯结束，红灯开启
                ptagTra->wCount = 0;
                ptagTraCache->wCount = 0;
                ptagTra->eCurrentState = RedRight;
                ptagTraCache->eCurrentState = RedRight; 
                return 0;
            }
            return ptagTra->wYellowDelay - ptagTra->wCount;
        }
        
        default:
            return 0;
        
    } // end switch()
}
