#ifndef __GENERAL_H__
#define __GENERAL_H__

#include "stm32f1xx_hal.h"

#define PIN_HIGH                GPIO_PIN_SET
#define PIN_LOW                 GPIO_PIN_RESET

#define CLOCK_FREQUENCY_MHZ     72

#define BOOTLOADER_SIZE         0x4000                          // 16KB
#define NVIC_OFFSET             (FLASH_BASE | BOOTLOADER_SIZE)  // F1: 0x08004000

#define UNUSED_FUNC             __attribute__((unused))

typedef union {
  uint8_t bytes[2];
  uint16_t word;
} U16Union;

typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
} GPIO_PortPinTypeDef;

void Delay_Us(__IO uint32_t delay);
uint16_t CRC16_Modbus(const uint8_t *buf, uint16_t len);
uint8_t CRC8_Maxim(const uint8_t *data, uint16_t len);
void NVIC_SetVectorTable(uint32_t offset);
void IntToString_1(int32_t val, char *str, uint8_t strlen);
void IntToString_2(int32_t val, char *str, uint8_t strlen);
int8_t monthMatch3c(const char *monthStr, uint8_t size);

#endif
