#ifndef __UART1_H__
#define __UART1_H__

#include "stm32f1xx_hal_def.h"

#define UART1_RX_BUFF_MAX_SIZE               256     // 接收最大帧长度
#define UART1_TX_BUFF_MAX_SIZE               256     // 发送最大帧长度
// #define UART1_RX_BUFF_MINLENTH          8       // 最小帧长度

// typedef enum {
//   BLOCK = 0,
//   DMA,
// } TransmitMode;

HAL_StatusTypeDef UART1_Init(void);
HAL_StatusTypeDef UART1_Transmit(const uint8_t *data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef UART1_Transmit_DMA(const uint8_t *data, uint16_t size);



#endif
