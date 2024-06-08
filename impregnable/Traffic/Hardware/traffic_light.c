#include "stm32f10x.h"                  // Device header
#include "traffic_light.h"

//###################################

// 双位数码管0~9对照表
const uint16_t w_digital_table[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f 
};

uint16_t w_EW_CountDown = 0; // 东西方向倒计时
uint16_t w_NS_CountDown = 0; // 南北方向倒计时
uint16_t w_EW_PressCount = 0; // 东西方向点按频次
uint16_t w_NS_PressCount = 0; // 南北方向点按频次

extern int8_t temp_status;

//###################################

void Traffic_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 开启GPIO时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
    // 信号灯初始化
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3| GPIO_Pin_4| GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_3); // 测试使用 012红黄绿 345绿黄红
    
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

// 交通灯定时运行
void Traffic_Run(struct tagTraffic * tagEW, struct tagTraffic * tagNS, struct tagTraffic * tagEW_cache, struct tagTraffic * tagNS_cache)
{
    w_EW_CountDown = Light_Show(tagEW, EW_DIRECTION, tagEW_cache);
    w_NS_CountDown = Light_Show(tagNS, NS_DIRECTION, tagNS_cache);
    if(temp_status)
    {
        if((w_EW_CountDown == 0 || w_NS_CountDown == 0) && (tagEW->w_exception == 1))
        {
            tagEW->w_red_delay = tagEW_cache->w_red_delay;
            tagNS->w_green_delay = tagNS_cache->w_green_delay;
            tagEW->w_green_delay = tagEW_cache->w_green_delay;
            tagNS->w_red_delay = tagNS_cache->w_red_delay;
            tagEW->w_exception = 0;
            tagEW_cache->w_exception = 0;
            temp_status = 0;
        }

    }
}

// 获取数码管特定位的显示数字
uint8_t GetDigitValue(uint8_t b_decoder_out, uint16_t w_EW, uint16_t w_NS)
{
    uint8_t b_temp = 0;
    switch(b_decoder_out)
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


// 依照定时，更新交通灯状态
// 修改原始值和副本值的 w_Count 和 w_Current_state
uint16_t Light_Show(struct tagTraffic * tagTra, uint8_t b_direction, struct tagTraffic * tagTra_cache)
{
    tagTra->w_Count = tagTra->w_Count + 1;
    tagTra_cache->w_Count = tagTra->w_Count; // #####
    
    switch(tagTra->w_Current_state)
    {
        case 0:
        {            // 红灯
            if(tagTra->w_Count == tagTra->w_red_delay) // +1会在结果中多一次00
            { 
                if(!b_direction) // 东西方向
                {                  
                    if(temp_status)
                    {
                        tagTra->w_exception = 1;
                        tagTra_cache->w_exception = 1;
                    }
                    GPIO_ResetBits(GPIOB, GPIO_Pin_0);
                    GPIO_SetBits(GPIOB, GPIO_Pin_2);
                }else            // 南北方向
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_5);
                    GPIO_SetBits(GPIOB, GPIO_Pin_3);
                }
                tagTra->w_Count = 0;
                tagTra_cache->w_Count = tagTra->w_Count; // #######
                tagTra->w_Current_state = 1;
                tagTra_cache->w_Count = tagTra->w_Current_state; // #####
                
                return 0; // 防止在倒计时结束后短暂一刻由于后面的语句又使数码管显示为最大值
                
            } // endif(tagTra->w_Count == tagTra->w_red_delay)
            return tagTra->w_red_delay - tagTra->w_Count; // 红灯倒计时
        } // endcase 0
        
        case 1:
        {            // 绿灯
            if(tagTra->w_Count == tagTra->w_green_delay)
            {
                if(!b_direction)
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_2);
                    GPIO_SetBits(GPIOB, GPIO_Pin_1);
                }else
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_3);
                    GPIO_SetBits(GPIOB, GPIO_Pin_4);
                }
                tagTra->w_Count = 0;
                tagTra_cache->w_Count = tagTra->w_Count; // #######
                tagTra->w_Current_state = 2;
                tagTra_cache->w_Count = tagTra->w_Current_state; // #####
               
                return 0;
            }
            return tagTra->w_green_delay - tagTra->w_Count; // 绿灯倒计时
        }
        
        case 2: // 黄灯
        {
            if(tagTra->w_Count == tagTra->w_yellow_delay)
            {
                if(!b_direction) // 红灯
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
                    GPIO_SetBits(GPIOB, GPIO_Pin_0);
                }else
                {
                    GPIO_ResetBits(GPIOB, GPIO_Pin_4);
                    GPIO_SetBits(GPIOB, GPIO_Pin_5);
                }
                tagTra->w_Count = 0;
                tagTra_cache->w_Count = tagTra->w_Count; // #######
                tagTra->w_Current_state = 0;
                tagTra_cache->w_Count = tagTra->w_Current_state; // #####
                
                return 0;
            }
            return tagTra->w_yellow_delay - tagTra->w_Count; // 黄灯倒计时
        }
    } // end switch()
}
