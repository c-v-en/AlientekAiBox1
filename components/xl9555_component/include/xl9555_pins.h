#ifndef XL9555_PINS_H
#define XL9555_PINS_H

/* Port 0 */
#define XL9555_P00_AP_INT       0x0001   /* 输入：光环境传感器中断 */
#define XL9555_P01_QMA_INT      0x0002   /* 输入：陀螺仪中断 */
#define XL9555_P02_BEEP         0x0004   /* 输出：蜂鸣器，低电平打开，高电平关闭 */
#define XL9555_P03_KEY1         0x0008   /* 输入：按键1 */
#define XL9555_P04_KEY0         0x0010   /* 输入：按键0 */
#define XL9555_P05_SPK_CTRL     0x0020   /* 输出：喇叭驱动，高电平选中 */
#define XL9555_P06_CTP_RST      0x0040   /* 输出：触摸屏复位，低电平复位 */
#define XL9555_P07_LCD_BL       0x0080   /* 输出：LCD背光，高电平点亮 */

/* Port 1 */
#define XL9555_P10_LEDR         0x0100   /* 输出：系统错误指示灯，高电平灭 */
#define XL9555_P11_CTP_INT      0x0200   /* 输入：触摸屏中断 */
#define XL9555_P12_IO1_2        0x0400   /* 输出：外扩引脚，默认低 */
#define XL9555_P13_IO1_3        0x0800   /* 输出：外扩引脚，默认低 */
#define XL9555_P14_IO1_4        0x1000   /* 输出：外扩引脚，默认低 */
#define XL9555_P15_IO1_5        0x2000   /* 输出：外扩引脚，默认低 */
#define XL9555_P16_IO1_6        0x4000   /* 输出：外扩引脚，默认低 */
#define XL9555_P17_IO1_7        0x8000   /* 输出：外扩引脚，默认低 */

/* 默认方向配置：0=输出，1=输入 */
#define XL9555_DEFAULT_DIR_MASK   0x021B

/* 默认输出状态 */
#define XL9555_DEFAULT_OUT_STATE  0x0144

#endif
