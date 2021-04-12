#include "schedule.h"

extern uint32_t SysTickCnt;
extern bool systemReady;

/**********************FUNCTION***********************
  * @brief: 500Hz频率，2ms执行一次，不要放太多代码
  * @note:
  *****************************************************/
void task_500Hz(void) {
}

/**********************FUNCTION***********************
  * @brief: 200Hz频率，5ms执行一次
  * @note:
  *****************************************************/
void task_200Hz_part1(void) {
}

/**********************FUNCTION***********************
  * @brief: 200Hz频率，5ms执行一次
  * @note:
  *****************************************************/
void task_200Hz_part2(void) {
}

/**********************FUNCTION***********************
  * @brief: 100Hz频率，10ms执行一次
  * @note:
  *****************************************************/
void task_100Hz_part1(void) {
}

/**********************FUNCTION***********************
  * @brief: 100Hz频率，10ms执行一次
  * @note:
  *****************************************************/
void task_100Hz_part2(void) {
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
    }
}
