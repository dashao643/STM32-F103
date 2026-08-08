#ifndef __AT24C64_H__
#define __AT24C64_H__

#include "stm32f1xx_hal_def.h"

#include <stdint.h>

// 写入完成后, 后续写入手动调用 WaitWriteInterval 等待写入完成

#define AT24C64_WRITE_INTERVAL_MS     5         // 执行写入操作后的间隔时间 

#define AT24C64_SLAVE_ADDR            0xA0      // 从机地址：0B1010 0000
#define AT24C64_MAX_ADDRESS_SPACE     0x1FFF    // 最大寻址空间：0B0001 1111 1111 1111
#define AT24C64_PAGE_SIZE             0x20      // 页大小：32 字节
#define AT24C64_PAGE_CNT              0x100     // 页数量：256
#define AT24C64_MAX_READ_SIZE         128       // 最大数据长度(4页大小)
#define AT24C64_BLANK_BYTE            0xFF      // 空白字节

void AT24C64_Init(void);
HAL_StatusTypeDef AT24C64_Check(void);

HAL_StatusTypeDef AT24C64_WriteByte(uint16_t page, uint8_t addrInPage, uint8_t data);
HAL_StatusTypeDef AT24C64_WriteBytes(uint16_t page, uint8_t addrInPage, const uint8_t *data, uint8_t size);
HAL_StatusTypeDef AT24C64_WritePage(uint16_t page, const uint8_t *data, uint8_t size);

HAL_StatusTypeDef AT24C64_ReadByte(uint16_t page, uint8_t addrInPage, uint8_t *data, uint8_t size);
HAL_StatusTypeDef AT24C64_ReadPage(uint16_t page, uint8_t *data, uint8_t size);

HAL_StatusTypeDef AT24C64_ErasePage(uint16_t page);

void AT24C64_WaitWriteInterval(void);

#endif
