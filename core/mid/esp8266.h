#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "stm32f1xx_hal_def.h"

#include <stdint.h>
#include <stdbool.h>

/********************* 选择ESP8266连接模式 *******************/
// #define ESP8266_MODE_AP
#define ESP8266_MODE_STA

#define ESP8266_INSTANCE                 USART2

// #define ESP8266_TX_TIMEOUT_MS            50
// #define ESP8266_TX_MAXLENTH              256
// #define ESP8266_RX_MAXLENTH              256
#define ESP8266_RX_MINLENTH              12       // 最小帧长度

#define ESP8266_RETRY_COUNT              3        // 重试次数   

#define ESP8266_CLOCK_SYN_MS             5000     // 等待8266时钟校准

HAL_StatusTypeDef ESP8266_AT_Transmit(const char *cmd);
HAL_StatusTypeDef ESP8266_AT_Receive(const char *res, uint16_t timeout);

bool ESP8266_ConnectToServer(void);

void ESP8266_Init(void);
void ESP8266_Task(void);

#endif
