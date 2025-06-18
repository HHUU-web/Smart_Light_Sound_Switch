#include "menu.h"
#include "u8g2.h"
#include <stdio.h>
#include <string.h> 
#include "key.h"
#include "sound_light.h"
#include "math.h"
#include "led.h"

extern u8g2_t u8g2;  

Speed_ENUM Speed_choose;

extern u8g2_t u8g2;
extern int led_on;
extern int time; 
char text[32];
int16_t display=48;
int16_t diaplay_trg=1;
uint8_t circle_num;
static uint8_t Picture_Flag=0;//功能选择标志位
int send_light=0,send_sound=0;//串口选择发送数据标志位
const char words[][20] = {
    {"LIGHT DETECTION"},
    {"SOUND DETECTION"},
    {"MIXED MODE"}, 
    {"BULB ADJUSTMENT"},
    {"SERIAL PORT"}
}; 
uint8_t Game_Menu_Flag=0;

const unsigned char light_icon[] = {
    // 光强检测图标
0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x00,0x00,0x80,0x03,0x00,0x00,0x80,0x03,0x00,0x00,0x80,0x03,0x00,0xE0,0x80,0x03,0x06,0xE0,0x01,0x00,0x0F,0xC0,0x81,0x81,0x07,
0x80,0xF1,0x8F,0x03,0x00,0xF8,0x3F,0x00,0x00,0x3C,0x7C,0x00,0x00,0x0E,0x70,0x00,0x00,0x07,0xE0,0x00,0x00,0x07,0xE0,0x00,0x3E,0x07,0xC0,0x01,0x3E,0x03,0xC0,0x7D,
0x3E,0x03,0xC0,0x7D,0x00,0x07,0xC0,0x7D,0x00,0x07,0xE0,0x00,0x00,0x0F,0xE0,0x00,0x00,0x0E,0x70,0x00,0x00,0x3C,0x7C,0x00,0x00,0xF8,0x3F,0x00,0xC0,0xF1,0x8F,0x01,
0xE0,0x81,0x81,0x03,0xF0,0x00,0x80,0x07,0x60,0xC0,0x01,0x07,0x00,0xC0,0x01,0x00,0x00,0xC0,0x01,0x00,0x00,0xC0,0x01,0x00,0x00,0xC0,0x01,0x00,0x00,0x00,0x00,0x00,
/* (32 X 32 )*/

};

