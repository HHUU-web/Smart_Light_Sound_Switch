#include "light.h"
#include "adc.h"
#include "stdio.h"
#include "oled.h"
#include "led.h"
#include "key.h"

double light_adc = 0, light_V = 0;  // ���� ADC ֵ�͵�ѹֵ
char str[64];                       // OLED ��ʾ�ַ���
float light_threshold = 3.0;        // Ĭ�Ϲ�����ֵ����λ��V��
extern u8g2_t u8g2;

// ��ȡ��ǰ����ֵ��ADC ��ȡ��
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
// ������ֵ���� LED
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

// �������ڹ�����ֵ
void adjust_light_threshold()
 {
    static uint8_t adjust_mode = 0;  // 0: δ�������ģʽ��1: �������ģʽ
	key_scan(); 

    if (key == KEY_OK) 
	{
        adjust_mode = !adjust_mode;  // �� OK ������/�˳�����ģʽ
        key = KEY_NONE;             // �������״̬
    }

    if (adjust_mode) 
	{
        // �ڵ���ģʽ�£�UP/DOWN ��������ֵ
        if (key == KEY_UP) {
            light_threshold += 1;  // ÿ������ 0.1V
            if (light_threshold > 50) light_threshold = 50;  // �������ֵ
            key = KEY_NONE;
        } else if (key == KEY_DOWN) {
            light_threshold -= 1;  // ÿ�μ��� 0.1V
            if (light_threshold < 4) light_threshold = 4;  // ������Сֵ
            key = KEY_NONE;
        }

        // ��ʾ��ǰ��ֵ������ģʽ��
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_pixzillav1_tf);
        sprintf(str, "Thr: %.1f V", light_threshold);
        u8g2_DrawStr(&u8g2, 0, 20, str);
        u8g2_SendBuffer(&u8g2);
    }
	 else 
	{
        // ����ģʽ����ʾ��ǰ����ֵ
        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_pixzillav1_tf);
        sprintf(str, "Light: %.1f V", light_V);
        u8g2_DrawStr(&u8g2, 0, 20, str);
        sprintf(str, "Thr: %.1f V", light_threshold);
        u8g2_DrawStr(&u8g2, 0, 40, str);
        u8g2_SendBuffer(&u8g2);
    }
}