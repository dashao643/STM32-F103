#ifndef __UART1_H__
#define __UART1_H__

#include "stm32f1xx_hal_def.h"

#include <stdbool.h>

#define ENABLE_UART1

#define UART1_RX_BUFF_MAX_SIZE               256     // 接收最大帧长度
#define UART1_TX_BUFF_MAX_SIZE               256     // 发送最大帧长度
#define UART1_TX_TIMEOUT_MS                  500

HAL_StatusTypeDef UART1_Init(void);
HAL_StatusTypeDef UART1_Transmit(const uint8_t *data, uint16_t size);
HAL_StatusTypeDef UART1_Transmit_DMA(const uint8_t *data, uint16_t size);

UART_HandleTypeDef* UART1_GetHandle(void);
DMA_HandleTypeDef* UART1_GetTxDMA(void);
DMA_HandleTypeDef* UART1_GetRxDMA(void);
volatile bool* UART1_GetRxFlag(void);
uint8_t* UART1_GetRxBuf(uint16_t *size);

#endif
