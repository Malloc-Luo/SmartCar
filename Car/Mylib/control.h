#ifndef __CONTROL_H__
#define __CONTROL_H__
#include "motorDrive.h"
#include "timer.h"
#include "pid.h"


                            
extern void pid_init();
extern void pid_control();
extern void control_mode_select();
extern void car_control();

#endif
