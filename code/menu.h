#ifndef __MENU_H
#define __MENU_H

#include "main.h"
#include "oled.h"

// 添加以下定义
typedef enum {
    MODE_LIGHT = 0,
    MODE_SOUND,
    MODE_COMBINED,
    MODE_MENU  // 菜单模式
} DetectMode;

extern DetectMode current_mode;

// 声明菜单函数
uint8_t Show_MainMenu(u8g2_t *u8g2); 
void menu(void);
#endif