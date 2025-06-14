#include "sound_light.h"
#include "adc.h"
#include "stdio.h"
#include "key.h"
#include "led.h"
#include "dma.h"
#include "menu.h" 
#include <string.h> 
#include <stdio.h>
/* 全局变量定义 */
volatile uint8_t dataReady = 0;
uint16_t adc_buffer[SAMPLE_BUFFER_SIZE * 2];
WaveformData sound_wave = {0};
float sound_V = 0, light_V = 0;
float sound_threshold = 1.0f, light_threshold = 0.0f;
char str[32];

// 初始化函数
void SoundLight_Init() 
{
    // 启动ADC DMA采集
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLE_BUFFER_SIZE*2);
}

float sound_max_peak = 0,light_max_peak = 0;  // 定义最大峰值变量
/* ADC数据处理 */
void Process_Dual_ADC_Data() 
{
    if(dataReady) {
        uint16_t sound_samples[SAMPLE_BUFFER_SIZE];
        uint16_t light_samples[SAMPLE_BUFFER_SIZE];
        
        // 分离双通道数据
        for(int i=0; i<SAMPLE_BUFFER_SIZE; i++) 
        {
            light_samples[i] = adc_buffer[i*2];     // 通道1(光敏)
            sound_samples[i] = adc_buffer[i*2+1];   // 通道9(声音)
        }

        // 滤波处理
        Filter_Data(light_samples, SAMPLE_BUFFER_SIZE);
        Filter_Data(sound_samples, SAMPLE_BUFFER_SIZE);
        
        // 计算电压值(3.3V参考)
        float current_light = Find_Max(light_samples, SAMPLE_BUFFER_SIZE) * 3.3f / 4096;
        float current_sound = Find_Max(sound_samples, SAMPLE_BUFFER_SIZE) * 3.3f / 4096;
        if(current_light > light_max_peak) 
        {
            light_max_peak = current_light;
        }
        if(current_sound > sound_max_peak) 
        {
            sound_max_peak = current_sound;
        }
        light_V = current_light;  // 更新当前值
        sound_V = current_sound;  // 更新当前值
       
        dataReady = 0;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLE_BUFFER_SIZE*2);
    }
}
#define WAVE_SMOOTHING 0.3f  // 平滑系数(0.1-0.3)

void Update_Waveform(float voltage) 
{
    static float filtered_voltage = 0;
    
    // 一阶低通滤波（核心防抖）
    filtered_voltage = filtered_voltage * (1-WAVE_SMOOTHING) + voltage * WAVE_SMOOTHING;
    
    // 动态放大变化部分（增强视觉效果）
    static float last_voltage = 0;
    float delta = fabsf(filtered_voltage - last_voltage) * 3.0f; // 变化量放大2倍
    float display_voltage = filtered_voltage + delta;
    
    // 归一化存储
    sound_wave.samples[sound_wave.index] = (uint8_t)(display_voltage * 255 / 3.3f);
    sound_wave.index = (sound_wave.index + 1) % WAVE_HISTORY;
    last_voltage = filtered_voltage;
}

