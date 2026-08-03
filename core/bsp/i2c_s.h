#ifndef __I2C_S_H__
#define __I2C_S_H__

#include "stm32f1xx_hal_def.h"

#include <stdio.h>

void I2C_Init(void);

HAL_StatusTypeDef I2C_Mem_Write(uint8_t devAddress, uint16_t memAddress, uint8_t memAddSize, 
                                const uint8_t *data, uint16_t size);
HAL_StatusTypeDef I2C_Mem_Read(uint8_t devAddress, uint16_t memAddress, uint8_t memAddSize, 
                               uint8_t *data, uint16_t size);

#endif
