#include "sound_light.h"
#include "adc.h"
#include "stdio.h"
#include "key.h"
#include "led.h"
#include "dma.h"
#include "menu.h" 
#include <string.h> 
#include <stdio.h>
/* ȫ�ֱ������� */
volatile uint8_t dataReady = 0;
uint16_t adc_buffer[SAMPLE_BUFFER_SIZE * 2];
WaveformData sound_wave = {0};
float sound_V = 0, light_V = 0;
float sound_threshold = 1.0f, light_threshold = 0.0f;
char str[32];

// ��ʼ������
void SoundLight_Init() 
{
    // ����ADC DMA�ɼ�
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLE_BUFFER_SIZE*2);
}

float sound_max_peak = 0,light_max_peak = 0;  // ��������ֵ����
/* ADC���ݴ��� */
void Process_Dual_ADC_Data() 
{
    if(dataReady) {
        uint16_t sound_samples[SAMPLE_BUFFER_SIZE];
        uint16_t light_samples[SAMPLE_BUFFER_SIZE];
        
        // ����˫ͨ������
        for(int i=0; i<SAMPLE_BUFFER_SIZE; i++) 
        {
            light_samples[i] = adc_buffer[i*2];     // ͨ��1(����)
            sound_samples[i] = adc_buffer[i*2+1];   // ͨ��9(����)
        }

        // �˲�����
        Filter_Data(light_samples, SAMPLE_BUFFER_SIZE);
        Filter_Data(sound_samples, SAMPLE_BUFFER_SIZE);
        
        // �����ѹֵ(3.3V�ο�)
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
        light_V = current_light;  // ���µ�ǰֵ
        sound_V = current_sound;  // ���µ�ǰֵ
       
        dataReady = 0;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, SAMPLE_BUFFER_SIZE*2);
    }
}
#define WAVE_SMOOTHING 0.3f  // ƽ��ϵ��(0.1-0.3)

void Update_Waveform(float voltage) 
{
    static float filtered_voltage = 0;
    
    // һ�׵�ͨ�˲������ķ�����
    filtered_voltage = filtered_voltage * (1-WAVE_SMOOTHING) + voltage * WAVE_SMOOTHING;
    
    // ��̬�Ŵ�仯���֣���ǿ�Ӿ�Ч����
    static float last_voltage = 0;
    float delta = fabsf(filtered_voltage - last_voltage) * 3.0f; // �仯���Ŵ�2��
    float display_voltage = filtered_voltage + delta;
    
    // ��һ���洢
    sound_wave.samples[sound_wave.index] = (uint8_t)(display_voltage * 255 / 3.3f);
    sound_wave.index = (sound_wave.index + 1) % WAVE_HISTORY;
    last_voltage = filtered_voltage;
}

void Draw_Waveform(u8g2_t *u8g2) 
{
    const uint8_t y_base = 58; // ����λ��
    uint8_t prev_y = y_base - (sound_wave.samples[sound_wave.index] * WAVE_HEIGHT / 256);
    
    // ���ƻ���
    u8g2_DrawHLine(u8g2, 0, y_base, WAVE_HISTORY);
    
    // ���ƴ��߲��Σ�2���ؿ�
    for(uint8_t i=1; i<WAVE_HISTORY; i++) 
    {
        uint8_t x = i;
        uint8_t curr_y = y_base - (sound_wave.samples[(sound_wave.index+i)%WAVE_HISTORY] * WAVE_HEIGHT / 256);
        
        // ���Ʒ�Χ
        curr_y = (curr_y < y_base-WAVE_HEIGHT) ? y_base-WAVE_HEIGHT : curr_y;
        
        // �����߶Σ��Ӵ֣�
        u8g2_DrawLine(u8g2, x-1, prev_y, x, curr_y);
        u8g2_DrawPixel(u8g2, x, curr_y+1); // �±�Ե����
        
        prev_y = curr_y;
    }
}
// ������ֵ���� LED
void sound_to_led() 
{
    static uint32_t trigger_time = 0;  // ��¼����ʱ��
    static uint8_t led_active = 0;    // LED״̬��־
    
    if(sound_V >= sound_threshold) 
    {
        // ����������ֵʱ����LED����¼ʱ��
        if(!led_active) 
        {
            led_duty(0);           // ����LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // ��������ʱˢ�¼�ʱ����ѡ��
        trigger_time = HAL_GetTick(); 
    }
    else 
    {
        // ����������ֵʱ����Ƿ�ʱ
        if(led_active && (HAL_GetTick() - trigger_time >= 5000)) 
        {
            led_duty(450);             // 5���ر�LED
            led_active = 0;
        }
    }
}
// ������ֵ���� LED
void light_to_led()
{
    static uint32_t trigger_time = 0;  // ��¼����ʱ��
    static uint8_t led_active = 0;    // LED״̬��־
    
    if(light_V <= light_threshold) 
    {
        // ����������ֵʱ����LED����¼ʱ��
        if(!led_active) 
        {
            led_duty(0);           // ����LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // ��������ʱˢ�¼�ʱ����ѡ��
        trigger_time = HAL_GetTick(); 
    }
    else 
    {
        // ����������ֵʱ����Ƿ�ʱ
        if(led_active && (HAL_GetTick() - trigger_time >= 5000)) 
        {
            led_duty(450);             // 5���ر�LED
            led_active = 0;
        }
    }
}
/* ��ֵ���� */
/* ��ֵ���ڣ�ֱ�ӵ���ģʽ�� */
void adjust_sound_threshold() 
{  
    key=0;
    key_scan();
    Update_Waveform(sound_V);  // ���²�������

    u8g2_ClearBuffer(&u8g2);
    
    // ʵʱ��ʾ��ǰֵ����ֵ
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    sprintf(str, "Sound:%.2fV", sound_V);
    u8g2_DrawStr(&u8g2, 0, 12, str);
    
    sprintf(str, "Set:%.2fV", sound_threshold);
    u8g2_DrawStr(&u8g2, 64, 12, str);
    
    sprintf(str, "Max:%.2fV", sound_max_peak);
    u8g2_DrawStr(&u8g2, 0, 24, str);

	printf("Sound:%.2fV", sound_V);
    // ֱ�Ӱ������ڣ�����ģʽ�л���
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
    Update_Waveform(light_V);  // ���²�������

    u8g2_ClearBuffer(&u8g2);
    
    // ʵʱ��ʾ��ǰֵ����ֵ
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tr);
    sprintf(str, "light:%.2fV", light_V);
    u8g2_DrawStr(&u8g2, 0, 12, str);
    
    sprintf(str, "Set:%.2fV", light_threshold);
    u8g2_DrawStr(&u8g2, 64, 12, str);
    
    sprintf(str, "Max:%.2fV", light_max_peak);
    u8g2_DrawStr(&u8g2, 0, 24, str);
    
    printf("Sound:%.2fV", sound_V);
    // ֱ�Ӱ������ڣ�����ģʽ�л���
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

// �ƶ�ƽ���˲�
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


// �������ֵ
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

// DMA������ɻص�����
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == hadc1.Instance) 
    {
        dataReady = 1;  // ��������׼����־
    }
}