const unsigned char sound_icon[] = {
    // 声音检测图标
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x0F,0x00,0xC0,0x00,0x0E,0x00,0xE0,0x01,0x1C,
0x00,0xF0,0x21,0x1C,0x00,0xF8,0x71,0x38,0x00,0xFE,0xF1,0x38,0xFC,0xFF,0xE1,0x78,0xFE,0xFF,0xE1,0x70,0xFE,0xFF,0xC1,0x71,0xFE,0xFF,0xC1,0x71,0xFE,0xFF,0xC1,0x71,
0xFE,0xFF,0xC1,0x71,0xFE,0xFF,0xC1,0x71,0xFE,0xFF,0xC1,0x71,0xFE,0xFF,0xE1,0x70,0xFC,0xFF,0xE1,0x78,0x00,0xFE,0xF1,0x38,0x00,0xF8,0x71,0x38,0x00,0xF0,0x21,0x1C,
0x00,0xC0,0x01,0x1C,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
/* (32 X 32 )*/
};
const unsigned char mix_icon[] = {
    // 混合检测图标
0x00,0x00,0x00,0x00,0x00,0xF0,0x0F,0x00,0x00,0xFE,0x7F,0x00,0x00,0xFF,0xFF,0x00,0xC0,0xFF,0xFF,0x03,0xE0,0x0F,0xF0,0x07,0xF0,0x03,0xC0,0x0F,0xF0,0x81,0x81,0x0F,
0xF8,0xC0,0x0F,0x1F,0x7C,0xC0,0x3F,0x3E,0x3C,0xC0,0x7F,0x3C,0x3C,0xC0,0x7F,0x3C,0x1E,0xC0,0xFB,0x78,0x1E,0xC0,0xF3,0x78,0x1E,0x87,0xF1,0x78,0x9E,0x07,0xF0,0x79,
0x9E,0x0F,0xE0,0x79,0x1E,0x8F,0xE1,0x78,0x1E,0xCF,0x03,0x78,0x1E,0xDF,0x03,0x78,0x3C,0xFE,0x03,0x3C,0x3C,0xFE,0x03,0x3C,0x7C,0xFC,0x03,0x3E,0xF8,0xF0,0x03,0x1F,
0xF0,0x81,0x80,0x0F,0xF0,0x03,0xC0,0x0F,0xE0,0x0F,0xF0,0x07,0xC0,0xFF,0xFF,0x03,0x00,0xFF,0xFF,0x00,0x00,0xFE,0x7F,0x00,0x00,0xF0,0x0F,0x00,0x00,0x00,0x00,0x00,
/* (32 X 32 )*/
};
const unsigned char bulb_icon[] = {
    // 灯泡调节图标
0x00,0x00,0x00,0x00,0x00,0xF0,0x0F,0x00,0x00,0xFC,0x3F,0x00,0x00,0x1E,0x78,0x00,0x00,0x07,0xE0,0x00,0x80,0x03,0xC0,0x01,0x80,0x01,0x80,0x01,0xC0,0x01,0x80,0x03,
0xC0,0x00,0x00,0x03,0xC0,0x00,0x00,0x03,0xC0,0x00,0x00,0x03,0xC0,0x00,0x00,0x03,0xC0,0x00,0x00,0x03,0xC0,0xB0,0x0D,0x03,0xC0,0xF0,0x0F,0x03,0xC0,0xE1,0x87,0x03,
0x80,0x61,0x86,0x01,0x00,0x43,0xC2,0x00,0x00,0x0E,0x70,0x00,0x00,0x7C,0x3E,0x00,0x00,0xF8,0x1F,0x00,0x00,0xC8,0x13,0x00,0x00,0xF8,0x1F,0x00,0x00,0xF8,0x1F,0x00,
0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00,0x00,0xF8,0x1F,0x00,0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00,0x00,0xF0,0x0F,0x00,0x00,0xE0,0x07,0x00,0x00,0x00,0x00,0x00,
/* (32 X 32 )*/
};

const unsigned char serial_icon[] = {
    // 串口图标
0x00,0x00,0x00,0x00,0x00,0xF8,0x1F,0x00,0x00,0xFE,0x7F,0x00,0x00,0x0F,0xF0,0x00,0x80,0x03,0xC0,0x01,0xC0,0x41,0x80,0x03,0xE0,0xC0,0x00,0x07,0x70,0xC0,0x03,0x0E,
0x70,0xC0,0x07,0x0E,0x30,0xC0,0x0F,0x0C,0x30,0xC0,0x3C,0x0C,0x38,0xCE,0x38,0x1C,0x38,0xDE,0x1C,0x1C,0x38,0xF8,0x0E,0x1C,0x38,0xF0,0x07,0x1C,0x38,0xE0,0x03,0x1C,
0x38,0xC0,0x03,0x1C,0x38,0xE0,0x07,0x1C,0x38,0xF0,0x0E,0x1C,0x38,0xFC,0x1C,0x1C,0x38,0xDE,0x38,0x1C,0x30,0xCC,0x3C,0x0C,0x30,0xC0,0x1E,0x0C,0x70,0xC0,0x0F,0x0C,
0x70,0xC0,0x03,0x0E,0xE0,0xC0,0x01,0x06,0xC0,0xC1,0x00,0x07,0x80,0x03,0xC0,0x03,0x00,0x0F,0xF0,0x01,0x00,0xFE,0x7F,0x00,0x00,0xF8,0x1F,0x00,0x00,0x00,0x00,0x00,
/* (32 X 32 )*/    
};

const unsigned char arrowhead[] = {
    // 箭头图标
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,
0x00,0x00,0x00,0x3C,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0xFE,0x00,0x00,0x00,0x00,0xFF,0x01,0x00,0x00,0x80,0xFF,0x01,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,
0x00,0x7C,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x7C,0x00,0x00,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0xE0,
0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"C:\Users\chang\Desktop\picture\9.bmp",0*/
/* (40 X 40 )*/
};

