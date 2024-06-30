#ifndef __TRAFFIC_LIGHT_H
#define __TRAFFIC_LIGHT_H

// 方向
#define NS_DIRECTION 1 // 南北方向
#define EW_DIRECTION 0 // 东西方向

// 状态
#define EW_RED_RIGHT_1 1
#define EW_RED_RIGHT_2 2
#define EW_GREEN_LEFT  3
#define EW_GREEN_RIGHT 4
#define EW_EVER_RED    5
#define EW_EVER_GREEN  6


enum eType
{
    GreenLeft,
    GreenRight,
    RedRight,
    Yellow,
};

// 信号灯结构体
struct tagTraffic
{
    uint16_t wGreenLeftDelay;
    uint16_t wGreenRightDelay;
    uint16_t wRedRightDelay;
    uint16_t wYellowDelay;
    uint16_t wCount;
    enum eType eCurrentState;
    uint16_t wException;
};

// 函数声明
void Traffic_Init(void);
void Traffic_Run(struct tagTraffic * tagEW, struct tagTraffic * tagNS, struct tagTraffic * tagEW_cache, struct tagTraffic * tagNS_cache);
uint8_t GetDigitValue(uint8_t b_value, uint16_t w_EW, uint16_t w_NS);
void Digit_Refresh_Show(uint8_t b_decoder_out);
uint16_t Light_Show(struct tagTraffic * ptagTra, uint8_t bDirection, struct tagTraffic * ptagTraCache);

#endif
