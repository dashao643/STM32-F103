#include "stm32f1xx_hal.h"
#include "at24c64.h"
#include "i2c_s.h"

#include <stdbool.h>
#include <string.h>

static uint32_t timer = 0;

static HAL_StatusTypeDef isWriteBusy(bool isWrite)
{
    if ((HAL_GetTick() - timer) > AT24C64_WRITE_INTERVAL_MS) {
        // 写操作刷新时间, 读操作不限制
        if (isWrite)
            timer = HAL_GetTick();
        return HAL_OK;
    }
    return HAL_BUSY;
}

/*-----------------------------------------------------------------*/

void AT24C64_Init(void)
{
    I2C_Init();
}

// #include <stdio.h>
HAL_StatusTypeDef AT24C64_Check(void)
{
    HAL_StatusTypeDef state = HAL_OK;
    uint8_t origin = 0;

    state = AT24C64_ReadByte(0, 0, &origin, 1);
    // printf("state=%d\n", state);
    // printf("origin=%d\n", origin);
    
    if(state != HAL_OK) return state;

    uint8_t writeTest = 0xBB;
    state = AT24C64_WriteByte(0, 0, writeTest);
    // if(state != HAL_OK) return state;
    AT24C64_WaitWriteInterval();

    uint8_t readTest = 0;
    AT24C64_ReadByte(0, 0, &readTest, 1);

    // printf("readTest=%d\n", readTest);

    if(readTest != writeTest)
        state = HAL_ERROR;

    AT24C64_WriteByte(0, 0, origin);

    return state;
}

/**
 * @brief 指定地址, 单字节写入
 *
 * @param page 页索引: 0 - 255
 * @param addrInPage 页内地址: 0 - 31
 * @param data 单字节数据
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef AT24C64_WriteByte(uint16_t page, uint8_t addrInPage, uint8_t data)
{
    return AT24C64_WriteBytes(page, addrInPage, &data, 1);
}

/**
 * @brief 页内指定地址，连续多字节写入(不支持跨页)
 * 
 * @param page 页索引: 0 - 255
 * @param addrInPage 页内地址: 0 - 31
 * @param data 字节数组
 * @param size 数组长度
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef AT24C64_WriteBytes(uint16_t page, uint8_t addrInPage, const uint8_t *data, uint8_t size)
{
    if (size == 0)
        return HAL_ERROR;
    if (page >= AT24C64_PAGE_CNT)
        return HAL_ERROR;
    if ((addrInPage + size) > AT24C64_PAGE_SIZE)
        return HAL_ERROR;
    if (isWriteBusy(true) == HAL_BUSY)
        return HAL_BUSY;

    uint16_t memAddress = page * AT24C64_PAGE_SIZE + addrInPage;

    return I2C_Mem_Write(AT24C64_SLAVE_ADDR, memAddress, 2, data, size);
}

/**
 * @brief 整页字节写入
 * 
 * @param page 页索引 0-255
 * @param data 字节数组
 * @param size 字节大小 1-32
 */
HAL_StatusTypeDef AT24C64_WritePage(uint16_t page, const uint8_t *data, uint8_t size)
{
    return AT24C64_WriteBytes(page, 0, data, size);
}

/**
 * @brief 从指定的具体地址读连续字节
 * 
 * @param page 页索引 0-255
 * @param addrInPage 页内地址: 0 - 31
 * @param data 字节数组
 * @param size 数组大小
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef AT24C64_ReadByte(uint16_t page, uint8_t addrInPage, uint8_t *data, uint8_t size)
{
    if (page >= AT24C64_PAGE_CNT)
        return HAL_ERROR;
    if (size == 0 || size > AT24C64_MAX_READ_SIZE)
        return HAL_ERROR;
    if ((addrInPage + size) > AT24C64_MAX_ADDRESS_SPACE)
        return HAL_ERROR;
    if (isWriteBusy(false) == HAL_BUSY)
        return HAL_BUSY;

    uint16_t memAddress = page * AT24C64_PAGE_SIZE + addrInPage;

    return I2C_Mem_Read(AT24C64_SLAVE_ADDR, memAddress, 2, data, size);
}

// 整页读取
HAL_StatusTypeDef AT24C64_ReadPage(uint16_t page, uint8_t *data, uint8_t size)
{
    return AT24C64_ReadByte(page, 0, data, size);
}

// 整页擦除
HAL_StatusTypeDef AT24C64_ErasePage(uint16_t page)
{
    uint8_t buf[AT24C64_PAGE_SIZE] = { 0 };

    memset(buf, AT24C64_BLANK_BYTE, AT24C64_PAGE_SIZE);

    return AT24C64_WritePage(page, buf, AT24C64_PAGE_SIZE);
}

void AT24C64_WaitWriteInterval(void)
{
    HAL_Delay(AT24C64_WRITE_INTERVAL_MS);
}
