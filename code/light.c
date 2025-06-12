#include "light.h"
#include "adc.h"
#include "stdio.h"
#include "oled.h"
uint16_t ADC_Sample = 0,ADC_Volt = 0;//ADC_ValueΪ����ֵ��ADC_VoltΪ��ѹֵ
uint8_t str[64];//����һ������ռ䣬���sprintf������
extern u8g2_t u8g2;

void Get_ADC_Sample()
{
	HAL_ADC_Start(&hadc1);//��ADCת��
	if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
	{
		ADC_Sample = HAL_ADC_GetValue(&hadc1);//���õ���ADC����ֵ�������ADC_Sample��
		ADC_Volt = ADC_Sample * 330/4096;//����ת������ѹΪ3.3V������ʱ12λ��������λС��
	}
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_courB10_tr);
    const char *done = "light��";
    u8g2_DrawStr(&u8g2, (128 - u8g2_GetStrWidth(&u8g2, done)) / 2, 32, done);
    u8g2_SendBuffer(&u8g2);

	HAL_ADC_Stop(&hadc1);//ֹͣADCת��
}
