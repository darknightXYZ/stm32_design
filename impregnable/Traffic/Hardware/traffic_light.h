#ifndef __TRAFFIC_LIGHT_H
#define __TRAFFIC_LIGHT_H


#define NS_DIRECTION 1 // 南北方向
#define EW_DIRECTION 0 // 东西方向


//#define MIN_RED_DELAY 8
//#define MAX_RED_DELAY 60
//#define MIN_GREEN_DELAY 5
//#define MAX_GREEN_DELAY 57

#define MIN_RED_LEFT_DALAY 6
#define MIN_RED_RIGHT_DELAY 3
#define MIN_GREEN_LEFT_DELAY 3
#define MIN_GREEN_RIGHT_DELAY 5


#define MAX_RED_LEFT_DALAY 60
#define MAX_RED_RIGHT_DELAY 30
#define MAX_GREEN_LEFT_DELAY 30
#define MAX_GREEN_RIGHT_DELAY 50


#define EW_RED_LEFT 1
#define EW_RED_RIGHT 2
#define EW_GREEN_LEFT 3
#define EW_GREEN_RIGHT 4


enum eType
{
    GreenLeft,
    GreenRight,
    RedLeft,
    RedRight,
    Yellow,
};

// 信号灯结构体
struct tagTraffic
{
    uint16_t wGreenLeftDelay;
    uint16_t wGreenRightDelay;
    uint16_t wRedLeftDelay;
    uint16_t wRedRightDelay;
    uint16_t wYellowDelay;
    uint16_t wCount;
    enum eType eCurrentState;
    uint16_t wException;
};

// 初始化
void Traffic_Init(void);

// 主函数
void Traffic_Run(struct tagTraffic * tagEW, struct tagTraffic * tagNS, struct tagTraffic * tagEW_cache, struct tagTraffic * tagNS_cache);

uint8_t GetDigitValue(uint8_t b_value, uint16_t w_EW, uint16_t w_NS);
void Digit_Refresh_Show(uint8_t b_decoder_out);

uint16_t Light_Show(struct tagTraffic * tagTra, uint8_t b_direction, struct tagTraffic * tagTra_cache);

uint16_t GetMinimum(uint16_t flag);
uint16_t GetMaximum(uint16_t flag);

#endif
