#include "stm32f1xx_hal.h"
#include "at24c64.h"
#include "i2c_s.h"

#include <stdbool.h>
#include <string.h>

static uint32_t timer = 0;

static HAL_StatusTypeDef isWriteBusy(bool isWrite)
{
    if ((HAL_GetTick() - timer) > AT24C64_WRITE_INTERVAL_MS) {
        // 如果是写操作，刷新时间。读操作不限制
        if (isWrite) {
            timer = HAL_GetTick();
        }
        return false;
    }
    return true;
}

/**
 * @brief 
 * 
 * @param memAddress 指定内存地址：范围 0x0000 - 0x1FFF
 * @param data 字节数据
 * @return HAL_StatusTypeDef 
 */

/**
 * @brief 指定地址，单字节写入
 * 
 * @param page 页索引：0 - 255
 * @param addrInPage 页内地址 0 - 31
 * @param data 单字节数据
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef AT24C64_Write_Byte(uint16_t page, uint8_t addrInPage, uint8_t data)
{
    return AT24C64_Write_Bytes(page, addrInPage, &data, 1);
}

/**
 * @brief 页内指定地址，连续多字节写入(禁止跨页)
 * 
 * @param memAddress 内存地址
 * @param data 数组指针
 * @param size 字节大小 1-32
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef AT24C64_Write_Bytes(uint16_t page, uint8_t addrInPage, const uint8_t *data, uint8_t size)
{
    if (size == 0)
        return HAL_ERROR;
    if (page >= AT24C64_PAGE_CNT)
        return HAL_ERROR;
    if ((addrInPage + size) > AT24C64_PAGE_SIZE)
        return HAL_ERROR;
    if (isWriteBusy(true))
        return HAL_BUSY;

    uint16_t memAddress = page * AT24C64_PAGE_SIZE + addrInPage;

    return I2C_Mem_Write(AT24C64_SLAVE_ADDR, memAddress, 2, data, size);
}

/**
 * @brief 整页字节写入
 * 
 * @param page 页索引 0-255
 * @param data 数组指针
 * @param size 字节大小 1-32
 */
HAL_StatusTypeDef AT24C64_Write_Page(uint16_t page, const uint8_t *data, uint8_t size)
{
    return AT24C64_Write_Bytes(page, 0, data, size);
}

/**
 * @brief 从指定的具体地址读连续字节
 * 
 * @param memAddress 起始内存地址
 * @param data 存放连续字节数据的地址
 * @param size 字节数据大小：0-0x1FFF(从指定地址开始读取的大小不得超过内存地址)
 */
HAL_StatusTypeDef AT24C64_Read_Byte(uint16_t page, uint8_t addrInPage, uint8_t *data, uint8_t size)
{
    if (page >= AT24C64_PAGE_CNT)
        return HAL_ERROR;
    if (size == 0 || size > AT24C64_MAX_READ_SIZE)
        return HAL_ERROR;
    if ((addrInPage + size) > AT24C64_MAX_ADDRESS_SPACE)
        return HAL_ERROR;
    if (isWriteBusy(false))
        return HAL_BUSY;

    uint16_t memAddress = page * AT24C64_PAGE_SIZE + addrInPage;

    return I2C_Mem_Read(AT24C64_SLAVE_ADDR, memAddress, 2, data, size);
}

// 整页读取
HAL_StatusTypeDef AT24C64_Read_Page(uint16_t page, uint8_t *data, uint8_t size)
{
    return AT24C64_Read_Byte(page, 0, data, size);
}

// 整页擦除
HAL_StatusTypeDef AT24C64_Erase_Page(uint16_t page)
{
    uint8_t buf[AT24C64_PAGE_SIZE] = { 0 };

    memset(buf, AT24C64_BLANK_BYTE, AT24C64_PAGE_SIZE);

    return AT24C64_Write_Page(page, buf, AT24C64_PAGE_SIZE);
}

void AT24C64_WaitWriteInterval(void)
{
    HAL_Delay(AT24C64_WRITE_INTERVAL_MS);
}
