#include "sound_light.h"
#include "adc.h"
#include "stdio.h"
#include "key.h"
#include "led.h"
#include "dma.h"
#include "menu.h" 
#include <string.h> 
#include <stdio.h>

extern uint8_t Game_Menu_Flag;
extern uint32_t led_last_time; 
extern u8g2_t u8g2;
extern uint8_t key;
extern int led_on;
extern int send_light, send_sound;

volatile uint16_t ADC_Buffer[2];  // DMA采集缓冲区（双路：声音+光强）
WaveformData sound_wave = {0};
float sound_threshold = 620;
uint32_t light_threshold = 500;
char str[32];


// 数据处理
#define LIGHT_FILTER_ALPHA 0.1f   // 光强滤波参数（越小越平稳）
#define SOUND_FILTER_ALPHA 0.2f   // 声音滤波参数（越小越平稳）
float sound_db = 0.0f;         // 声音分贝值
uint32_t light_lux = 0;        // 光强照度值 (0-65536Lx)
uint32_t light_sum = 0;
uint32_t voice_amp_sum = 0;
uint8_t sample_count = 0;
// 声强检测相关变量
uint8_t voice_counter = 0;
uint16_t voice_max = 0, voice_min = 4095;


// 初始化函数
void SoundLight_Init(void) 
{
    // 启动ADC校准和DMA采集
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Buffer, 2);
}
// ADC值转电压函数
float adc_to_voltage(uint16_t adc_val) {
    return adc_val * 3.3f / 4095.0f;
}
// 定时器5ms回调函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) // 5ms定时器
    {
        uint16_t light = ADC_Buffer[0];
        uint16_t voice = ADC_Buffer[1];

        // 1. 光强转换
        float lux = (float)(light) * 65536.0f / 4095.0f;
        if (lux > 65536.0f) lux = 65536.0f;

        light_sum += (uint32_t)lux;

        // 2. 声音处理（每25ms计算一次幅度）
        if (voice > voice_max) voice_max = voice;
        if (voice < voice_min) voice_min = voice;
        voice_counter++;

        if (voice_counter >= 5)  // 25ms 到了
        {
            voice_counter = 0;

            uint16_t amplitude = voice_max - voice_min;
            voice_amp_sum += amplitude;

            voice_max = 0;
            voice_min = 4095;

            sample_count++;
        }

        // 3. 每满100次输出一次（100 * 5ms = 500ms）
        if (sample_count >= 70 / 5)  // 20次 * 25ms = 500ms
        {
            light_lux = light_sum / 70;          // 100次平均光强
            sound_db = voice_amp_sum / 14;        // 20次平均声音幅度

            // 重置累加
            light_sum = 0;
            voice_amp_sum = 0;
            sample_count = 0;
        }
    }
}

#define WAVE_HEIGHT     60     // 波形显示高度(像素)
#define WAVE_HISTORY    128     // 波形历史点数(建议等于OLED宽度)
#define WAVE_TYPE_SOUND 0
#define WAVE_TYPE_LIGHT 1

uint8_t waveform_type = WAVE_TYPE_SOUND;  // 默认波形类型

void Update_Waveform_Raw(uint16_t raw_input)
{
    sound_wave.samples[sound_wave.index] = raw_input;
    sound_wave.index = (sound_wave.index + 1) % WAVE_HISTORY;
}


void Draw_Waveform(u8g2_t *u8g2)
{
    const uint8_t y_base = 58;
    uint16_t max_val = (waveform_type == WAVE_TYPE_LIGHT) ? 65535 : 4095;

    u8g2_DrawHLine(u8g2, 0, y_base, WAVE_HISTORY);

    uint8_t prev_y = y_base - (sound_wave.samples[sound_wave.index] * WAVE_HEIGHT / max_val);

    for(uint8_t i = 1; i < WAVE_HISTORY; i++)
    {
        uint8_t x = i;
        uint16_t raw = sound_wave.samples[(sound_wave.index + i) % WAVE_HISTORY];

        uint8_t curr_y = y_base - (raw * WAVE_HEIGHT / max_val);
        curr_y = (curr_y < y_base - WAVE_HEIGHT) ? (y_base - WAVE_HEIGHT) : curr_y;

        u8g2_DrawLine(u8g2, x - 1, prev_y, x, curr_y);
        u8g2_DrawPixel(u8g2, x, curr_y + 1);

        prev_y = curr_y;
    }
}

