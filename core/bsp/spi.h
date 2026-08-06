#ifndef __SPI_H__
#define __SPI_H__

#include "stm32f1xx_hal.h"

// #define ENABLE_SPI1
// #define REMAP_SPI1

#define ENABLE_SPI2

#define SPI_TIMEOUT_MS      200

void SPI1_Init(void);
void SPI2_Init(void);
void SPI1_MspInit(SPI_HandleTypeDef *hspi);
void SPI2_MspInit(SPI_HandleTypeDef *hspi);

HAL_StatusTypeDef SPI_Transmit(SPI_TypeDef *Instance, const uint8_t *data, uint16_t size);
HAL_StatusTypeDef SPI_Receive(SPI_TypeDef *Instance, uint8_t *data, uint16_t size);

#endif
