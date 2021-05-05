#ifndef __PID_H__
#define __PID_H__

/**
 * PID数据结构体，实际只需要用PI控制或者P控制就行
 */
typedef struct {
    /* pid 系数 */
    float Kp;
    float Ki;
    float Kd;
    /* 输出限幅 */
    float resMax;
    /* 积分限幅 */
    float inteMax;
    
} PidParam_t;


typedef struct {
    /* Pid参数 */
    PidParam_t param;
    /* 差值 */
    float err;
    /* 上一次的差值 */
    float lastErr;
    /* 设定值和返回值 */
    float setVal;
    float fbVal;
    /* 误差积分值 */
    float inteErr;
    /* 误差微分 */
    float deltErr;
    /* 输出值 */
    float out;
    
} PidDefStruct_t;

void load_pid_param(PidParam_t* pid, float kp, float ki, float kd, float rmax, float imax);
void init_pid_struct(PidDefStruct_t* pidstruct, PidParam_t* pid);
float pid_calc(float setval, float retval, PidDefStruct_t* pidstruct);

#endif
