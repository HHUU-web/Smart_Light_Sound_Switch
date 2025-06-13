#include "light.h"
#include "adc.h"
#include "stdio.h"
#include "oled.h"
double light_adc = 0,light_V = 0;//
char str[64];//
extern u8g2_t u8g2;

void Get_light()
{
	HAL_ADC_Start(&hadc1);//
	if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
	{
		light_adc = HAL_ADC_GetValue(&hadc1);//
		light_V = light_adc * 330/4096;//
	}
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_courB10_tr);
    sprintf(str, "light: %.1f V", light_V);
    u8g2_DrawStr(&u8g2, 0, 30, str);
    u8g2_SendBuffer(&u8g2);

	HAL_ADC_Stop(&hadc1);//
}
