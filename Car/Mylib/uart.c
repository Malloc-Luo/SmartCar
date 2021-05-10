#include "uart.h"
#include "main.h"
#include "timer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Send2EspData_t Send2EspData;
RecvfromEspData_t RecvfromEspData;
Send2RaspberryData_t Send2RaspberryData;
RecvfromRaspberryData_t RecvfromRaspberryData;
void parse_feedback_data(const char data[]);
void parse_control_data(const char data[]);
extern SpeedStruct_t MotorSpeed;

bool isEsp8266Offline = true;
bool isRaspberryOffline = true;

/* Wifi设置及服务器host */
const char SSID[] = "yuqi";
const char PSWD[] = "20184023";
const char HOST[] = "39.106.216.248";
const char PORT[] = "19501";
/* 本地端口 */
const char LOCALPORT[] = "10086";
/* 小车ID */
const char CARID[] = "14vQy";
/* 小车名字，用于客户端显示 */
const char CARNAME[] = "SmartCar";

/**********************FUNCTION***********************
 * @brief: 初始化串口1(PA9 PA10)
 * @note:  波特率115200  一个停止位  无校验
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
    GPIO_Init(GPIOA, &gpio);
    
    gpio.GPIO_Pin = GPIO_Pin_10; /* RX */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);
    
    nvic.NVIC_IRQChannel = USART1_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic.NVIC_IRQChannelSubPriority = 0x00;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
    
    usart.USART_BaudRate = 115200;
	usart.USART_WordLength = USART_WordLength_8b;
	usart.USART_StopBits = USART_StopBits_1;
	usart.USART_Parity = USART_Parity_No;
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

/**********************FUNCTION***********************
 * @brief: 初始化串口2(PC10 PC11)
 * @note:  波特率115200  一个停止位  无校验
 *****************************************************/
DMA_InitTypeDef DMARx;
const uint16_t MSG_BUFFER_SIZE = 30;
/* 缓冲区 */
char FeedbackMsgBuffer[MSG_BUFFER_SIZE] = {'\0'};

static void uart4_init() {
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    
    gpio.GPIO_Pin = GPIO_Pin_10; /* TX */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);
    
    gpio.GPIO_Pin = GPIO_Pin_11; /* RX */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &gpio);
    
    nvic.NVIC_IRQChannel = UART4_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic.NVIC_IRQChannelSubPriority = 0x01;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
    
    USART_DeInit(UART4);
    usart.USART_BaudRate = 115200;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &usart);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    //USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
    USART_Cmd(UART4, ENABLE);
    
    DMARx.DMA_PeripheralBaseAddr = (uint32_t)&(UART4->DR);
    DMARx.DMA_MemoryBaseAddr = (uint32_t)FeedbackMsgBuffer;
    DMARx.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMARx.DMA_BufferSize = MSG_BUFFER_SIZE;
    DMARx.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMARx.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMARx.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMARx.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMARx.DMA_Mode = DMA_Mode_Circular;
    DMARx.DMA_Priority = DMA_Priority_High;
    DMARx.DMA_M2M = DMA_M2M_Disable;
    /* DMA2 通道3 */
    DMA_Init(DMA2_Channel3, &DMARx);
    USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA2_Channel3, ENABLE);
    
    DMA_ITConfig(DMA2_Channel3, DMA_IT_TC, ENABLE);
    nvic.NVIC_IRQChannel = DMA2_Channel3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_Init(&nvic);
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

static void uart4_send_string(const char string[]) {
    uint8_t i = 0;
    while (string[i]) {
        uart4_send_byte(string[i++]);
    }
}

/**
 * AT指令Esp8266 UDP连接
 * 参考一个大佬的代码，以及：
 * https://blog.csdn.net/qq_38410730/article/details/86538288
 */

/* AT指令建立连接 */
typedef enum {
    SetWorkMode, /* 设置工作模式 */
    SetRST,      /* 模块重启 */
    EchoOff,     /* 关闭回显 */
    SetWifi,     /* 连接Wifi */
    SetUDP,      /* 设置UDP的host和port */
    StartCipMode,/* 开启透传模式 */
    SendData,    /* 开始传输数据 */
    Sending,     /* 发送 */
    StopCipMode, /* 关闭透传模式 */
} Esp8266SendStatus_t;

Esp8266SendStatus_t Esp8266SendStatus = SetWorkMode;
static char SendBuffer[80] = {'\0'};
/* 等待回应 */
bool isWaitFeedback = false;
uint16_t WaitFeedbackCnt = 0;
uint16_t ProcWait = 0;

/**********************FUNCTION***********************
 * @brief: 向Esp8266发送数据，也就是反馈到PC客户端的数据
 * @note:  要在这个函数里补充更新结构体的代码
 *****************************************************/
