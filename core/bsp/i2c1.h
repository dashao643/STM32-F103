#ifndef __I2C1_H__
#define __I2C1_H__

#include "stm32f1xx_hal_i2c.h"

#include <stdint.h>

void I2C1_Init(void);
void I2C1_MspInit(I2C_HandleTypeDef* hi2c);

HAL_StatusTypeDef I2C1_Mem_Write(uint16_t devAddress, uint16_t memAddress, 
    uint16_t memAddSize, const uint8_t *data, uint16_t size);
HAL_StatusTypeDef I2C1_Mem_Read(uint16_t devAddress, uint16_t memAddress, 
    uint16_t memAddSize, uint8_t *data, uint16_t size);

#endif
