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

volatile uint16_t ADC_Buffer[2];  // DMA�ɼ���������˫·������+��ǿ��
WaveformData sound_wave = {0};
float sound_threshold = 620;
uint32_t light_threshold = 500;
char str[32];


// ���ݴ���
#define LIGHT_FILTER_ALPHA 0.1f   // ��ǿ�˲�������ԽСԽƽ�ȣ�
#define SOUND_FILTER_ALPHA 0.2f   // �����˲�������ԽСԽƽ�ȣ�
float sound_db = 0.0f;         // �����ֱ�ֵ
uint32_t light_lux = 0;        // ��ǿ�ն�ֵ (0-65536Lx)
uint32_t light_sum = 0;
uint32_t voice_amp_sum = 0;
uint8_t sample_count = 0;
// ��ǿ�����ر���
uint8_t voice_counter = 0;
uint16_t voice_max = 0, voice_min = 4095;


// ��ʼ������
void SoundLight_Init(void) 
{
    // ����ADCУ׼��DMA�ɼ�
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Buffer, 2);
}
// ADCֵת��ѹ����
float adc_to_voltage(uint16_t adc_val) {
    return adc_val * 3.3f / 4095.0f;
}
// ��ʱ��5ms�ص�����
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) // 5ms��ʱ��
    {
        uint16_t light = ADC_Buffer[0];
        uint16_t voice = ADC_Buffer[1];

        // 1. ��ǿת��
        float lux = (float)(light) * 65536.0f / 4095.0f;
        if (lux > 65536.0f) lux = 65536.0f;

        light_sum += (uint32_t)lux;

        // 2. ��������ÿ25ms����һ�η��ȣ�
        if (voice > voice_max) voice_max = voice;
        if (voice < voice_min) voice_min = voice;
        voice_counter++;

        if (voice_counter >= 5)  // 25ms ����
        {
            voice_counter = 0;

            uint16_t amplitude = voice_max - voice_min;
            voice_amp_sum += amplitude;

            voice_max = 0;
            voice_min = 4095;

            sample_count++;
        }

        // 3. ÿ��100�����һ�Σ�100 * 5ms = 500ms��
        if (sample_count >= 70 / 5)  // 20�� * 25ms = 500ms
        {
            light_lux = light_sum / 70;          // 100��ƽ����ǿ
            sound_db = voice_amp_sum / 14;        // 20��ƽ����������

            // �����ۼ�
            light_sum = 0;
            voice_amp_sum = 0;
            sample_count = 0;
        }
    }
}

#define WAVE_HEIGHT     60     // ������ʾ�߶�(����)
#define WAVE_HISTORY    128     // ������ʷ����(�������OLED���)
#define WAVE_TYPE_SOUND 0
#define WAVE_TYPE_LIGHT 1

uint8_t waveform_type = WAVE_TYPE_SOUND;  // Ĭ�ϲ�������

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

// ������ֵ���� LED
void sound_to_led(void) 
{
    static uint32_t trigger_time = 0;  // ��¼����ʱ��
    static uint8_t led_active = 0;    // LED״̬��־
    uint8_t last=0;
    if((sound_db >= sound_threshold))
    {
        // ����������ֵʱ����LED����¼ʱ��
        if(!led_active) 
        {
            led_duty(led_on);           // ����LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // ��������ʱˢ�¼�ʱ����ѡ��
        trigger_time = HAL_GetTick(); 

    }
    else 
    {
        // ����������ֵʱ����Ƿ�ʱ
        if(led_active && (HAL_GetTick() - trigger_time >= led_last_time)) 
        {
            led_duty(100);             // 5���ر�LED
            led_active = 0;
        }
    }

}

void sound_up_led(void)
{
    // �趨һ��������ӳ�䣨�������/������
    float max_amplitude = 1000.0f;  // ����ʵ����� sound_db ����
    float duty;

    if (sound_db > max_amplitude) sound_db = max_amplitude;

    // ӳ�乫ʽ�������� �� ռ�ձ�С��Խ����������С �� ռ�ձȴ�Խ����
    duty = 100.0f - (sound_db / max_amplitude) * 100.0f;

    // ���Ʒ�Χ
    if (duty < 0) duty = 0;
    if (duty > 100) duty = 100;

    led_duty((int)duty);
}

// ������ֵ���� LED
void light_to_led()
{
    static uint32_t trigger_time = 0;  // ��¼����ʱ��
    static uint8_t led_active = 0;    // LED״̬��־
    
    if(light_lux <= light_threshold) 
    {
        // ����������ֵʱ����LED����¼ʱ��
        if(!led_active) 
        {
            led_duty(led_on);           // ����LED
            trigger_time = HAL_GetTick();
            led_active = 1;
        }
        // ��������ʱˢ�¼�ʱ����ѡ��
        trigger_time = HAL_GetTick(); 
    }
    else 
    {
        // ����������ֵʱ����Ƿ�ʱ
        if(led_active && (HAL_GetTick() - trigger_time >= led_last_time)) 
        {
            led_duty(100);             // 5���ر�LED
            led_active = 0;
        }
    }
}
/* ��ֵ���� */
/* ��ֵ���ڣ�ֱ�ӵ���ģʽ�� */
int sound_led_mode=0;//Ĭ��Ϊ������������

void adjust_sound_threshold() 
{  
    key=0;
    key_scan();
    Game_Menu_Flag = key;
    waveform_type = WAVE_TYPE_SOUND;
    Update_Waveform_Raw((uint16_t)sound_db); 
    u8g2_ClearBuffer(&u8g2);
    
    // ʵʱ��ʾ��ǰֵ����ֵ
    u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr );
    sprintf(str, "Sound:%.0f db", sound_db);
    u8g2_DrawStr(&u8g2, 0, 12, str);
    
    sprintf(str, "Set:%.0f db", sound_threshold);
    u8g2_DrawStr(&u8g2, 64, 12, str);
    
    if(send_sound)
    {
        printf("Sound:%.0f db\n", sound_db);//���ڷ���
    }
    // ֱ�Ӱ�������
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
        
        // ʵʱ��ʾ��ǰֵ����ֵ
        u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr );
        sprintf(str, "light:%d Lx", light_lux);
        u8g2_DrawStr(&u8g2, 0, 12, str);
        
        sprintf(str, "Set:%d Lx", light_threshold);
        u8g2_DrawStr(&u8g2, 64, 12, str);

        if(send_light)
        {
            printf("Light:%d Lx\n", light_lux);//���ڷ���
        }
        // ֱ�Ӱ�������
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
    
        // ʵʱ��ʾ������ǰֵ����ֵ
        u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr  );
        sprintf(str, "Sound:%.0f db", sound_db);
        u8g2_DrawStr(&u8g2, 0, 10, str);
        sprintf(str, "Set:%.0f db", sound_threshold);
        u8g2_DrawStr(&u8g2, 64, 10, str);

        // ʵʱ��ʾ�⵱ǰֵ����ֵ
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

// DMA������ɻص�����
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == hadc1.Instance) 
    {

    }
}
