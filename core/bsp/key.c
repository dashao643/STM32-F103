#include "stm32f1xx_hal.h"
#include "general.h"
#include "key.h"

#include <stdbool.h>

#define KEY_GPIO_Port       GPIOB

#define KEY_1_GPIO_Port     GPIOB
#define KEY_1_Pin           GPIO_PIN_11

#define KEY_2_GPIO_Port     GPIOB
#define KEY_2_Pin           GPIO_PIN_10

#define KEY_3_GPIO_Port     GPIOB
#define KEY_3_Pin           GPIO_PIN_1

#define KEY_4_GPIO_Port     GPIOB
#define KEY_4_Pin           GPIO_PIN_0

typedef struct {
    uint16_t preKey;
    uint16_t curKey;
    uint32_t scanTimer;
} Key_TypeDef;

static GPIO_PortPinTypeDef keyPortPin[KEY_CNT] = { 
    { KEY_1_GPIO_Port, KEY_1_Pin }, 
    { KEY_2_GPIO_Port, KEY_2_Pin }, 
    { KEY_3_GPIO_Port, KEY_3_Pin }, 
    { KEY_4_GPIO_Port, KEY_4_Pin } 
};

static Key_TypeDef key = { 0 };

static uint16_t keyScan(void)
{
    uint16_t keyMask = KEY_NONE;

    for (uint8_t i = 0; i < KEY_CNT; i++) {
        if (HAL_GPIO_ReadPin(keyPortPin[i].port, keyPortPin[i].pin) == KEY_PRESSED) {
            keyMask |= (1 << i);
        }
    }

    return keyMask;
}

void Key_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio;

    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;        // 固定上拉
    gpio.Pin = KEY_1_Pin | KEY_2_Pin | KEY_3_Pin | KEY_4_Pin;

    HAL_GPIO_Init(KEY_GPIO_Port, &gpio);

    key.preKey = KEY_NONE;
    key.curKey = KEY_NONE;
    key.scanTimer = HAL_GetTick();
}

// 主循环调用
uint16_t Key_Read(void)
{
    if (HAL_GetTick() - key.scanTimer < KEY_INTERVAL_MS)
        return KEY_NONE;

    key.scanTimer = HAL_GetTick();

    key.preKey = key.curKey;
    key.curKey = keyScan();

    // 按下触发：上次没按，此次按下
#ifdef KEY_MODE_TRIGGER
    if (key.preKey == KEY_NONE && key.curKey != KEY_NONE)
        return key.curKey;

    // 松开触发：上次按下，此次没按(返回上次值，此次已清零)
#elif defined(KEY_MODE_RELEASE)
    if (key.preKey != KEY_NONE && key.curKey == KEY_NONE)
        return key.preKey;

    // 按住触发：当前按键值
#elif defined(KEY_MODE_HOLD)
    return key.curKey;

#endif
    return KEY_NONE;
}

// example:
// KeyNumTypeDef keyNum = Key_Read();

// if (READ_BIT(keyNum, KEY_1)) {
//     printf("test\n");
// } 