void ui_run(char* a ,char* a_trg,int b)
{
    if(*a < *a_trg)
    {
        *a += b;
        if(*a > *a_trg)    //防止加过头
            *a = *a_trg;
    }
    if(*a > *a_trg)
    {
        *a -= b;
        if(*a < *a_trg)//防止减过头
            *a = *a_trg;
    }
}

void ui_right_one_Picture(int16_t* a, int b)
{
    uint8_t i=0;
    if((i<=48))
    {
        *a += b;
        i += b;
    }
}

void ui_left_one_Picture(int16_t* a, int b )
{
    uint8_t i=0;
    if((i<=48))
    {
        *a -= b;
        i += b; 
    }
}

void Show_Menu_Config(void) 
{
    u8g2_SetFontMode(&u8g2, 1); // 设置字体模式
    u8g2_SetFontDirection(&u8g2, 0); // 设置字体方向
    u8g2_SetFont(&u8g2, u8g2_font_squeezed_b7_tr); // 设置字体格式

    u8g2_DrawXBM(&u8g2, 44, 36, 40, 40, arrowhead); // 箭头图标
    u8g2_DrawXBM(&u8g2, display, 16, 32, 32, light_icon); // 光强检测图标
    u8g2_DrawXBM(&u8g2, display + 48, 16, 32, 32, sound_icon); // 声音检测图标
    u8g2_DrawXBM(&u8g2, display + 96, 16, 32, 32, mix_icon); // 混合模式图标
    u8g2_DrawXBM(&u8g2, display + 144, 16, 32, 32, bulb_icon); // 灯泡调节图标
    u8g2_DrawXBM(&u8g2, display + 192, 16, 32, 32, serial_icon); // 串口图标
    u8g2_SendBuffer(&u8g2); // 更新显示
}
void Show_Menu(Speed_ENUM Speed_choose) {
    key = 0;
    key_scan();
    Game_Menu_Flag = key;

    if ((key == KEY_LEFT) && (display > -144)) {
        if (Picture_Flag < 4)
            Picture_Flag++;      
        circle_num = 48 / Speed_choose;
        while (circle_num) {
            u8g2_ClearBuffer(&u8g2);
            ui_left_one_Picture(&display, Speed_choose);

            u8g2_DrawXBM(&u8g2, 44, 36, 40, 40, arrowhead);
            u8g2_DrawXBM(&u8g2, display, 16, 32, 32, light_icon);
            u8g2_DrawXBM(&u8g2, display + 48, 16, 32, 32, sound_icon);
            u8g2_DrawXBM(&u8g2, display + 96, 16, 32, 32, mix_icon);
            u8g2_DrawXBM(&u8g2, display + 144, 16, 32, 32, bulb_icon);
            u8g2_DrawXBM(&u8g2, display + 192, 16, 32, 32, serial_icon);

            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_SendBuffer(&u8g2);

            circle_num--;
        }
        key = 0;
    }

    else if ((key == KEY_RIGHT) && (display < 48)) {
        if (Picture_Flag > 0) 
            Picture_Flag--;        
        circle_num = 48 / Speed_choose;
        while (circle_num) {
            u8g2_ClearBuffer(&u8g2);
            ui_right_one_Picture(&display, Speed_choose);

            u8g2_DrawXBM(&u8g2, 44, 36, 40, 40, arrowhead);
            u8g2_DrawXBM(&u8g2, display, 16, 32, 32, light_icon);
            u8g2_DrawXBM(&u8g2, display + 48, 16, 32, 32, sound_icon);
            u8g2_DrawXBM(&u8g2, display + 96, 16, 32, 32, mix_icon);
            u8g2_DrawXBM(&u8g2, display + 144, 16, 32, 32, bulb_icon);
            u8g2_DrawXBM(&u8g2, display + 192, 16, 32, 32, serial_icon);

            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_SendBuffer(&u8g2);

            circle_num--;
        }
        key = 0;
    }

    switch (Picture_Flag % 5) {
        case 0:
            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_DrawStr(&u8g2, 50, 10, words[0]); // 显示光强检测文字
            break;
        case 1:
            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_DrawStr(&u8g2, 50, 10, words[1]); // 显示声音检测文字
            break;
        case 2:
            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_DrawStr(&u8g2, 50, 10, words[2]); // 显示混合模式文字
            break;
        case 3:
            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_DrawStr(&u8g2, 50, 10, words[3]); // 显示灯泡调节文字
            break;
        case 4:
            u8g2_DrawStr(&u8g2, 20, 10, "MENU:");
            u8g2_DrawStr(&u8g2, 50, 10, words[4]); // 显示串口文字
            
            break;
    }
    if(Game_Menu_Flag == KEY_OK)//确认键
    {
        Game_Menu_Flag=0;
        switch (Picture_Flag % 5)
        {
            case 0:
                light_detect();
                break;
            case 1:
                sound_detect();
                break;
            case 2:
                mix_detect();
                break;
            case 3:
                Blib_adjustment_Menu();
                break;
            case 4:
                Serial_Adjustment_Menu();
                break;
        }            
    }
    u8g2_SendBuffer(&u8g2);
}

