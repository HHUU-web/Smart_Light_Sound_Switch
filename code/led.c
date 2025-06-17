#include "led.h"
#include "tim.h"
#include "key.h"
#include "u8g2.h"
#include <stdio.h>
#include <string.h> 
#include "menu.h" 

int led_on=0;//led最亮为0，关为100
uint32_t led_last_time = 5000;
int time=5; 
char led[32];
extern u8g2_t u8g2; 
extern uint8_t Game_Menu_Flag;
void led_duty(int led_pwm)
{
	__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,led_pwm);
}

void bright_adjust(void)
{
	while(1)
	{
		key=0;
		key_scan();
		Game_Menu_Flag = key;
		u8g2_ClearBuffer(&u8g2);
		if(key == KEY_LEFT) 
		{
			led_on -= 10;
			if(led_on <= 0) led_on = 0;
			key = KEY_NONE;
		}
		else if(key == KEY_RIGHT) 
		{
			led_on += 10;
			if(led_on >= 100) led_on = 100;
			key = KEY_NONE;
		}
		u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr );
		u8g2_DrawStr(&u8g2,52,10,"Blib adjustment");
		u8g2_DrawLine(&u8g2, 1, 13, 128, 13);
		sprintf(led, "Brightness:%d%%", led_on);
		u8g2_DrawStr(&u8g2,3,24,led);
		led_duty(led_on);
		if(Game_Menu_Flag == KEY_EXIT)
        {
            u8g2_ClearBuffer(&u8g2);
            Blib_adjustment_Menu();  
            break;
        }
		u8g2_SendBuffer(&u8g2);
	}
}

void time_adjust(void)
{
	while(1)
	{
		key=0;
		key_scan();
		Game_Menu_Flag = key;
		u8g2_ClearBuffer(&u8g2);
		if(key == KEY_LEFT) 
		{
			time -= 1;
			if(time <= 0) time = 0;
			key = KEY_NONE;
		}
		else if(key == KEY_RIGHT) 
		{
			time += 1;
			if(time >= 100) time = 100;
			key = KEY_NONE;
		}
		led_last_time=time*1000;
		u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr );
		u8g2_DrawStr(&u8g2,52,10,"Blib adjustment");
		u8g2_DrawLine(&u8g2, 1, 13, 128, 13);
		sprintf(led, "Time:%ds", time);
		u8g2_DrawStr(&u8g2,3,36,led);
		if(Game_Menu_Flag == KEY_EXIT)
        {
            u8g2_ClearBuffer(&u8g2);
            Blib_adjustment_Menu();  
            break;
        }
		u8g2_SendBuffer(&u8g2);
	}
}

