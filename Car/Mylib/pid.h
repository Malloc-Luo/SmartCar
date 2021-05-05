#ifndef __PID_H__
#define __PID_H__

/**
 * PID���ݽṹ�壬ʵ��ֻ��Ҫ��PI���ƻ���P���ƾ���
 */
typedef struct {
    /* pid ϵ�� */
    float Kp;
    float Ki;
    float Kd;
    /* ����޷� */
    float resMax;
    /* �����޷� */
    float inteMax;
    
} PidParam_t;


typedef struct {
    /* Pid���� */
    PidParam_t param;
    /* ��ֵ */
    float err;
    /* ��һ�εĲ�ֵ */
    float lastErr;
    /* �趨ֵ�ͷ���ֵ */
    float setVal;
    float fbVal;
    /* ������ֵ */
    float inteErr;
    /* ���΢�� */
    float deltErr;
    /* ���ֵ */
    float out;
    
} PidDefStruct_t;

void load_pid_param(PidParam_t* pid, float kp, float ki, float kd, float rmax, float imax);
void init_pid_struct(PidDefStruct_t* pidstruct, PidParam_t* pid);
float pid_calc(float setval, float retval, PidDefStruct_t* pidstruct);

#endif