void Blib_adjustment_Menu(void) 
{
    static char Box_x = 1;
    static char Box_y = 14;
    static char Box_w = 84;
    static char Box_h = 13;
           
    static char Box_x_trg;
    static char Box_y_trg;
    static char Box_w_trg;
    static char Box_h_trg;
    
    static int8_t Box_Flag = 0;
    
    if(Game_Menu_Flag == KEY_OK)//确认键
    {
        To_Blib_adjustment_Menu();
        while(1)
        {	
            key=0;
            key_scan();
            Game_Menu_Flag = key; 
            u8g2_ClearBuffer(&u8g2);
            
            u8g2_DrawStr(&u8g2,20,10,"Blib adjustment");
            u8g2_DrawLine(&u8g2, 1, 13, 128, 13);
            sprintf(text, "Brightness:%d%%", led_on);
            u8g2_DrawStr(&u8g2,3,24,text);
            sprintf(text, "Time:%ds", time);
            u8g2_DrawStr(&u8g2,3,36,text);
            
            ui_run(&Box_x, &Box_x_trg,1);
            ui_run(&Box_y, &Box_y_trg,1);
            ui_run(&Box_w, &Box_w_trg,2);
            ui_run(&Box_h, &Box_h_trg,1);
            
            u8g2_DrawFrame(&u8g2, Box_x, Box_y, Box_w, Box_h);
            u8g2_SendBuffer(&u8g2);
            
            if(Game_Menu_Flag == KEY_LEFT)
            {
                Box_Flag++;
                if(Box_Flag >= 3)Box_Flag=3;
            }
            
            if(Game_Menu_Flag == KEY_RIGHT)
            {
                Box_Flag--;
                if(Box_Flag <= 0)Box_Flag=0;
            }
            if(Game_Menu_Flag == KEY_OK)
            {
                if(Box_Flag==0)
                {
                    bright_adjust();
                }
                else if(Box_Flag==1)
                {
                    time_adjust();
                }
            }
            if(Game_Menu_Flag == KEY_EXIT)
            {
                u8g2_ClearBuffer(&u8g2);
                Blib_adjustment_To_Menu_Display();  
                break;
            }
            
            switch(Box_Flag)
            {
                case 0: Box_x_trg = 1;Box_y_trg = 14;Box_w_trg = 90;Box_h_trg = 13;break;
                case 1: Box_x_trg = 1;Box_y_trg = 27;Box_w_trg = 90;Box_h_trg = 13;break;
            }                              
        }               
    }  
    Game_Menu_Flag = 0;//重置	
    Show_Menu_Config();//回到主显示页面
    display = -96;
    Picture_Flag = 3;
}