void send_esp8266_data() {
    static uint8_t *head = (uint8_t *)&Send2EspData;
    if (isWaitFeedback == false) {
        switch (Esp8266SendStatus) {
        case SetWorkMode: {
            uart4_send_string("AT+CWMODE=3\r\n");
            break;
        }
        case SetRST: {
            uart4_send_string("AT+RST\r\n");
            break;
        }
        case EchoOff: {
            uart4_send_string("ATE0\r\n");
            break;
        }
        case SetWifi: {
            sprintf(SendBuffer, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", SSID, PSWD);
            uart4_send_string(SendBuffer);
            memset(SendBuffer, '\0', 80);
            break;
        }
        case SetUDP: {
            /* IP 远程端口 本地端口10086 */
            sprintf(SendBuffer, "AT+CIPSTART=\"UDP\",\"%s\",%s,%s,0\r\n", HOST, PORT, LOCALPORT);
            uart4_send_string(SendBuffer);
            memset(SendBuffer, '\0', 80);
            break;
        }
        case StartCipMode: {
            uart4_send_string("AT+CIPMODE=1\r\n");
            break;
        }
        case SendData: {
            uart4_send_string("AT+CIPSEND\r\n");
            break;
        }
        case Sending: {
            /* 发送字符串 */
            sprintf(SendBuffer, "slave+%s+%s+%d,%d,%d,%d", CARID, CARNAME, MotorSpeed.v1, MotorSpeed.v2, MotorSpeed.v3, MotorSpeed.v4);
            uart4_send_string(SendBuffer);
            memset(SendBuffer, '\0', 80);
            break;
        }
        case StopCipMode: {
            uart4_send_string("+++");
            break;
        }
        default: break;
        }
        
        if (Esp8266SendStatus != Sending) {
            isWaitFeedback = true;
        }
    }
    
    /* 超时解析数据 */
    if (Esp8266SendStatus != Sending && ProcWait > 35) {
        parse_feedback_data(FeedbackMsgBuffer);
        memset(FeedbackMsgBuffer, '\0', MSG_BUFFER_SIZE);
        ProcWait = 0;
    }
    ProcWait++;
}

/**********************FUNCTION***********************
 * @brief: 向Raspberry发送数据，选择模式
 * @note:  更新数据代码
 *****************************************************/


extern uint32_t raspberryOfflineCnt;
extern uint32_t esp8266OfflineCnt;
uint16_t isIdle = 0;
/**
 * 中断服务函数，接收到\n或\r停止
 */
void USART1_IRQHandler() {
    static uint8_t recvCnt = 0;
    
    if (USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        uint8_t data = USART_ReceiveData(USART1);
        /* 如果接收到\n\r或者缓存溢出则停止 */
        switch (data) {
            case '1':
                RecvfromRaspberryData.action = Action_Back;
                break;
            case '2':
                RecvfromRaspberryData.action = Action_Fore;
                break;
            default:
                RecvfromRaspberryData.action = Action_Pause;
                break;
        }
        raspberryOfflineCnt = 0;
    }
}

/**********************FUNCTION***********************
  * @brief: 处理应答数据，完成状态转换
  * @param: const char[] 收到的应答数据
  * @return: None
  * @note:
  *****************************************************/
void parse_feedback_data(const char data[]) {
    if (strstr(data, "OK") != NULL || strstr(data, ">") != NULL) {
        switch (Esp8266SendStatus) {
        case SetWorkMode: {
            Esp8266SendStatus = SetRST;
            break;
        }
        case SetRST: {
            Esp8266SendStatus = EchoOff;
            break;
        }
        case EchoOff: {
            Esp8266SendStatus = SetWifi;
            break;
        }
        case SetWifi: {
            Esp8266SendStatus = SetUDP;
            break;
        }
        case SetUDP: {
            /* 开启透传模式 */
            Esp8266SendStatus = StartCipMode;
            break;
        }
        case StartCipMode: {
            Esp8266SendStatus = SendData;
            break;
        }
        case SendData: {
            Esp8266SendStatus = Sending;
            break;
        }
        case Sending:
        case StopCipMode:
        default:
            break;
        }
    }
}

/*
 * 解析字符串
 * 类似这种：1,12,15,-56
 */
void parse_control_data(const char data[]) {
    uint8_t len = strlen(data), index = 0;
    int8_t v[5] = {0, 0, 0, 0, 0};
    uint8_t cnt = 0;
    while (data[index] == '+') { 
        index++; 
    }
    while (index < len && data[index] != '+') {
        bool sig = false;
        while (data[index] != ',' && data[index] != '+') {
            if (data[index] == '-') {
                sig = true;
            } else {
                v[cnt] = v[cnt] * 10 + (data[index] - '0');
            }
            if (++index == len) {
                break;
            }
        }
        index++;
        if (sig) {
            v[cnt] *= -1;
        }
        cnt++;
    }
    RecvfromEspData.mode = v[0] ? Mode_Remote : Mode_Identify;
    RecvfromEspData.vx = v[1];
    RecvfromEspData.vy = v[2];
    RecvfromEspData.vr = v[3];
}


void UART4_IRQHandler() {
    /* 接收Sending模式的数据 */
    static uint8_t sendingCnt = 0;
    static bool recvflag = false;
    /* 没有什么用的变量 */
    uint8_t uselessTemp;
    
    /* 接收到数据 */
    if (USART_GetFlagStatus(UART4, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        esp8266OfflineCnt = 0;
    }
    
    if (USART_GetFlagStatus(UART4, USART_IT_ORE) != RESET) {
        uselessTemp = UART4->SR;
        uselessTemp = UART4->DR;
        USART_ClearFlag(UART4, USART_FLAG_ORE);
    }
}

/*
 * DMA接收中断
 */
void DMA2_Channel3_IRQHandler(void) {
    if (DMA_GetITStatus(DMA2_IT_TC3) != RESET) {
        DMA_ClearITPendingBit(DMA2_IT_TC3);
        WaitFeedbackCnt = 0;
        ProcWait = 0;
        if (Esp8266SendStatus != Sending) {
            parse_feedback_data(FeedbackMsgBuffer);
        } else {
            parse_control_data(FeedbackMsgBuffer);
        }
        memset(FeedbackMsgBuffer, '\0', MSG_BUFFER_SIZE);
    }
}

