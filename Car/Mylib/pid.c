#include "pid.h"
#include <stdlib.h>
#include <stdint.h>

static inline float ABS_f32(float v) {
    return v > 0 ? v : -v;
}

static inline float ZOOM_f32(float v, float min, float max) {
    return v <= min ? min : (v >= max ? max : v);
}

static inline int32_t ZOOM_i32(int32_t v, int32_t min, int32_t max) {
    return v <= min ? min : (v >= max ? max : v);
}

/**********************FUNCTION***********************
  * @brief: 加载pid参数
  * @param: [PidParam_t* pid]     PID结构体指针
  *         [float kp]            Kp参数
  *         [float ki]            Ki参数
  *         [float kd]            Kd参数
  *         [float rmax]          输出限幅
  *         [float imax]          积分误差限幅
  * @return: None
  * @note:
  *****************************************************/
void load_pid_param(PidParam_t* pid, float kp, float ki, float kd, float rmax, float imax) {
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->resMax = rmax;
    pid->inteMax = imax;
}


void init_pid_struct(PidDefStruct_t* pidstruct, PidParam_t* pid) {
    memset(pidstruct, 0, sizeof(PidDefStruct_t));
    pidstruct->param = *pid;
}

/**********************FUNCTION***********************
  * @brief: PID控制器运算
  * @param: [float setval]: 设定值
  *         [float retval]: 实际值
  *         [PidDefStruct_t* pidstruct]: PID结构体
  * @return: PID输出值
  * @note:
  *****************************************************/
float pid_calc(float setval, float retval, PidDefStruct_t* pidstruct) {
    float result = 0.0f;
    
    pidstruct->setVal = setval;
    pidstruct->fbVal = retval;
    /* 计算误差 */
    pidstruct->err = pidstruct->setVal - pidstruct->fbVal;
    /* 误差积分累计 */
    pidstruct->inteErr += pidstruct->err;
    /* 对积分限幅 */
    if (pidstruct->param.inteMax != 0.0f) {
        pidstruct->inteErr = ZOOM_f32(pidstruct->inteErr, -ABS_f32(pidstruct->param.inteMax), ABS_f32(pidstruct->param.inteMax));
    }
    /* 误差微分差值 */
    pidstruct->deltErr = pidstruct->err - pidstruct->lastErr;
    /* 更新误差 */
    pidstruct->lastErr = pidstruct->err;
    
    /* 计算输出值 */
    result = pidstruct->param.Kp * pidstruct->err + \
                    pidstruct->param.Ki * pidstruct->inteErr + \
                    pidstruct->param.Kd * pidstruct->deltErr;
    
    if (pidstruct->param.resMax != 0.0f) {
        pidstruct->out = ZOOM_f32(result, -ABS_f32(pidstruct->param.resMax), ABS_f32(pidstruct->param.resMax));
    } else {
        pidstruct->out = result;
    }
    
    return pidstruct->out;
}