void To_Blib_adjustment_Menu(void)//菜单→光强检测菜单过渡动画
{
    char Menu_Display = 10;
    char Menu_Display_trg =74;
    
    while(Menu_Display != Menu_Display_trg)
    {	
        u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"MENU:");//Y要变为0或>=72“MENU”才会完全消失
        u8g2_DrawXBM(&u8g2,display,Menu_Display+6,32,32,light_icon);
        u8g2_DrawXBM(&u8g2,display+48,Menu_Display+6,32,32,sound_icon);
        u8g2_DrawXBM(&u8g2,display+96,Menu_Display+6,32,32,mix_icon);    
        u8g2_DrawXBM(&u8g2,display+144,Menu_Display+6,32,32,bulb_icon);
        u8g2_DrawXBM(&u8g2,display+192,Menu_Display+6,32,32,serial_icon); 
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_SendBuffer(&u8g2);
    }
    
    Menu_Display = 74;	
    Menu_Display_trg =10;

    while(Menu_Display != Menu_Display_trg)
    {	u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"Blib adjustment");
        u8g2_DrawLine(&u8g2, 1, Menu_Display+3, 128, Menu_Display+3);

        u8g2_DrawStr(&u8g2,3,Menu_Display+14,"Brightness: ");
        u8g2_DrawStr(&u8g2,3,Menu_Display+26,"Time: ");
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_SendBuffer(&u8g2);
    }	
}

void Blib_adjustment_To_Menu_Display(void)//光强检测菜单→菜单过渡动画
{
    char Menu_Display = 10;
    char Menu_Display_trg =74;
    
    while(Menu_Display != Menu_Display_trg)
    {	
        u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"Blib adjustment");
        u8g2_DrawLine(&u8g2, 1, Menu_Display+3, 128, Menu_Display+3);

        u8g2_DrawStr(&u8g2,3,Menu_Display+14,"Brightness: ");
        u8g2_DrawStr(&u8g2,3,Menu_Display+26,"Time: ");
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_SendBuffer(&u8g2);
    }
    Menu_Display = 74;	
    Menu_Display_trg =10;

    while(Menu_Display != Menu_Display_trg)
    {	
        u8g2_ClearBuffer(&u8g2);
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"MENU:");
        u8g2_DrawXBM(&u8g2,display,Menu_Display+6,32,32,light_icon);
        u8g2_DrawXBM(&u8g2,display+48,Menu_Display+6,32,32,sound_icon);
        u8g2_DrawXBM(&u8g2,display+96,Menu_Display+6,32,32,mix_icon);   
        u8g2_DrawXBM(&u8g2,display+144,Menu_Display+6,32,32,bulb_icon);
        u8g2_DrawXBM(&u8g2,display+192,Menu_Display+6,32,32,serial_icon); 
        u8g2_SendBuffer(&u8g2);
    }	
}

void Serial_Adjustment_Menu(void) 
{
    static char Box_x = 1;
    static char Box_y = 14;
    static char Box_w = 84;
    static char Box_h = 13;
           
    static char Box_x_trg;
    static char Box_y_trg;
    static char Box_w_trg;
    static char Box_h_trg;
    
    static int8_t Box_Flag = 0;
    
    if(Game_Menu_Flag == KEY_OK)//确认键
    {
        To_Serial_Adjustment_Menu();
        while(1)
        {	
            key=0;
            key_scan();
            Game_Menu_Flag = key; 
            u8g2_ClearBuffer(&u8g2);
            
            u8g2_DrawStr(&u8g2,20,10,"Serial Data Send");
            u8g2_DrawLine(&u8g2, 1, 13, 128, 13);
            sprintf(text, "Send Light Data");
            u8g2_DrawStr(&u8g2,3,24,text);
            sprintf(text, "Send Sound Data");
            u8g2_DrawStr(&u8g2,3,36,text);
            if(send_light==1)
            {
                sprintf(text, "ON");
                u8g2_DrawStr(&u8g2,100,24,text);
            }
            else if(send_light==0)
            {
                sprintf(text, "OFF");
                u8g2_DrawStr(&u8g2,100,24,text);
            }
            if(send_sound==1)
            {
                sprintf(text, "ON");
                u8g2_DrawStr(&u8g2,100,36,text);
            }
            else if(send_sound==0)
            {
                sprintf(text, "OFF");
                u8g2_DrawStr(&u8g2,100,36,text);
            }
            
            ui_run(&Box_x, &Box_x_trg,1);
            ui_run(&Box_y, &Box_y_trg,1);
            ui_run(&Box_w, &Box_w_trg,2);
            ui_run(&Box_h, &Box_h_trg,1);
            
            u8g2_DrawFrame(&u8g2, Box_x, Box_y, Box_w, Box_h);
            u8g2_SendBuffer(&u8g2);
            
            if(Game_Menu_Flag == KEY_LEFT)
            {
                Box_Flag++;
                if(Box_Flag >= 2)Box_Flag=2; // 现在有2个选项
            }
            
            if(Game_Menu_Flag == KEY_RIGHT)
            {
                Box_Flag--;
                if(Box_Flag <= 0)Box_Flag=0;
            }
            if(Game_Menu_Flag == KEY_OK)
            {
                if(Box_Flag==0)
                {
                    send_light=!send_light;
                }
                else if(Box_Flag==1)
                {
                    send_sound=!send_sound;

                }
            }
            if(Game_Menu_Flag == KEY_EXIT)
            {
                u8g2_ClearBuffer(&u8g2);
                Serial_Adjustment_To_Menu_Display();  
                break;
            }
            
            switch(Box_Flag)
            {
                case 0: Box_x_trg = 1;Box_y_trg = 14;Box_w_trg = 90;Box_h_trg = 13;break;
                case 1: Box_x_trg = 1;Box_y_trg = 27;Box_w_trg = 90;Box_h_trg = 13;break;
            }                              
        }               
    }  
    Game_Menu_Flag = 0;//重置	
    Show_Menu_Config();//回到主显示页面
    display = -144;
    Picture_Flag = 4;
}

