#include "control.h"

extern PwmOutStruct_t PwmOutStruct;
extern SpeedStruct_t MotorSpeed;
extern SpeedStruct_t SetMotorSpeed;

PidParam_t PidParam[4] = {{0.0, 0.0, 0.0, 1000, 1000},
                          {0.0, 0.0, 0.0, 1000, 1000},
                          {0.0, 0.0, 0.0, 1000, 1000},
                          {0.0, 0.0, 0.0, 1000, 1000}};

PidDefStruct_t PidStruct[4];

void pid_init() {
    for (uint8_t i = 0; i < 4; i++) {
        init_pid_struct(PidStruct + i, PidParam + i);
    }
}

void pid_control() {
    PwmOutStruct.pwm1 = pid_calc(SetMotorSpeed.v1, MotorSpeed.v1, &PidStruct[0]);
    PwmOutStruct.pwm2 = pid_calc(SetMotorSpeed.v2, MotorSpeed.v2, &PidStruct[1]);
    PwmOutStruct.pwm3 = pid_calc(SetMotorSpeed.v3, MotorSpeed.v3, &PidStruct[2]);
    PwmOutStruct.pwm4 = pid_calc(SetMotorSpeed.v4, MotorSpeed.v4, &PidStruct[3]);
}
