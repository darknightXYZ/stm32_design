#ifndef __LED_H
#define __LED_H

void LED_Init(void);
void LED_shift(uint16_t num);
void LED_Loop(uint16_t *p_Count, uint16_t *p_reverse);
uint16_t State(uint16_t num,uint16_t flag);
uint16_t State_shift(uint16_t num,uint16_t flag);

#endif
