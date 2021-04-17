#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f10x.h"

/* 系统时钟计数 */
extern uint32_t SysTickCnt;
/* 系统硬件初始化完成 */
extern bool systemReady;

#ifndef __bool_true_false_are_defined
/* typedef bool */
typedef enum {
    false = 0, true = 1
} bool;

#endif 

/* 三种模式 */
typedef unsigned char Mode;
const uint8_t Mode_Remote = 0x01;
const uint8_t Mode_Track = 0x02;
const uint8_t Mode_Identify = 0x04;

/***
  * STM32与ESP8266通信
  * 串口4，波特率9600，1停止位，奇校验
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
    Mode mode;
    /* 三个速度 */
    int16_t vx;
    int16_t vy;
    int16_t vr;
} RecvfromEspData_t;


/***
  * STM32与树莓派通信，使用串口1，
  * 波特率9600，1个停止位，奇校验
 */
typedef struct __attribute__((packed)) {
    /* 小车当前的控制模式 */
    Mode mode;
} Send2RaspberryData_t;

typedef unsigned char Action;
const uint8_t Action_Fore = 0x01;
const uint8_t Action_Back = 0x02;
const uint8_t Action_Left = 0x04;
const uint8_t Action_Right = 0x08;

const uint8_t Action_Rotate = 0x0f;
typedef struct __attribute__((packed)) {
    /* 帧头 */
    uint8_t header;
    /* 要求的动作 */
    Action action;
    /* 速度 */
    int16_t vx;
    int16_t vy;
    int16_t vr;
} RecvfromRaspberryData_t;

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
