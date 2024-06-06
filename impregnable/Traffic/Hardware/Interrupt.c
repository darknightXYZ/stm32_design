#include "stm32f10x.h"  
#include "interrupt.h"

extern uint16_t w_digital_table[];

void NVIC_Configuration(void);
void EXTI_Configuration(void);
void GPIO_Configuration(void);

void switch_Init(void)
{
    GPIO_Configuration();
    EXTI_Configuration();
    NVIC_Configuration();
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 选择优先级组别
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; // 选择中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; // 响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能引脚作为中断源
	NVIC_Init(&NVIC_InitStructure); // 调用NVIC_Init固件库函数进行设置
    
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 选择优先级组别
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; // 选择中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能引脚作为中断源
	NVIC_Init(&NVIC_InitStructure); // 调用NVIC_Init固件库函数进行设置
}


void EXTI_Configuration(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4); // 参数分别是中断口和中断口对应的引脚号 ==> [PC4]
	EXTI_InitStructure.EXTI_Line = EXTI_Line4; // 将中断映射到中断源Line0
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿中断
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; // 中断使能，即开中断
	EXTI_Init(&EXTI_InitStructure); // 调用固件库函数，将结构体写入EXTI相关寄存器中
    
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5); // 参数分别是中断口和中断口对应的引脚号 ==> [PC5]
	EXTI_InitStructure.EXTI_Line = EXTI_Line5; // 将中断映射到中断源Line1
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿中断
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; // 中断使能，即开中断
	EXTI_Init(&EXTI_InitStructure); // 调用固件库函数，将结构体写入EXTI相关寄存器中
    
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6); // 参数分别是中断口和中断口对应的引脚号 ==> [PC6]
	EXTI_InitStructure.EXTI_Line = EXTI_Line6; // 将中断映射到中断源Line2
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿中断
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; // 中断使能，即开中断
	EXTI_Init(&EXTI_InitStructure); // 调用固件库函数，将结构体写入EXTI相关寄存器中
    
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7); // 参数分别是中断口和中断口对应的引脚号 ==> [PC7]
	EXTI_InitStructure.EXTI_Line = EXTI_Line7; // 将中断映射到中断源Line3
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿中断
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; // 中断使能，即开中断
	EXTI_Init(&EXTI_InitStructure); // 调用固件库函数，将结构体写入EXTI相关寄存器中
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; // 声明初始化GPIO的结构体
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE); // 使能PC口和AFIO时钟
	// 配置输入脚控制按键
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}



