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
  * @brief: ����pid����
  * @param: [PidParam_t* pid]     PID�ṹ��ָ��
  *         [float kp]            Kp����
  *         [float ki]            Ki����
  *         [float kd]            Kd����
  *         [float rmax]          ����޷�
  *         [float imax]          ��������޷�
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
  * @brief: PID����������
  * @param: [float setval]: �趨ֵ
  *         [float retval]: ʵ��ֵ
  *         [PidDefStruct_t* pidstruct]: PID�ṹ��
  * @return: PID���ֵ
  * @note:
  *****************************************************/
float pid_calc(float setval, float retval, PidDefStruct_t* pidstruct) {
    float result = 0.0f;
    
    pidstruct->setVal = setval;
    pidstruct->fbVal = retval;
    /* ������� */
    pidstruct->err = pidstruct->setVal - pidstruct->fbVal;
    /* �������ۼ� */
    pidstruct->inteErr += pidstruct->err;
    /* �Ի����޷� */
    if (pidstruct->param.inteMax != 0.0f) {
        pidstruct->inteErr = ZOOM_f32(pidstruct->inteErr, -ABS_f32(pidstruct->param.inteMax), ABS_f32(pidstruct->param.inteMax));
    }
    /* ���΢�ֲ�ֵ */
    pidstruct->deltErr = pidstruct->err - pidstruct->lastErr;
    /* ������� */
    pidstruct->lastErr = pidstruct->err;
    
    /* �������ֵ */
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
