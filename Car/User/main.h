#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f10x.h"

/* 系统时钟计数，1ms计一次 */
extern volatile uint32_t SysTickCnt;
/* 系统硬件初始化完成 */
extern bool systemReady;

/* 三种模式 */
typedef enum {
    Mode_Remote,  /* 遥控模式 */
    Mode_Identify,/* 识别模式 */
    Mode_Pause    /* 停止模式 */
} Mode_t;

/* 规定动作 */
typedef enum {
    Action_Fore,
    Action_Back,
    Action_Left,
    Action_Right,
    Action_Rotate,
    Action_Pause
} Action_t;

/***
  * STM32与ESP8266通信
  * 串口4，波特率115200，1停止位，无校验
 */

/* 发送到Esp8266的信息，字节对齐1，长度9字节*/
typedef struct __attribute__((packed)) {
    /* 帧头0x50 */
    uint8_t header;
    /* 四个轮子的速度（编码器读到的）*/
    int16_t v1;
    int16_t v2;
    int16_t v3;
    int16_t v4;
} Send2EspData_t;

typedef struct __attribute__((packed)) {
    /* 帧头0x50 */
    uint8_t header;
    /* 上位机是否在线 */
    uint8_t isMasterOnline;
    /* 控制模式 */
    Mode_t mode;
    /* 三个速度 */
    int16_t vx;
    int16_t vy;
    int16_t vr;
} RecvfromEspData_t;


/***
  * STM32与树莓派通信，使用串口1，
  * 波特率115200，1个停止位，无校验
 */
typedef struct __attribute__((packed)) {
    /* 小车当前的控制模式 */
    Mode_t mode;
} Send2RaspberryData_t;


typedef struct __attribute__((packed)) {
    /* 帧头 */
    uint8_t header;
    /* 要求的动作 */
    Action_t action;
    /* 速度 */
    int16_t vx;
    int16_t vy;
    int16_t vr;
} RecvfromRaspberryData_t;

/* 小车当前的控制模式 */
extern Mode_t ControlMode;
/* 发送到Esp8266的数据 */
extern Send2EspData_t Send2EspData;
/* 接收自Esp8266的数据 */
extern RecvfromEspData_t RecvfromEspData;
/* 发送到树莓派的数据 */
extern Send2RaspberryData_t Send2RaspberryData;
/* 接收自树莓派的数据 */
extern RecvfromRaspberryData_t RecvfromRaspberryData;

/* Esp8266是否离线，默认为是 */
extern bool isEsp8266Offline;
/* Raspberry pi是否离线，默认为是 */
extern bool isRaspberryOffline;

#endif
