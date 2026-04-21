
#ifndef ACTIONCONTROL_H
#define ACTIONCONTROL_H

#include <Arduino.h>
#include <Servo.h>
#define ANGLE_CENTER  90
#define ACTION_NUM    9
// 声明外部变量
extern Servo servo1, servo2, servo3, servo4;
extern int engine1, engine2, engine3, engine4;
extern int speed, runtime, ActionState;

extern void (*doAction[])();

// 声明函数
void servo_Setup();
void front();
void back();
void left();
void right();
void sitdown();
void lie();
void toplefthand();
void toprighthand();
void dosleep();
void Action(AsyncWebServer & server);
#endif