#include "schedule.h"

extern uint32_t SysTickCnt;
extern bool systemReady;
extern bool isEsp8266Offline;
extern bool isRaspberryOffline;
extern bool isRecvingFromEsp8266;
extern uint16_t RecvingFromEsp8266Cnt;

/* 树莓派离线计数 */
uint32_t raspberryOfflineCnt = 0;
uint32_t esp8266OfflineCnt = 0;

/**********************FUNCTION***********************
  * @brief: 500Hz频率，2ms执行一次，不要放太多代码
  * @note: 两个串口的离线检测，超过500ms没有收到数据即认为离线
  *****************************************************/
static void task_500Hz(void) {
    isRaspberryOffline = (raspberryOfflineCnt++ >= 250);
    isEsp8266Offline = (esp8266OfflineCnt++ >= 250);
    isRecvingFromEsp8266 = (RecvingFromEsp8266Cnt++ <= 10);
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
  * @note:
  *****************************************************/
static void task_100Hz_part1(void) {
}

/**********************FUNCTION***********************
  * @brief: 100Hz频率，10ms执行一次
  * @note:
  *****************************************************/
static void task_100Hz_part2(void) {
}


static void task_20Hz(void) {
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
    }
}
