#ifndef __W25Q64_H
#define __W25Q64_H

#include "spi.h"
#include <stdint.h>

// CS 初始化为推挽输出, 高电平
// D0: MISO
// D1: MOSI
// 总大小: 64MBit -> 8MB: 8,388,608

#define W25Q64_INSTANCE                 SPI2

#define W25Q64_BUSY_BLOCK_MS            200            // 阻塞等待busy

#define W25Q64_MAX_ADDRESS_SPACE        0x7FFFFF
#define W25Q64_PAGE_SIZE                256            // 页大小：byte
#define W25Q64_PAGE_CNT                 32768          // 页数
#define W25Q64_SECTOR_SIZE              4096           // 扇区大小
#define W25Q64_SECTOR_CNT               2048           // 扇区数

void W25Q64_Init(void);
HAL_StatusTypeDef W25Q64_Check(void);
// 加个校验通信函数

HAL_StatusTypeDef W25Q64_WriteByte(uint16_t page, uint16_t addrInPage, const uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_WritePage(uint16_t page, const uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_WriteSector(uint16_t sector, const uint8_t *data, uint32_t size);

HAL_StatusTypeDef W25Q64_ReadByte(uint16_t page, uint16_t addrInPage, uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_ReadPage(uint16_t page, uint8_t *data, uint16_t size);
HAL_StatusTypeDef W25Q64_ReadSector(uint16_t sector, uint8_t *data, uint32_t size);

HAL_StatusTypeDef W25Q64_EraseSector(uint16_t sector);

#endif
