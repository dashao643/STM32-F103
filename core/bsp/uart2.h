#ifndef __UART2_H__
#define __UART2_H__

#include "stm32f1xx_hal.h"

#include <stdbool.h>

#define UART2_RX_BUFF_MAX_SIZE               256     // 接收最大帧长度
#define UART2_TX_BUFF_MAX_SIZE               256     // 发送最大帧长度
#define UART2_TX_TIMEOUT_MS                  500

typedef struct {
    volatile uint8_t rxFlag;                    // 接收到数据次数
    uint8_t rxBuf[UART2_RX_BUFF_MAX_SIZE];      // 接收缓冲区
    volatile uint16_t size;                     // 累计接收大小
} UART2_RxTypeDef;

void UART2_Init(void);
void UART2_MspInit(UART_HandleTypeDef *huart);

HAL_StatusTypeDef UART2_Transmit(const uint8_t *data, uint16_t size);
HAL_StatusTypeDef UART2_Transmit_DMA(const uint8_t *data, uint16_t size);

UART_HandleTypeDef* UART2_GetHandle(void);
UART2_RxTypeDef* UART2_GetRxStruct(void);
void UART2_GetRxClear(void);

// void UART2_Task(void);

#endif
