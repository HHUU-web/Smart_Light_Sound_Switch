#include "menu.h"            
#include "sound_light.h"
#include "key.h"

DetectMode current_mode;
uint8_t Show_MainMenu(u8g2_t *u8g2) 
{
    static uint8_t selected = 0;
    const char *items[] = {"Light Detect", "Sound Detect", "Combined Detect"};
    
    while(1) 
    {
        // 1. 显示菜单
        u8g2_ClearBuffer(u8g2);
        u8g2_SetFont(u8g2, u8g2_font_6x10_tr);
        
        for(uint8_t i=0; i<3; i++) 
        {
            if(i == selected) 
            {
                u8g2_DrawBox(u8g2, 0, 10*i, 128, 10);
                u8g2_SetDrawColor(u8g2, 0);
            }
            u8g2_DrawStr(u8g2, 5, 10*(i+1), items[i]);
            u8g2_SetDrawColor(u8g2, 1);
        }
        u8g2_SendBuffer(u8g2);
        
        // 2. 按键处理
        key_scan();
        if(key == KEY_UP) 
        {
            selected = (selected > 0) ? selected-1 : 2;
            key = KEY_NONE;
        }
        if(key == KEY_DOWN)
        {
            selected = (selected < 2) ? selected+1 : 0;
            key = KEY_NONE;
        }
        if(key == KEY_OK)
        {
            return selected;  // 返回选中的模式
            key = KEY_NONE;
        } 
        
        HAL_Delay(100);
    }
}

void menu(void)
{
    current_mode = MODE_MENU;  // 启动时显示菜单
    key=0;
    while(1) 
    {
        // 模式选择
        if(current_mode == MODE_MENU) 
        {
            current_mode = Show_MainMenu(&u8g2);
            continue;
        }
        
        // 执行当前模式
        Process_Dual_ADC_Data();
        switch(current_mode) 
        {
            case MODE_LIGHT: 
                adjust_light_threshold();
                light_to_led();
                break;
            case MODE_SOUND: 
                adjust_sound_threshold();
                sound_to_led();
                break;
            case MODE_COMBINED:
                if(light_V < light_threshold && sound_V > sound_threshold) 
                    led_duty(450);
                else 
                    led_duty(0);
                break;
        }
    }
    key=0;
}