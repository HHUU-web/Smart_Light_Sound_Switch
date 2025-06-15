#ifndef __SOUND_LIGHT_h
#define __SOUND_LIGHT_h

#include "main.h"
#include "oled.h"
/* 硬件配置 */
#define FILTER_WINDOW      3     // 移动平均滤波窗口
#define SAMPLE_BUFFER_SIZE 32    // ADC采样缓冲区大小
#define WAVE_HEIGHT       36     // 波形显示高度(像素)
#define WAVE_HISTORY      128     // 波形历史点数(建议等于OLED宽度)

/* 全局变量声明 */
extern u8g2_t u8g2;
extern uint8_t key;

typedef struct {
    uint8_t samples[WAVE_HISTORY];  // 波形数据环形缓冲区
    uint8_t index;                  // 当前写入位置
} WaveformData;

extern WaveformData sound_wave;
extern float sound_V, light_V;
extern float sound_threshold, light_threshold;

/* 函数声明 */
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
