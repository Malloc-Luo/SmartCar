#include "motorDrive.h"
#include "stm32f10x.h"
#include "timer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

PwmOutStruct_t PwmOutStruct = {0, 0, 0, 0};

static void driction_control_init() {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

#define PWM_MODE 0
static void pwm_init() {

    #if PWM_MODE
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 7199;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
    
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    
	TIM_OC2Init(TIM8, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    
	TIM_OC3Init(TIM8, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    
	TIM_OC4Init(TIM8, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);

	TIM_ARRPreloadConfig(TIM8, ENABLE);
	TIM_Cmd(TIM8, ENABLE);
    #else
    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    #endif
}

void motor_driver_init() {
    driction_control_init();
    pwm_init();
}

/**********************FUNCTION***********************
  * @brief: 设置电机PWM输出
  * @param: v1 v2 v3 v4 依次为 A B C D四个电机的设定值
  * @return: None
  * @note:
  *****************************************************/
void set_speed(int16_t v1, int16_t v2, int16_t v3, int16_t v4) {
    if (v1 > 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_4);
        TIM8->CCR1 = v1;
    } else {
        GPIO_SetBits(GPIOC, GPIO_Pin_4);
        TIM8->CCR1 = 7200 + v1;
    }
    
    if (v2 > 0) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        TIM8->CCR2 = v2;
    } else {
        GPIO_SetBits(GPIOB, GPIO_Pin_0);
        TIM8->CCR2 = 7200 + v2;
    }
    
    if (v3 > 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_5);
        TIM8->CCR3 = v3;
    } else {
        GPIO_SetBits(GPIOC, GPIO_Pin_5);
        TIM8->CCR3 = 7200 + v3;
    }
    
    if (v4 > 0) {
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
        TIM8->CCR4 = 7200 - v4;
    } else {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        TIM8->CCR4 = -v4;
    }
}


void simple_control(int8_t v1, int8_t v2, int8_t v3, int8_t v4) {
    if (v3 > 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_4);
        GPIO_SetBits(GPIOC, GPIO_Pin_8);
    } else if (v3 < 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_8);
        GPIO_SetBits(GPIOC, GPIO_Pin_4);
    } else {
        GPIO_ResetBits(GPIOC, GPIO_Pin_8);
        GPIO_ResetBits(GPIOC, GPIO_Pin_4);
    }
    
    if (v2 > 0) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
        GPIO_SetBits(GPIOC, GPIO_Pin_7);
    } else if (v2 < 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
        GPIO_SetBits(GPIOB, GPIO_Pin_0);
    } else {
        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
        GPIO_ResetBits(GPIOB, GPIO_Pin_0);
    }
    
    if (v1 > 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_5);
        GPIO_SetBits(GPIOC, GPIO_Pin_6);
    } else if (v1 < 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);
        GPIO_SetBits(GPIOC, GPIO_Pin_5);
    } else {
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);
        GPIO_ResetBits(GPIOC, GPIO_Pin_5);
    }
    
    if (v4 > 0) {
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
        GPIO_SetBits(GPIOB, GPIO_Pin_1);
    } else if (v4 < 0) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
        GPIO_SetBits(GPIOC, GPIO_Pin_9);
    } else {
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
        GPIO_ResetBits(GPIOB, GPIO_Pin_1);
    }
}

