#include "uart.h"
#include "main.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Send2EspData_t Send2EspData;
RecvfromEspData_t RecvfromEspData;
Send2RaspberryData_t Send2RaspberryData;
RecvfromRaspberryData_t RecvfromRaspberryData;

bool isEsp8266Offline = true;
bool isRaspberryOffline = true;

/* Wifi设置及服务器host */
const char SSID[] = "yuqi";
const char PSWD[] = "20184023";
const char HOST[] = "39.106.216.248";
const char PORT[] = "19501";
/* 本地端口 */
const char LOCALPORT[] = "10086";

/**********************FUNCTION***********************
 * @brief: 初始化串口1(PA9 PA10)
 * @note:  波特率9600  一个停止位  无校验
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
    
    usart.USART_BaudRate = 9600;                                    
	usart.USART_WordLength = USART_WordLength_8b;                    
	usart.USART_StopBits = USART_StopBits_1;                        
	usart.USART_Parity = USART_Parity_No;                            
	usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
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
/* 通信缓冲区 */
uint8_t DataBuffer[sizeof(RecvfromEspData_t)] = {0};

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
    //USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
    //DMA_Cmd(DMA2_Channel3, ENABLE);
    
    //DMA_ITConfig(DMA2_Channel3, DMA_IT_TC, ENABLE);
    nvic.NVIC_IRQChannel = DMA2_Channel3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_Init(&nvic);
}

/**********************FUNCTION***********************
  * @brief: DMA重新初始化
  * @param: mode: 0 连接模式 1 通信模式
  * @return: None
  * @note:
  *****************************************************/
static void dma_reinit(uint8_t mode) {
    DMA_Cmd(DMA2_Channel3, DISABLE);
    DMA_DeInit(DMA2_Channel3);
    
    /* 发送数据模式 */
    if (mode == 1) {
        DMARx.DMA_MemoryBaseAddr = (uint32_t)DataBuffer;
        DMARx.DMA_BufferSize = sizeof(RecvfromEspData_t);
    } else {
        DMARx.DMA_MemoryBaseAddr = (uint32_t)FeedbackMsgBuffer;
        DMARx.DMA_BufferSize = MSG_BUFFER_SIZE;
    }
    DMA_Init(DMA2_Channel3, &DMARx);
    DMA_Cmd(DMA2_Channel3, ENABLE);
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

/* 向Esp8266发送数据的状态 */
typedef enum {
    Disconnect, /* 未连接状态 */
    Connect,    /* 建立连接*/
    CIPMode     /* 透传模式 */
} Esp8266LinkStatus_t;

/* AT指令建立连接 */
typedef enum {
    SetWorkMode, /* 设置工作模式 */
    SetRST,      /* 模块重启 */
    EchoOff,     /* 关闭回显 */
    SetWifi,     /* 连接Wifi */
    SetCipMux,   /* 设置单路由模式 */
    SetUDP,      /* 设置UDP的host和port */
    StartCipMode,/* 开启透传模式 */
    SendData,    /* 开始传输数据 */
    Sending,     /* 发送 */
    StopCipMode, /* 关闭透传模式 */
} Esp8266SendStatus_t;

Esp8266SendStatus_t Esp8266SendStatus = SetWorkMode;
static char SendBuffer[100] = {'\0'};
/* 等待回应 */
bool isWaitFeedback = false;
uint16_t WaitFeedbackCnt = 0;

/**********************FUNCTION***********************
 * @brief: 向Esp8266发送数据，也就是反馈到PC客户端的数据
 * @note:  要在这个函数里补充更新结构体的代码
 *****************************************************/
void send_esp8266_data() {
    static uint8_t *head = (uint8_t *)&Send2EspData;
    /***
     * 如果不是发送数据模式，则每次发送后等待验证信息
     * 如果等待200ms后没有回应或者回应错误，则再次发送，状态不变
     */
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
            /* 发送完之后等1s，等待数据处理 */
            uart4_send_string(SendBuffer);
            break;
        }
        case SetCipMux: {
            uart4_send_string("AT+CIPMUX=0\r\n");
            break;
        }
        case SetUDP: {
            /* IP 远程端口 本地端口10086 */
            sprintf(SendBuffer, "AT+CIPSTART=\"UDP\",\"%s\",%s,%s,0", HOST, PORT, LOCALPORT);
            uart4_send_string(SendBuffer);
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
            /*****
             * \TODO:在这里更新结构体的信息，也就是四个轮子编码器读到的速度
             ****/
            uart4_send_string("Hello, I am STM32");
//            Send2EspData.header = 0xa5;
//            uint8_t ptr = 0;
//            for (ptr = 0; ptr < sizeof(Send2EspData_t); ptr++) {
//                uart4_send_byte(*(head + ptr));
//            }
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

extern uint32_t raspberryOfflineCnt;
extern uint32_t esp8266OfflineCnt;
/**
 * 中断服务函数
 */
void USART1_IRQHandler() {
    static uint8_t recvCnt = 0;
    static uint8_t buffer[sizeof(RecvfromRaspberryData_t)] = {0};
    static bool recvFlag = false;
    uint8_t uselessTemp;
    
    if (USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        uint8_t data = USART_ReceiveData(USART1);
        /* 接到帧头后准备接收后续的数字 */
        if (data == 0xa5) {
            recvFlag = true;
        }
        if (recvFlag) {
            if (recvCnt < sizeof(RecvfromRaspberryData_t)) {
                buffer[recvCnt++] = data;
            } else {
                recvCnt = 0;
                recvFlag = false;
            }
        }
        raspberryOfflineCnt = 0;
    }
    
    if (USART_GetFlagStatus(USART1, USART_IT_IDLE) != RESET) {
        uselessTemp = USART1->SR;
        uselessTemp = USART1->DR;
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
        raspberryOfflineCnt = 0;
        
        memcpy((uint8_t *)&RecvfromRaspberryData, buffer, sizeof(RecvfromRaspberryData_t));
        recvCnt = 0;
        recvFlag = false;
    }
}

/**********************FUNCTION***********************
  * @brief: 处理应答数据，完成状态转换
  * @param: const char[] 收到的应答数据
  * @return: None
  * @note:
  *****************************************************/
void parse_feedback_data(const char data[]) {
    if (strstr(data, "OK") != NULL) {
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
            Esp8266SendStatus = SetCipMux;
            break;
        }
        case SetCipMux: {
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
            /* 重新配置DMA */
            dma_reinit(1);
            break;
        }
        case Sending:
        case StopCipMode:
        default:
            break;
        }
    }
}

