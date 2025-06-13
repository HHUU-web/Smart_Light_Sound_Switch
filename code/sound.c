#include "sound.h"
#include "adc.h"
#include "stdio.h"
#include "oled.h"
double sound_adc = 0,sound_V = 0;//
char str1[64];//
extern u8g2_t u8g2;

void Get_sound()
{
	HAL_ADC_Start(&hadc2);//
	if(HAL_ADC_PollForConversion(&hadc2,10) == HAL_OK)
	{
		sound_adc = HAL_ADC_GetValue(&hadc2);//
		sound_V = sound_adc * 330/4096;//
	}
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_courB10_tr);
    sprintf(str1, "sound: %.1f V", sound_V);
    u8g2_DrawStr(&u8g2, 0, 60, str1);
    u8g2_SendBuffer(&u8g2);

	HAL_ADC_Stop(&hadc2);//
}


