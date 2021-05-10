/**********************FILE***************************
  * @filename: Mylib/motorDrive.h
  * @author: yuqi li
  * @data: 04-13-2021
  * @brief: 电机驱动，DAC信号；电机速度获取
  * @history:
  *
  *****************************************************/
#ifndef __MOTOR_DRIVE_H__
#define __MOTOR_DRIVE_H__
#include <stdint.h>
#include "timer.h"

typedef struct {
    int16_t pwm1;
    int16_t pwm2;
    int16_t pwm3;
    int16_t pwm4;
} PwmOutStruct_t;

extern PwmOutStruct_t PwmOutStruct;

extern void motor_driver_init();
extern void set_speed(int16_t v1, int16_t v2, int16_t v3, int16_t v4);
extern void set_motor_pwm(Motor_t, int16_t);
extern void simple_control(int8_t v1, int8_t v2, int8_t v3, int8_t v4);

#endif
