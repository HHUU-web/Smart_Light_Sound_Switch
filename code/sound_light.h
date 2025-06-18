#ifndef __SOUND_LIGHT_h
#define __SOUND_LIGHT_h

#include "main.h"
#include "oled.h"

#define WAVE_HISTORY 128

typedef struct {
    uint16_t samples[WAVE_HISTORY];
    uint8_t index;
} WaveformData;

void SoundLight_Init(void);
void Update_Waveform_Raw(uint16_t raw_input);
void Draw_Waveform(u8g2_t *u8g2);
void sound_to_led(void);
void sound_up_led(void);
void light_to_led(void);
void adjust_sound_threshold(void);
void adjust_light_threshold(void);
void light_detect(void);
void sound_detect(void);
void mix_detect(void);

#endif
