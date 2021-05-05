#include "schedule.h"
#include "timer.h"
#include "motorDrive.h"

extern volatile uint32_t SysTickCnt;
extern bool systemReady;
extern bool isEsp8266Offline;
extern bool isRaspberryOffline;
extern bool isWaitFeedback;
extern uint16_t WaitFeedbackCnt;
extern SpeedStruct_t MotorSpeed;
extern PwmOutStruct_t PwmOutStruct;
extern void send_esp8266_data(void);
extern void send_raspberry_data(void);
extern int32_t get_encoder(Motor_t motor);
extern void set_speed(int16_t v1, int16_t v2, int16_t v3, int16_t v4);
extern void pid_control();

/* 树莓派离线计数 */
uint32_t raspberryOfflineCnt = 0;
uint32_t esp8266OfflineCnt = 0;
int v1 = 0, v2 = 0, v3 = 0, v4 = 0;

/**********************FUNCTION***********************
  * @brief: 500Hz频率，2ms执行一次，不要放太多代码
  * @note: 两个串口的离线检测，超过500ms没有收到数据即认为离线
  *****************************************************/
static void task_500Hz(void) {
    isRaspberryOffline = (raspberryOfflineCnt++ >= 250);
    isEsp8266Offline = (esp8266OfflineCnt++ >= 250);
    /* 等待接收计数 */
    if (WaitFeedbackCnt++ >= 100) {
        isWaitFeedback = false;
        WaitFeedbackCnt = 0;
    }
}

/**********************FUNCTION***********************
  * @brief: 200Hz频率，5ms执行一次
  * @note: 
  *****************************************************/
static void task_200Hz_part1(void) {
}

/**********************FUNCTION***********************
  * @brief: 200Hz频率，5ms执行一次
  * @note:
  *****************************************************/
static void task_200Hz_part2(void) {
}

/**********************FUNCTION***********************
  * @brief: 100Hz频率，10ms执行一次
  * @note: 获取小车的速度，保存在结构体MotorSpeed中
  *****************************************************/
static void task_100Hz_part1(void) {
    MotorSpeed.v1 = get_encoder(MotorA);
    MotorSpeed.v2 = -get_encoder(MotorB);
    MotorSpeed.v3 = get_encoder(MotorC);
    MotorSpeed.v4 = get_encoder(MotorD);
    pid_control();
    set_speed(PwmOutStruct.pwm1, PwmOutStruct.pwm2, PwmOutStruct.pwm3, PwmOutStruct.pwm4);
}

/**********************FUNCTION***********************
  * @brief: 100Hz频率，10ms执行一次
  * @note:
  *****************************************************/
static void task_100Hz_part2(void) {
}


static void task_20Hz(void) {
    send_esp8266_data();
}


static void task_1Hz(void) {
}

/**********************FUNCTION***********************
  * @brief: 任务调度函数
  * @note: 完成硬件初始化之后才能开始调度
  *****************************************************/
void shcedule(void) {
    if (systemReady) {
        if (SysTickCnt % 2 == 0) {
            task_500Hz();
        }
        if (SysTickCnt % 5 == 0) {
            task_200Hz_part1();
        } else if (SysTickCnt % 5 == 2) {
            task_200Hz_part2();
        }
        
        if (SysTickCnt % 10 == 1) {
            task_100Hz_part1();
        } else if (SysTickCnt % 10 == 3) {
            task_100Hz_part2();
        }
        
        if (SysTickCnt % 50 == 7) {
            task_20Hz();
        }
        
        if (SysTickCnt % 1000 == 1) {
            task_1Hz();
        }
    }
}