void UART4_IRQHandler() {
    /* 接收Sending模式的数据 */
    static uint8_t sendingCnt = 0;
    /* Sending模式开始接收 */
    static bool recvflag = false;
    /* 非sending模式接收 */
    static uint8_t msgCnt = 0;
    /* 没有什么用的变量 */
    uint8_t uselessTemp;
    
    /* 接收到数据 */
    if (USART_GetFlagStatus(UART4, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        esp8266OfflineCnt = 0;
        uint8_t data = USART_ReceiveData(UART4);
        
        if (Esp8266SendStatus == Sending) {
            if (data == 0xa5) {
                recvflag = true;
            }
            if (recvflag == true && sendingCnt < sizeof(RecvfromEspData_t)) {
                DataBuffer[sendingCnt++] = data;
            }
        } else {
            isWaitFeedback = false;
            WaitFeedbackCnt = 0;
            if (msgCnt < MSG_BUFFER_SIZE && data != '\0') {
                FeedbackMsgBuffer[msgCnt++] = data;
            } else {
                msgCnt = 0;
                parse_feedback_data(FeedbackMsgBuffer);
            }
        }
    }
    
    /* 进入空闲中断，接收结束 */
    if (USART_GetFlagStatus(UART4, USART_IT_IDLE) == SET) {
        /* 没有实际用途但必须有，否则无法清除空闲中断标志位 */
        uselessTemp = UART4->SR;
        uselessTemp = UART4->DR;

        esp8266OfflineCnt = 0;
        /* 关闭DMA防止干扰 */
        DMA_Cmd(DMA2_Channel3, DISABLE);
        DMA_ClearFlag(DMA2_FLAG_TC3);
        
        uint16_t length = MSG_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel4);
        FeedbackMsgBuffer[length] = '\0';
        
        if (Esp8266SendStatus == Sending) {
            memcpy((uint8_t *)&RecvfromEspData, DataBuffer, sizeof(RecvfromEspData_t));
            sendingCnt = 0;
            recvflag = false;
        } else {
            parse_feedback_data(FeedbackMsgBuffer);
            memset(FeedbackMsgBuffer, '\0', MSG_BUFFER_SIZE);
            msgCnt = 0;
        }
        DMA_Cmd(DMA2_Channel3, ENABLE);
    }
    
    if (USART_GetFlagStatus(UART4, USART_IT_ORE) != RESET) {
        uselessTemp = UART4->SR;
        uselessTemp = UART4->DR;
        USART_ClearFlag(UART4, USART_FLAG_ORE);
    }
}

void DMA2_Channel3_IRQHandler(void) {
    if (DMA_GetITStatus(DMA2_IT_TC3) != RESET) {
        DMA_ClearITPendingBit(DMA2_IT_TC3);
        
        if (Esp8266SendStatus != Sending) {
            parse_feedback_data(FeedbackMsgBuffer);
            memset(FeedbackMsgBuffer, '\0', MSG_BUFFER_SIZE);
        } else {
            if (DataBuffer[0] == 0xa5) {
                memcpy((uint8_t *)&RecvfromEspData, DataBuffer, sizeof(RecvfromEspData_t));
            }
        }
    }
}

