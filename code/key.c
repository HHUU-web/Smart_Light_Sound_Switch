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
    uint8_t key_value;      // ��Ӧ�Ĺ���ֵ����/��/ȷ�ϣ�
    KeyState state;
    uint16_t counter;
} KeyButton;

// ����4������
KeyButton keys[] = {
    {GPIOB, GPIO_PIN_3, KEY_LEFT,    KEY_IDLE, 0},
    {GPIOB, GPIO_PIN_4, KEY_RIGHT,  KEY_IDLE, 0},
    {GPIOB, GPIO_PIN_5, KEY_OK,    KEY_IDLE, 0},
    {GPIOB, GPIO_PIN_6, KEY_EXIT,  KEY_IDLE, 0},  // �� GPIO_PIN_6 ����Ϊ�˳�����
};
#define NUM_KEYS 4

uint8_t key_repeat_enable = 0;  // 0: ��ֹ������1: ��������
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
                    if (k->counter >= 1) {  // ����ȷ��
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
                    key = k->key_value;  // ���ð���״̬
                } else {
                    k->counter++;
                    if (i == 3 && k->counter >= 60000) {
                        wait_key++;
                        if (i == 3 && wait_key >= 600) {
                            key = KEY_LONG;
                            wait_key = 0;
                            k->state = KEY_WAIT_RELEASE;
                        }
                    } else if (key_repeat_enable && k->counter >= 5) {
                        key = k->key_value;  // ����������ÿ 20 �ν���һ��
                        k->counter = 0;      // ���ü��������γ�����
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