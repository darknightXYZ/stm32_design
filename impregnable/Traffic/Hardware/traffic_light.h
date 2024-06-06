#ifndef __TRAFFIC_LIGHT_H
#define __TRAFFIC_LIGHT_H


#define NS_DIRECTION 1 // 南北方向
#define EW_DIRECTION 0 // 东西方向


#define MIN_RED_DELAY 8
#define MAX_RED_DELAY 60
#define MIN_GREEN_DELAY 5
#define MAX_GREEN_DELAY 57


// 信号灯结构体
struct tagTraffic
{
    uint16_t w_green_delay;
    uint16_t w_red_delay;
    uint16_t w_yellow_delay;
    uint16_t w_Count;
    uint16_t w_Current_state;
    uint16_t w_exception;
};

// 初始化
void Traffic_Init(void);

// 主函数
void Traffic_Run(struct tagTraffic * tagEW, struct tagTraffic * tagNS, struct tagTraffic * tagEW_cache, struct tagTraffic * tagNS_cache);

uint8_t GetDigitValue(uint8_t b_value, uint16_t w_EW, uint16_t w_NS);
void Digit_Refresh_Show(uint8_t b_decoder_out);

uint16_t Light_Show(struct tagTraffic * tagTra, uint8_t b_direction, struct tagTraffic * tagTra_cache);

#endif
