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

/**********************FUNCTION***********************
 * @brief: 初始化串口1(PA9 PA10)
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
 * @note:  波特率115200  一个停止位  奇校验
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
    GPIO_Init(GPIOC, &gpio);
    
    gpio.GPIO_Pin = GPIO_Pin_11; /* RX */
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &gpio);
    
    nvic.NVIC_IRQChannel = UART4_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0x01;
	nvic.NVIC_IRQChannelSubPriority = 0x01;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
    
    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
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


static void uart4_send_string(const char string[]) {
    uint8_t i = 0;
    while (string[i]) {
        uart1_send_byte(string[i++]);
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
    SetWifi,     /* 连接Wifi */
    SetCipMux,   /* 设置单路由模式 */
    SetUDP,      /* 设置UDP的host和port */
    StartCipMode,/* 开启透传模式 */
    SendData,    /* 开始传输数据 */
    Sending,     /* 发送 */
    StopCipMode, /* 关闭透传模式 */
} Esp8266SendStatus_t;

static Esp8266SendStatus_t Esp8266SendStatus = SetWorkMode;
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
            uart4_send_string("AT+CWMODE=1\r\n");
            break;
        }
        case SetRST: {
            uart4_send_string("AT+RST\r\n");
            break;
        }
        case SetWifi: {
            sprintf(SendBuffer, "AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", SSID, PSWD);
            uart4_send_string(SendBuffer);
            break;
        }
        case SetCipMux: {
            uart4_send_string("AT+CIPMUX=0\r\n");
            break;
        }
        case SetUDP: {
            /* IP 远程端口 本地端口10086 */
            sprintf(SendBuffer, "AT+CIPSTART=\"UDP\",\"%s\",%s,10086,0", HOST, PORT);
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
            Send2EspData.header = 0xa5;
            uint8_t ptr = 0;
            for (ptr = 0; ptr < sizeof(Send2EspData_t); ptr++) {
                uart4_send_byte(*(head + ptr));
            }
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
    
    if (USART_GetFlagStatus(USART1, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        uint8_t data = USART_ReceiveData(USART1);
        /* 接到帧头后准备接收后续的数字 */
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
        raspberryOfflineCnt = 0;
    }
}

/**********************FUNCTION***********************
  * @brief: 处理应答数据，完成状态转换
  * @param: const char[] 收到的应答数据
  * @return: None
  * @note:
  *****************************************************/
static void parse_feedback_data(const char data[]) {
    if (strstr(data, "OK") != NULL) {
        switch (Esp8266SendStatus) {
        case SetWorkMode: {
            Esp8266SendStatus = SetRST;
            break;
        }
        case SetRST: {
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
        case Sending:
        case StopCipMode:
        default:
            break;
        }
    }
}

/* 是否正在接收Esp8266串口发来的数据 */
bool isRecvingFromEsp8266 = false;
/* 接收计时，2ms计一次，超过10次认为结束 */
uint16_t RecvingFromEsp8266Cnt = 0;
/* 缓冲区 */
static char Esp8266RecvBuffer[100] = {'\0'};
/* 通信缓冲区 */
static uint8_t buffer[sizeof(RecvfromEspData_t)] = {0};

void UART4_IRQHandler() {
    if (USART_GetFlagStatus(UART4, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        uint8_t data = USART_ReceiveData(UART4);
        
        if (Esp8266SendStatus == Sending) {
            static uint8_t recvCnt = 0;
            static bool recvflag = false;
            /* 发送模式下解析数据，判断0xa5作为开头，接收一定长度的数据后停止 */
            /* Esp8266可能会返回其它信息，但我们只要0xa5开头的 */
            if (data == 0xa5) {
                recvflag = true;
            }
            if (recvflag == true) {
                buffer[recvCnt++] = data;
                if (recvCnt >= sizeof(RecvfromEspData_t)) {
                    memcpy((uint8_t *)&RecvfromEspData, buffer, sizeof(RecvfromEspData_t));
                    recvCnt = 0;
                    recvflag = false;
                }
            }
            esp8266OfflineCnt = 0;
        } else {
            static uint8_t index = 0;
            /* 在非数据模式下发送，等待10ms后没有新的数据则认为接收结束 */
            isWaitFeedback = false;
            WaitFeedbackCnt = 0;
            if (isRecvingFromEsp8266 == true && index < 100) {
                RecvingFromEsp8266Cnt = 0;
                Esp8266RecvBuffer[index++] = data;
            } else {
                parse_feedback_data(Esp8266RecvBuffer);
                index = 0;
                memset(Esp8266RecvBuffer, '\0', 100);
            }
        }
    }
}