void To_Serial_Adjustment_Menu(void)//菜单→串口模式菜单过渡动画
{
    char Menu_Display = 10;
    char Menu_Display_trg =74;
    
    while(Menu_Display != Menu_Display_trg)
    {	
        u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"MENU:");//Y要变为0或>=72“MENU”才会完全消失
        u8g2_DrawXBM(&u8g2,display,Menu_Display+6,32,32,light_icon);
        u8g2_DrawXBM(&u8g2,display+48,Menu_Display+6,32,32,sound_icon);
        u8g2_DrawXBM(&u8g2,display+96,Menu_Display+6,32,32,mix_icon);    
        u8g2_DrawXBM(&u8g2,display+144,Menu_Display+6,32,32,bulb_icon);
        u8g2_DrawXBM(&u8g2,display+192,Menu_Display+6,32,32,serial_icon); 
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_SendBuffer(&u8g2);
    }
    
    Menu_Display = 74;	
    Menu_Display_trg =10;

    while(Menu_Display != Menu_Display_trg)
    {	u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"Serial Data Send");
        u8g2_DrawLine(&u8g2, 1, Menu_Display+3, 128, Menu_Display+3);

        u8g2_DrawStr(&u8g2,3,Menu_Display+14,"Send Light Data");
        u8g2_DrawStr(&u8g2,3,Menu_Display+26,"Send Sound Data");
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_SendBuffer(&u8g2);
    }	
}

void Serial_Adjustment_To_Menu_Display(void)//串口模式菜单→菜单过渡动画
{
    char Menu_Display = 10;
    char Menu_Display_trg =74;
    
    while(Menu_Display != Menu_Display_trg)
    {	
        u8g2_ClearBuffer(&u8g2);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"Serial Data Send");
        u8g2_DrawLine(&u8g2, 1, Menu_Display+3, 128, Menu_Display+3);

        u8g2_DrawStr(&u8g2,3,Menu_Display+14,"Send Light Data");
        u8g2_DrawStr(&u8g2,3,Menu_Display+26,"Send Sound Data");
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_SendBuffer(&u8g2);
    }
    Menu_Display = 74;	
    Menu_Display_trg =10;

    while(Menu_Display != Menu_Display_trg)
    {	
        u8g2_ClearBuffer(&u8g2);
        ui_run(&Menu_Display,&Menu_Display_trg,8);
        u8g2_DrawStr(&u8g2,20,Menu_Display,"MENU:");
        u8g2_DrawXBM(&u8g2,display,Menu_Display+6,32,32,light_icon);
        u8g2_DrawXBM(&u8g2,display+48,Menu_Display+6,32,32,sound_icon);
        u8g2_DrawXBM(&u8g2,display+96,Menu_Display+6,32,32,mix_icon);   
        u8g2_DrawXBM(&u8g2,display+144,Menu_Display+6,32,32,bulb_icon);
        u8g2_DrawXBM(&u8g2,display+192,Menu_Display+6,32,32,serial_icon); 
        u8g2_SendBuffer(&u8g2);
    }	
}


