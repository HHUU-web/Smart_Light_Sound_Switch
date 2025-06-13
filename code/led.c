#include "led.h"
#include "tim.h"

uint16_t led_pwm=0;

void led_duty()
{
	__HAL_TIM_SetCompare(&htim3,TIM_CHANNEL_1,led_pwm);
}

void led_fun()//µ∆≈›‘À––
{
	while (led_pwm< 500)
	{
	  led_pwm++;
	  led_duty();
	  HAL_Delay(1);
	}
	while (led_pwm)
	{
	  led_pwm--;
	  led_duty();
	  HAL_Delay(1);
	}
	  
}