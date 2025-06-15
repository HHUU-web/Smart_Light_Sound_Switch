#ifndef __SOUND_LIGHT_h
#define __SOUND_LIGHT_h

#include "main.h"
#include "oled.h"
/* Ӳ������ */
#define FILTER_WINDOW      3     // �ƶ�ƽ���˲�����
#define SAMPLE_BUFFER_SIZE 32    // ADC������������С
#define WAVE_HEIGHT       36     // ������ʾ�߶�(����)
#define WAVE_HISTORY      128     // ������ʷ����(�������OLED���)

/* ȫ�ֱ������� */
extern u8g2_t u8g2;
extern uint8_t key;

typedef struct {
    uint8_t samples[WAVE_HISTORY];  // �������ݻ��λ�����
    uint8_t index;                  // ��ǰд��λ��
} WaveformData;

extern WaveformData sound_wave;
extern float sound_V, light_V;
extern float sound_threshold, light_threshold;

/* �������� */
void SoundLight_Init(void);
void Process_Dual_ADC_Data(void);
void Update_Waveform(float voltage);
void Draw_Waveform(u8g2_t *u8g2);
void sound_to_led(void);
void light_to_led(void);
void adjust_sound_threshold(void);
void adjust_light_threshold(void);
void Filter_Data(uint16_t *data, uint32_t length);
uint16_t Find_Max(uint16_t *data, uint32_t length);
void light_detect(void);
void sound_detect(void);

#endif
