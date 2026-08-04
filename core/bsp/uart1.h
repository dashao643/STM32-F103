#ifndef __UART1_H__
#define __UART1_H__

#include "stm32f1xx_hal_def.h"

#include <stdbool.h>

#define UART1_RX_BUFF_MAX_SIZE               256     // 接收最大帧长度
#define UART1_TX_BUFF_MAX_SIZE               256     // 发送最大帧长度
#define UART1_TX_TIMEOUT_MS                  500

void UART1_Init(void);
void UART1_MspInit(UART_HandleTypeDef *huart);

HAL_StatusTypeDef UART1_Transmit(const uint8_t *data, uint16_t size);
HAL_StatusTypeDef UART1_Transmit_DMA(const uint8_t *data, uint16_t size);

UART_HandleTypeDef* UART1_GetHandle(void);
volatile bool* UART1_GetRxFlag(void);
uint8_t* UART1_GetRxBuf(uint16_t *size);

#endif
