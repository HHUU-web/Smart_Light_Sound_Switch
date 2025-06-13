#include "key.h"

uint8_t key = 0;



typedef enum {
    KEY_IDLE,
    KEY_DEBOUNCE,
    KEY_PRESSED,
    KEY_WAIT_RELEASE
} KeyState;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t key_value;      // 对应的功能值（上/下/确认）
    KeyState state;
    uint16_t counter;
} KeyButton;

// 定义4个按键（PB2, PB1, PB0, PA3）
KeyButton keys[] = {
    {GPIOB, GPIO_PIN_3, KEY_UP,  KEY_IDLE, 0},
    {GPIOB, GPIO_PIN_4, KEY_DOWN,  KEY_IDLE, 0},
    {GPIOB, GPIO_PIN_5, KEY_OK,  KEY_IDLE, 0},
    {GPIOB, GPIO_PIN_6, 0,       KEY_IDLE, 0},  // 第4个按键特殊处理
};
#define NUM_KEYS 4

uint8_t key_repeat_enable = 0;  // 0: 禁止连按，1: 允许连按
uint16_t wait_key = 0;
void key_scan(void) {
    for (int i = 0; i < NUM_KEYS; i++) {
        KeyButton *k = &keys[i];
        uint8_t is_pressed = (HAL_GPIO_ReadPin(k->port, k->pin) == GPIO_PIN_RESET);

        switch (k->state) {
            case KEY_IDLE:
                if (is_pressed) {
                    k->state = KEY_DEBOUNCE;
                    k->counter = 0;
                }
                break;

            case KEY_DEBOUNCE:
                if (is_pressed) {
                    k->counter++;
                    if (k->counter >= 1) {  // 消抖确认
                        k->state = KEY_PRESSED;
                        k->counter = 0;
                    }
                } else {
                    k->state = KEY_IDLE;
                }
                break;

            case KEY_PRESSED:
                if (!is_pressed) {
                    k->state = KEY_IDLE;
                    if (i == 3) {
                        key = KEY_SHORT;
                    } else {
                        key = k->key_value;
                    }
                } else {
                    k->counter++;
                    if (i == 3 && k->counter >= 60000) {
											wait_key++;
											if(i == 3 && wait_key >= 600){
                        key = KEY_LONG;
												wait_key = 0;
                        k->state = KEY_WAIT_RELEASE;
											}
                    } 
										else if (key_repeat_enable && k->counter >= 5) {
												
                        // 连按触发，每 20 次进入一次
                        key = k->key_value;
                        k->counter = 0;  // 重置计数器以形成周期
                    }
                }
                break;

            case KEY_WAIT_RELEASE:
                if (!is_pressed) {
                    k->state = KEY_IDLE;
                    k->counter = 0;
										wait_key = 0;
                }
                break;
        }
    }
}