void Draw_Waveform(u8g2_t *u8g2) 
{
    const uint8_t y_base = 58; // 基线位置
    uint8_t prev_y = y_base - (sound_wave.samples[sound_wave.index] * WAVE_HEIGHT / 256);
    
    // 绘制基线
    u8g2_DrawHLine(u8g2, 0, y_base, WAVE_HISTORY);
    
    // 绘制粗线波形（2像素宽）
    for(uint8_t i=1; i<WAVE_HISTORY; i++) 
    {
        uint8_t x = i;
        uint8_t curr_y = y_base - (sound_wave.samples[(sound_wave.index+i)%WAVE_HISTORY] * WAVE_HEIGHT / 256);
        
        // 限制范围
        curr_y = (curr_y < y_base-WAVE_HEIGHT) ? y_base-WAVE_HEIGHT : curr_y;
        
        // 绘制线段（加粗）
        u8g2_DrawLine(u8g2, x-1, prev_y, x, curr_y);
        u8g2_DrawPixel(u8g2, x, curr_y+1); // 下边缘像素
        
        prev_y = curr_y;
    }
}
// 根据阈值控制 LED
void sound_to_led() 
{
    static uint32_t trigger_time = 0;  // 记录触发时刻
    static uint8_t led_active = 0;    // LED状态标志
    
    if(sound_V >= sound_threshold) 
    {
        // 声音超过阈值时激活LED并记录时间
        if(!led_active) 
        {
            led_duty(0);           // 点亮LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // 持续触发时刷新计时（可选）
        trigger_time = HAL_GetTick(); 
    }
    else 
    {
        // 声音低于阈值时检查是否超时
        if(led_active && (HAL_GetTick() - trigger_time >= 5000)) 
        {
            led_duty(450);             // 5秒后关闭LED
            led_active = 0;
        }
    }
}
// 根据阈值控制 LED
void light_to_led()
{
    static uint32_t trigger_time = 0;  // 记录触发时刻
    static uint8_t led_active = 0;    // LED状态标志
    
    if(light_V <= light_threshold) 
    {
        // 声音超过阈值时激活LED并记录时间
        if(!led_active) 
        {
            led_duty(0);           // 点亮LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // 持续触发时刷新计时（可选）
        trigger_time = HAL_GetTick(); 
    }
    else 
    {
        // 声音低于阈值时检查是否超时
        if(led_active && (HAL_GetTick() - trigger_time >= 5000)) 
        {
            led_duty(450);             // 5秒后关闭LED
            led_active = 0;
        }
    }
}
/* 阈值调节 */
/* 阈值调节（直接调节模式） */
void adjust_sound_threshold() 
{  
    key=0;
    key_scan();
    Update_Waveform(sound_V);  // 更新波形数据

    u8g2_ClearBuffer(&u8g2);
    
    // 实时显示当前值和阈值
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    sprintf(str, "Sound:%.2fV", sound_V);
    u8g2_DrawStr(&u8g2, 0, 12, str);
    
    sprintf(str, "Set:%.2fV", sound_threshold);
    u8g2_DrawStr(&u8g2, 64, 12, str);
    
    sprintf(str, "Max:%.2fV", sound_max_peak);
    u8g2_DrawStr(&u8g2, 0, 24, str);

	printf("Sound:%.2fV", sound_V);
    // 直接按键调节（无需模式切换）
    if(key == KEY_UP) 
    {
        sound_threshold += 0.1f;
        if(sound_threshold > 3.3f) sound_threshold = 3.3f;
        key = KEY_NONE;
    }
    else if(key == KEY_DOWN) 
    {
        sound_threshold -= 0.1f;
        if(sound_threshold < 0.0f) sound_threshold = 0.0f;
        key = KEY_NONE;
    }

    Draw_Waveform(&u8g2);
    u8g2_SendBuffer(&u8g2);

    if(key == KEY_OK) current_mode = MODE_MENU;
    key=0;
}

void adjust_light_threshold()
{  
    key=0;
    key_scan();
    Update_Waveform(light_V);  // 更新波形数据

    u8g2_ClearBuffer(&u8g2);
    
    // 实时显示当前值和阈值
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    sprintf(str, "light:%.2fV", light_V);
    u8g2_DrawStr(&u8g2, 0, 12, str);
    
    sprintf(str, "Set:%.2fV", light_threshold);
    u8g2_DrawStr(&u8g2, 64, 12, str);
    
    sprintf(str, "Max:%.2fV", light_max_peak);
    u8g2_DrawStr(&u8g2, 0, 24, str);
    
    printf("Sound:%.2fV", sound_V);
    // 直接按键调节（无需模式切换）
    if(key == KEY_UP) 
    {
        light_threshold += 0.1f;
        if(light_threshold > 3.3f) light_threshold = 3.3f;
        key = KEY_NONE;
    }
    else if(key == KEY_DOWN) 
    {
        light_threshold -= 0.1f;
        if(light_threshold < 0.0f) light_threshold = 0.0f;
        key = KEY_NONE;
    }
    Draw_Waveform(&u8g2);
    u8g2_SendBuffer(&u8g2);

    if(key == KEY_OK) current_mode = MODE_MENU;
    key=0;
}

// 移动平均滤波
void Filter_Data(uint16_t *data, uint32_t length) 
{
    uint16_t temp[FILTER_WINDOW];
    for(uint32_t i=FILTER_WINDOW/2; i<length-FILTER_WINDOW/2; i++) 
    {
        uint32_t sum = 0;
        for(uint32_t j=0; j<FILTER_WINDOW; j++) 
        {
            temp[j] = data[i-FILTER_WINDOW/2+j];
            sum += temp[j];
        }
        data[i] = sum / FILTER_WINDOW;
    }
}


// 查找最大值
uint16_t Find_Max(uint16_t *data, uint32_t length)
{
    uint16_t max = 0;
    for(uint32_t i = 0; i < length; i++) 
    {
        if(data[i] > max) 
        {
            max = data[i];
        }
    }
    return max;
}

// DMA传输完成回调函数
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == hadc1.Instance) 
    {
        dataReady = 1;  // 设置数据准备标志
    }
}

