/**********************FILE***************************
  * @filename: Mylib/timer.h
  * @author: yuqi li
  * @data: 04-13-2021
  * @brief: 编码器计数器的初始化
  * @history:
  *
 *****************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__
#include "stm32f10x.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    MotorA, MotorB, MotorC, MotorD,
} Motor_t;

/* 小车的速度 */
typedef struct {
    int16_t v1;
    int16_t v2;
    int16_t v3;
    int16_t v4;
} SpeedStruct_t;
extern SpeedStruct_t MotorSpeed;
extern SpeedStruct_t SetMotorSpeed;

void encoder_init(void);

#endif
