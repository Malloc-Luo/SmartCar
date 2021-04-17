#ifndef _UART_H_
#define _UART_H_
#include "stm32f10x.h"
#include <stdbool.h>

/*
 * 初始化串口1和串口4
 */
extern void uart_init(void);
extern void send_esp8266_data(void);
extern void send_raspberry_data(void);

extern bool isRecvingFromEsp8266;
extern uint16_t RecvingFromEsp8266Cnt;

#endif