// 根据阈值控制 LED
void sound_to_led(void) 
{
    static uint32_t trigger_time = 0;  // 记录触发时刻
    static uint8_t led_active = 0;    // LED状态标志
    uint8_t last=0;
    if((sound_db >= sound_threshold))
    {
        // 声音超过阈值时激活LED并记录时间
        if(!led_active) 
        {
            led_duty(led_on);           // 点亮LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // 持续触发时刷新计时（可选）
        trigger_time = HAL_GetTick(); 

    }
    else 
    {
        // 声音低于阈值时检查是否超时
        if(led_active && (HAL_GetTick() - trigger_time >= led_last_time)) 
        {
            led_duty(100);             // 5秒后关闭LED
            led_active = 0;
        }
    }

}

void sound_up_led(void)
{
    // 设定一个最大幅度映射（避免过亮/过暗）
    float max_amplitude = 1000.0f;  // 根据实际最大 sound_db 调整
    float duty;

    if (sound_db > max_amplitude) sound_db = max_amplitude;

    // 映射公式：声音大 → 占空比小（越亮）；声音小 → 占空比大（越暗）
    duty = 100.0f - (sound_db / max_amplitude) * 100.0f;

    // 限制范围
    if (duty < 0) duty = 0;
    if (duty > 100) duty = 100;

    led_duty((int)duty);
}

// 根据阈值控制 LED
void light_to_led()
{
    static uint32_t trigger_time = 0;  // 记录触发时刻
    static uint8_t led_active = 0;    // LED状态标志
    
    if(light_lux <= light_threshold) 
    {
        // 声音超过阈值时激活LED并记录时间
        if(!led_active) 
        {
            led_duty(led_on);           // 点亮LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // 持续触发时刷新计时（可选）
        trigger_time = HAL_GetTick(); 
    }
    else 
    {
        // 声音低于阈值时检查是否超时
        if(led_active && (HAL_GetTick() - trigger_time >= led_last_time)) 
        {
            led_duty(100);             // 5秒后关闭LED
            led_active = 0;
        }
    }
}
/* 阈值调节 */
/* 阈值调节（直接调节模式） */
int sound_led_mode=0;//默认为声音控制亮度

void adjust_sound_threshold() 
{  
    key=0;
    key_scan();
    Game_Menu_Flag = key;
    waveform_type = WAVE_TYPE_SOUND;
    Update_Waveform_Raw((uint16_t)sound_db); 
    u8g2_ClearBuffer(&u8g2);
    
    // 实时显示当前值和阈值
    u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr );
    sprintf(str, "Sound:%.0f db", sound_db);
    u8g2_DrawStr(&u8g2, 0, 12, str);
    
    sprintf(str, "Set:%.0f db", sound_threshold);
    u8g2_DrawStr(&u8g2, 64, 12, str);
    
    if(send_sound)
    {
        printf("Sound:%.0f db\n", sound_db);//串口发送
    }
    // 直接按键调节
    if(key == KEY_LEFT) 
    {
        sound_threshold += 100;
        if(sound_threshold > 4095) sound_threshold = 4095;
        key = KEY_NONE;
    }
    else if(key == KEY_RIGHT) 
    {
        sound_threshold -= 100;
        if(sound_threshold < 0) sound_threshold = 0;
        key = KEY_NONE;
    }
    else if(key == KEY_OK)
    {
        sound_led_mode=!sound_led_mode;
    }
    if(sound_led_mode==0)
    {
        sound_up_led();
        sprintf(str, "OFF");
        u8g2_DrawStr(&u8g2,0,24,str);
    }
    else if(sound_led_mode==1)
    {
        led_duty(100);
        sound_to_led();
        sprintf(str, "ON");
        u8g2_DrawStr(&u8g2,0,24,str);
    }

    Draw_Waveform(&u8g2);
    u8g2_SendBuffer(&u8g2);

}

void adjust_light_threshold()
{  
        key=0;
        key_scan();
        Game_Menu_Flag = key;
        waveform_type = WAVE_TYPE_LIGHT;
        Update_Waveform_Raw(light_lux); 

        u8g2_ClearBuffer(&u8g2);
        
        // 实时显示当前值和阈值
        u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr );
        sprintf(str, "light:%d Lx", light_lux);
        u8g2_DrawStr(&u8g2, 0, 12, str);
        
        sprintf(str, "Set:%d Lx", light_threshold);
        u8g2_DrawStr(&u8g2, 64, 12, str);

        if(send_light)
        {
            printf("Light:%d Lx\n", light_lux);//串口发送
        }
        // 直接按键调节
        if(key == KEY_LEFT) 
        {
            light_threshold += 100;
            if(light_threshold > 65535) light_threshold = 65535;
            key = KEY_NONE;
        }
        else if(key == KEY_RIGHT) 
        {
            light_threshold -= 100;
            if(light_threshold < 0) light_threshold = 0;
            key = KEY_NONE;
        }

        Draw_Waveform(&u8g2);
        u8g2_SendBuffer(&u8g2);
    
    
}

void light_detect(void)
{
    while(1)
    {
        adjust_light_threshold();
        light_to_led();
        if(Game_Menu_Flag == KEY_EXIT)
        {
            u8g2_ClearBuffer(&u8g2);
            Show_Menu_Config();  
            break;
        }

    }
}

void sound_detect(void)
{
    while(1)
    {
        adjust_sound_threshold();

        if(Game_Menu_Flag == KEY_EXIT)
        {
            u8g2_ClearBuffer(&u8g2);
            Show_Menu_Config();  
            break;
        }

    }

}

void mix_detect(void)
{
    while(1)
    {
        key=0;
        key_scan();
        Game_Menu_Flag = key;

        waveform_type = WAVE_TYPE_LIGHT;
        Update_Waveform_Raw(light_lux); 

        u8g2_ClearBuffer(&u8g2);
    
        // 实时显示声音当前值和阈值
        u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr  );
        sprintf(str, "Sound:%.0f db", sound_db);
        u8g2_DrawStr(&u8g2, 0, 10, str);
        sprintf(str, "Set:%.0f db", sound_threshold);
        u8g2_DrawStr(&u8g2, 64, 10, str);

        // 实时显示光当前值和阈值
        sprintf(str, "light:%d Lx", light_lux);
        u8g2_DrawStr(&u8g2, 0, 30, str);
        sprintf(str, "Set:%d Lx", light_threshold);
        u8g2_DrawStr(&u8g2, 64, 30, str);

        Draw_Waveform(&u8g2);
        u8g2_SendBuffer(&u8g2);

        sound_to_led();
        light_to_led();
        if(Game_Menu_Flag == KEY_EXIT)
        {
            u8g2_ClearBuffer(&u8g2);
            Show_Menu_Config();  
            break;
        }
    }
}

// DMA传输完成回调函数
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == hadc1.Instance) 
    {

    }
}
