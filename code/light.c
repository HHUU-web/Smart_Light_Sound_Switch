#include "light.h"
#include "adc.h"
#include "stdio.h"
#include "oled.h"
#include "led.h"
#include "key.h"

double light_adc = 0, light_V = 0;  // 光照 ADC 值和电压值
char str[64];                       // OLED 显示字符串
float light_threshold = 3.0;        // 默认光敏阈值（单位：V）
extern u8g2_t u8g2;

// 获取当前光照值（ADC 读取）
void Get_light()
{
	HAL_ADC_Start(&hadc1);//
	if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
	{
		light_adc = HAL_ADC_GetValue(&hadc1);//
		light_V = light_adc * 330/4096;//
	}


	HAL_ADC_Stop(&hadc1);//
}
// 根据阈值控制 LED
void light_to_led()
{
	Get_light();
	if(light_V <= light_threshold)
	{
		led_duty(0);
	}
	else
	{
		led_duty(450);
	}
}

// 按键调节光敏阈值
void adjust_light_threshold()
 {
    static uint8_t adjust_mode = 0;  // 0: 未进入调节模式，1: 进入调节模式
	key_scan(); 

    if (key == KEY_OK) 
	{
        adjust_mode = !adjust_mode;  // 按 OK 键进入/退出调节模式
        key = KEY_NONE;             // 清除按键状态
    }

    if (adjust_mode) 
	{
        // 在调节模式下，UP/DOWN 键调整阈值
        if (key == KEY_UP) {
            light_threshold += 1;  // 每次增加 0.1V
            if (light_threshold > 50) light_threshold = 50;  // 限制最大值
            key = KEY_NONE;
        } else if (key == KEY_DOWN) {
            light_threshold -= 1;  // 每次减少 0.1V
            if (light_threshold < 4) light_threshold = 4;  // 限制最小值
            key = KEY_NONE;
        }

        // 显示当前阈值（调节模式）
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_pixzillav1_tf);
        sprintf(str, "Thr: %.1f V", light_threshold);
        u8g2_DrawStr(&u8g2, 0, 20, str);
        u8g2_SendBuffer(&u8g2);
    }
	 else 
	{
        // 正常模式：显示当前光照值
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_pixzillav1_tf);
        sprintf(str, "Light: %.1f V", light_V);
        u8g2_DrawStr(&u8g2, 0, 20, str);
        sprintf(str, "Thr: %.1f V", light_threshold);
        u8g2_DrawStr(&u8g2, 0, 40, str);
        u8g2_SendBuffer(&u8g2);
    }
}