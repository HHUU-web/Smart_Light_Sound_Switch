#include "led.h"
#include "tim.h"

//uint16_t led_pwm=0;

void led_duty(uint16_t led_pwm)
{
	__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,led_pwm);
}

void led_huxi()//ºôÎüµÆ
{
	uint16_t pwm=0;
	while (pwm< 500)
	{
	  pwm++;
	  led_duty(pwm);
	  HAL_Delay(1);
	}
	while (pwm)
	{
	  pwm--;
	  led_duty(pwm);
	  HAL_Delay(1);
	}
	  
}