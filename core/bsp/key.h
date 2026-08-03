#ifndef __KEY_H__
#define __KEY_H__

#include <stdint.h>

// 只有 HOLD 模式支持多按键同时读取

/********************* ↓选择按键触发模式↓ *******************/
// #define KEY_MODE_TRIGGER        // 按下瞬间有效
#define KEY_MODE_RELEASE        // 松开瞬间有效
// #define KEY_MODE_HOLD           // 按住不放有效
/********************* ↑选择按键触发模式↑ *******************/

#define KEY_UNPRESSED       GPIO_PIN_SET       // 按键空闲
#define KEY_PRESSED         GPIO_PIN_RESET     // 按键按下

#define KEY_INTERVAL_MS            20
#define KEY_CNT                    4

typedef enum { 
  KEY_NONE = 0,
  KEY_1 = 0x01,
  KEY_2 = 0x02,
  KEY_3 = 0x04,
  KEY_4 = 0x08
  // KEY_5 = 0x10,
  // KEY_6 = 0x20,
  // KEY_7 = 0x40,
  // KEY_8 = 0x80,
} KeyNumTypeDef;

void Key_Init(void);
uint16_t Key_Read(void);

#endif
