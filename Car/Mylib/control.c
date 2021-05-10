#include "control.h"
#include "motorDrive.h"
#include "main.h"

extern PwmOutStruct_t PwmOutStruct;
extern SpeedStruct_t MotorSpeed;
extern SpeedStruct_t SetMotorSpeed;
extern Mode_t ControlMode;

PidParam_t PidParam[4] = {{1.0, 0.0, 0.0, 1000, 1000},
                          {1.0, 0.0, 0.0, 1000, 1000},
                          {1.0, 0.0, 0.0, 1000, 1000},
                          {1.0, 0.0, 0.0, 1000, 1000}};

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

/*
 * 选择控制模式
 */
void control_mode_select() {
    if (isEsp8266Offline == true) {
        ControlMode = Mode_Pause;
    } else {
        ControlMode = RecvfromEspData.mode;
        if (isRaspberryOffline == true && RecvfromEspData.mode == Mode_Identify) {
            ControlMode = Mode_Pause;
        }
    }
}

void calc_speed(int16_t vx, int16_t vy, int16_t vr) {
    SetMotorSpeed.v1 = +vy + vx + vr;
    SetMotorSpeed.v2 = +vy - vx + vr;
    SetMotorSpeed.v3 = +vy + vx - vr;
    SetMotorSpeed.v4 = +vy - vx - vr;
}

void car_control() {
    control_mode_select();
    switch (ControlMode) {
        case Mode_Pause: {
            simple_control(0, 0, 0, 0);
            // set_speed(0, 0, 0, 0);
            break;
        }    
        case Mode_Remote: {
            calc_speed(RecvfromEspData.vx, RecvfromEspData.vy, RecvfromEspData.vr);
            simple_control(SetMotorSpeed.v1, SetMotorSpeed.v2, SetMotorSpeed.v3, SetMotorSpeed.v4);
            break;
        }
        case Mode_Identify: {
            switch (RecvfromRaspberryData.action) {
                case Action_Fore: {
                    //calc_speed(50, 0, 0);
                    simple_control(1, 1, 1, 1);
                    break;
                }
                case Action_Back: {
                    simple_control(-1, -1, -1, -1);
                    //calc_speed(-50, 0, 0);
                    break;
                }
                default: {
                    simple_control(0, 0, 0, 0);
                    //set_speed(0, 0, 0, 0);
                    break;
                }
            }
        }
        default:
            break;
    }
    //pid_control();
    //set_speed(PwmOutStruct.pwm1, PwmOutStruct.pwm2, PwmOutStruct.pwm3, PwmOutStruct.pwm4);
}
