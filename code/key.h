#ifndef __KEY_H
#define __KEY_H

#include "main.h"

extern uint8_t key;
extern uint8_t key_repeat_enable;  // 0: 禁止连按，1: 允许连按
void key_scan(void);

#endif