#include "light.h"
#include "adc.h"
#include "stdio.h"
#include "oled.h"
uint16_t ADC_Sample = 0,ADC_Volt = 0;//ADC_Value为采样值，ADC_Volt为电压值
uint8_t str[64];//给定一个数组空间，存放sprintf的内容
extern u8g2_t u8g2;

void Get_ADC_Sample()
{
	HAL_ADC_Start(&hadc1);//打开ADC转换
	if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
	{
		ADC_Sample = HAL_ADC_GetValue(&hadc1);//将得到的ADC采样值放入变量ADC_Sample中
		ADC_Volt = ADC_Sample * 330/4096;//数据转换，电压为3.3V，数据时12位，保留两位小数
	}
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_courB10_tr);
    const char *done = "light：";
    u8g2_DrawStr(&u8g2, (128 - u8g2_GetStrWidth(&u8g2, done)) / 2, 32, done);
    u8g2_SendBuffer(&u8g2);

	HAL_ADC_Stop(&hadc1);//停止ADC转换
}
