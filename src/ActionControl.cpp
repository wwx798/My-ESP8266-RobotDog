#include <Arduino.h>
#include <Servo.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include "ActionControl.h"

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
int engine1 = 14; // 舵机引脚
int engine2 = 16; // 舵机引脚
int engine3 = 12; // 舵机引脚
int engine4 = 13; // 舵机引脚
int speed = 300;                 // 舵机转速
int runtime = 100;              // 运动延时**预留变量，用于控制动作连贯性
int ActionState=-1;
void Action(AsyncWebServer & server) {
    const char* paths[] = {"/front","/back","/left","/right",
                           "/toplefthand","/toprighthand","/sitdown","/lie","/sleep"};
  
    for (int i = 0; i < ACTION_NUM; i++) {
        server.on(paths[i], HTTP_GET, [i](AsyncWebServerRequest *request) {
            ActionState = i;
            request->send(200, "text/plain", "OK");
        });
    }
}
void moveServos(int s1, int s2, int s3, int s4 ,int delayMs = 100) {
    servo1.write(s1);
    servo2.write(s2);
    servo3.write(s3);
    servo4.write(s4);
    delay(delayMs);
}
void resetServos() {
    moveServos(ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER, 50);
}
void servo_Setup()
{
    servo1.attach(engine1, 500, 2500); // 引脚 D1，500µs=0度，2500µs=180度
    servo2.attach(engine2, 500, 2500); // 引脚 D1，500µs=0度，2500µs=180度
    servo3.attach(engine3, 500, 2500); // 引脚 D1，500µs=0度，2500µs=180度
    servo4.attach(engine4, 500, 2500); // 引脚 D1，500µs=0度，2500µs=180度
    resetServos();
}
// 前进
void front() {
    moveServos(ANGLE_CENTER, 140, 40, ANGLE_CENTER);
    moveServos(ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER);
    moveServos(140, ANGLE_CENTER, ANGLE_CENTER, 40);
    moveServos(ANGLE_CENTER, 40, 140, ANGLE_CENTER);
    resetServos();
}

// 后退
void back() {
    moveServos(ANGLE_CENTER, 40, 140, ANGLE_CENTER);
    moveServos(ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER);
    moveServos(40, ANGLE_CENTER, ANGLE_CENTER, 140);
    moveServos(ANGLE_CENTER, 140, 40, ANGLE_CENTER);
    resetServos();
}

// 左转
void left() {
    for (int i = 0; i < 4; i++) {
        moveServos(100, 60, 60, 100);
        moveServos(140, 40, 40, 140);
        moveServos(ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER);
        moveServos(80, 120, 120, 80);
        moveServos(ANGLE_CENTER, 140, 140, ANGLE_CENTER);
        resetServos();
    }
}

// 右转
void right() {
    for (int i = 0; i < 3; i++) {
        moveServos(80, 120, 120, 80);
        moveServos(40, 140, 140, 40);
        moveServos(ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER, ANGLE_CENTER);
        moveServos(100, 60, 60, 100);
        moveServos(ANGLE_CENTER, 40, 40, ANGLE_CENTER);
        resetServos();
    }
}

// 坐下
void sitdown() {
    moveServos(ANGLE_CENTER, 140, ANGLE_CENTER, 40, 3000);
    resetServos();
}

// 躺下
void lie() {
    moveServos(180, 0, 0, 180, 3000);
    resetServos();
}

// 抬左手
void toplefthand() {
    for (int i = 0; i < 3; i++) {
        servo3.write(0);
        delay(100);
        servo3.write(30);
        delay(100);
    }
    servo3.write(ANGLE_CENTER);
}

// 抬右手
void toprighthand() {
    for (int i = 0; i < 3; i++) {
        servo1.write(180);
        delay(100);
        servo1.write(150);
        delay(100);
    }
    servo1.write(ANGLE_CENTER);
}

// 睡眠姿势
void dosleep() {
    moveServos(0, 180, 180, 0);
}
void (*doAction[])() = {  front,back,left,right,toplefthand,toprighthand,sitdown,lie,dosleep};