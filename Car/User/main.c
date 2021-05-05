/**
  ******************************************************************************
  * @file    main.c
  * @author  Smart Car System
  * @date    11-04-2021
  * @brief   Main program body
  ******************************************************************************
  */  

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "uart.h"
#include "timer.h"
#include "motorDrive.h"
#include "control.h"
#include <stdbool.h>

volatile uint32_t SysTickCnt = 0;
bool systemReady = false;

static inline void hardware_init(void) {
    /* 串口初始化 */
    uart_init();
    /* 编码器初始化 */
    encoder_init();
    /* 电机驱动初始化 */
    motor_driver_init();
    /* pid参数初始化 */
    pid_init();
    /* 系统滴答定时器中断1ms */
    SysTick_Config(SystemCoreClock / 1000);
    /* 高速时钟作为时钟源 */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    /* nvic中断分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* 系统初始化完成标志位 */
    systemReady = true;
}


int main() {
    hardware_init();
    
    for (;;) {
        /* run code here */
    }
    
    return 0;
}
