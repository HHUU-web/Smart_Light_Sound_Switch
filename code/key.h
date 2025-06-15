#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#define KEY_NONE    0
#define KEY_LEFT    1
#define KEY_RIGHT   2
#define KEY_OK      3
#define KEY_SHORT   4
#define KEY_LONG    5
#define KEY_EXIT    6  // 新增退出按键功能值
extern uint8_t key;
extern uint8_t key_repeat_enable;  // 0: 禁止连按，1: 允许连按
void key_scan(void);

#endif