#ifndef __MENU_H
#define __MENU_H

#include "main.h"
#include "oled.h"

// ������¶���
typedef enum {
    MODE_LIGHT = 0,
    MODE_SOUND,
    MODE_COMBINED,
    MODE_MENU  // �˵�ģʽ
} DetectMode;

extern DetectMode current_mode;

// �����˵�����
uint8_t Show_MainMenu(u8g2_t *u8g2); 
void menu(void);
#endif