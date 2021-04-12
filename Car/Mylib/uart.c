#include "uart.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>

Send2EspData_t Send2EspData;
RecvfromEspData_t RecvfromEspData;
Send2RaspberryData_t Send2RaspberryData;
RecvfromRaspberryData_t RecvfromRaspberryData;

bool isEsp8266Offline = true;
bool isRaspberryOffline = true;

/**********************FUNCTION***********************
 * @brief: 初始化串口1(PA9 PA10)
 * @param: None
 * @return:None
 * @note:  波特率9600  一个停止位  奇校验
 *****************************************************/
static void usart1_init() {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    gpio.GPIO_Pin = GPIO_Pin_9; /* TX */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA ,&gpio);
    
    gpio.GPIO_Pin = GPIO_Pin_10; /* RX */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA ,&gpio);
    
    nvic.NVIC_IRQChannel = USART1_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic.NVIC_IRQChannelSubPriority = 0x00;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
    
    usart.USART_BaudRate = 9600;                                    
	usart.USART_WordLength = USART_WordLength_9b;                    
	usart.USART_StopBits = USART_StopBits_1;                        
	usart.USART_Parity = USART_Parity_Odd;                            
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

/**********************FUNCTION***********************
 * @brief: 初始化串口2(PC10 PC11)
 * @param: None
 * @return:None
 * @note:  波特率9600  一个停止位  奇校验
 *****************************************************/
static void uart4_init() {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    
    gpio.GPIO_Pin = GPIO_Pin_10; /* TX */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC ,&gpio);
    
    gpio.GPIO_Pin = GPIO_Pin_11; /* RX */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC ,&gpio);
    
    nvic.NVIC_IRQChannel = UART4_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic.NVIC_IRQChannelSubPriority = 0x01;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
    
    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_9b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_Odd;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &usart);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
}

void uart_init() {
    usart1_init();
    uart4_init();
}

static void uart1_send_byte(uint8_t data) {
    USART_SendData(USART1, data);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

static void uart4_send_byte(uint8_t data) {
    USART_SendData(UART4, data);
    while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
}

/**********************FUNCTION***********************
 * @brief: 向Esp8266发送数据，也就是反馈到PC客户端的数据
 * @note:  要在这个函数里补充更新结构体的代码
 *****************************************************/
void send_esp8266_data() {
    static uint8_t *head = (uint8_t *)&Send2EspData;
    /*****
     * \TODO:在这里更新结构体的信息，也就是四个轮子编码器读到的速度
     ****/
    Send2EspData.header = 0xa5;
    /* Code */
    
    uint8_t ptr = 0;
    for (ptr = 0; ptr < sizeof(Send2EspData_t); ptr++) {
        uart4_send_byte(*(head + ptr));
    }
}

/**********************FUNCTION***********************
 * @brief: 向Raspberry发送数据，选择模式
 * @note:  更新数据代码
 *****************************************************/
void send_raspberry_data() {
    static uint8_t *head2 = (uint8_t *)&Send2RaspberryData;
    /**
     * \TODO: 在这里更新数据
     */
    uint8_t ptr = 0;
    for (ptr = 0; ptr < sizeof(Send2RaspberryData_t); ptr++) {
        uart1_send_byte(*(head2 + ptr));
    }
}

/**
 * 中断服务函数
 */
void USART1_IRQHandler() {
    static uint8_t recvCnt = 0;
    static uint8_t buffer[sizeof(RecvfromRaspberryData_t)] = {0};
    static bool recvFlag = false;
    
    if (USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        uint8_t data = USART_ReceiveData(USART1);
        
        if (data == 0xa5) {
            recvFlag = true;
        }
        
        if (recvFlag) {
            buffer[recvCnt++] = data;
            if (recvCnt >= sizeof(RecvfromRaspberryData_t)) {
                memcpy((uint8_t *)&RecvfromRaspberryData, buffer, sizeof(RecvfromRaspberryData_t));
                recvCnt = 0;
                recvFlag = false;
            }
        }
    }
}

void UART4_IRQHandler() {
    static uint8_t recvCnt = 0;
    static uint8_t buffer[sizeof(RecvfromRaspberryData)] = {0};
    static bool recvflag = false;
    
    if (USART_GetFlagStatus(UART4, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        uint8_t data = USART_ReceiveData(UART4);
        
        if (data == 0xa5) {
            recvflag = true;
        }
        if (recvflag == true) {
            buffer[recvCnt++] = data;
            if (recvCnt >= sizeof(RecvfromRaspberryData_t)) {
                memcpy((uint8_t *)&RecvfromRaspberryData, buffer, sizeof(RecvfromRaspberryData_t));
                recvCnt = 0;
                recvflag = false;
            }
        }
    }
}

