#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#define KEY_NONE    0
#define KEY_UP      1
#define KEY_DOWN    2
#define KEY_OK      3
#define KEY_SHORT   4
#define KEY_LONG    5
extern uint8_t key;
extern uint8_t key_repeat_enable;  // 0: 禁止连按，1: 允许连按


void key_scan(void);

#endif