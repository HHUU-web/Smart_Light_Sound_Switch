/* menu.h */
#ifndef __MENU_H
#define __MENU_H

#include "u8g2.h"
#include "main.h"

#define LOW_SPEED			1
#define MID_SPEED			3
#define HIGH_SPEED			4
#define	FAST_SPEED			6
#define	SO_FAST_SPEED		8

typedef enum {
	low_speed = LOW_SPEED, 
	mid_speed = MID_SPEED,
	high_speed = HIGH_SPEED,
	fast_speed = FAST_SPEED,
	so_fast_speed = SO_FAST_SPEED
} Speed_ENUM;



void ui_run(char* a ,char* a_trg,int b);
void ui_right_one_Picture(int16_t* a, int b);
void ui_left_one_Picture(int16_t* a, int b );
void Show_Menu(Speed_ENUM Speed_choose);
void Show_Menu_Config(void);
void Blib_adjustment_Menu(void);
void To_Blib_adjustment_Menu(void);//菜单→光强检测菜单过渡动画
void Blib_adjustment_To_Menu_Display(void);//光强检测菜单→菜单过渡动画
void Serial_Adjustment_Menu(void); 
void To_Serial_Adjustment_Menu(void);//菜单→串口模式菜单过渡动画
void Serial_Adjustment_To_Menu_Display(void);//串口模式菜单→菜单过渡动画



#endif /* __MENU_H */