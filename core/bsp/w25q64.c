#include "stm32f1xx_hal.h"
#include "w25q64.h"
#include "spi.h"

#include <stdint.h>
#include <stdbool.h>

#define W25Q64_CS_GPIO_Port     GPIOA
#define W25Q64_CS_Pin           GPIO_PIN_8

// 只能写0，不能写1。
// 写之前判断当前是不是全1，是全1，直接写
// 不是全1，读出扇区到内存，修改内存，擦除扇区，再写入扇区
// 1个扇区，16个页

// 创建一个扇区大小的缓冲数组，用于读改写
static uint8_t w25q64_Buf[W25Q64_SECTOR_SIZE];

#define CMD_WRITE_ENABLE                0x06
#define CMD_WRITE_DISABLE               0x04
#define CMD_READ_DATA                   0x03
#define CMD_PAGE_PROGRAM                0x02
#define CMD_SECTOR_ERASE                0x20
#define CMD_BLOCK_ERASE_32KB            0x52
#define CMD_BLOCK_ERASE_64KB            0xD8
#define CMD_READ_STATUS_REG_1           0x05
#define CMD_WRITE_STATUS_REG_1          0x01

static inline void W25Q64_CS_LOW(void);
static inline void W25Q64_CS_HIGH(void);
static inline HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size);
static inline HAL_StatusTypeDef receive(uint8_t *data, uint16_t size);
static void cmdTransmit(uint8_t cmd);
static bool isBusy(void);
static bool waitBusyTimeout(void);
static bool isNeedRMW(uint16_t page, uint16_t addrInPage, uint16_t size);
static HAL_StatusTypeDef writeDirectly(uint32_t addr, const uint8_t *data, uint16_t size);

static inline void W25Q64_CS_LOW(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_RESET);
}

static inline void W25Q64_CS_HIGH(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port, W25Q64_CS_Pin, GPIO_PIN_SET);
}

static inline HAL_StatusTypeDef transmit(const uint8_t *data, uint16_t size)
{
    return SPI_Transmit(W25Q64_INSTANCE, data, size);
}

static inline HAL_StatusTypeDef receive(uint8_t *data, uint16_t size)
{
    return SPI_Receive(W25Q64_INSTANCE, data, size);
}

static void cmdTransmit(uint8_t cmd)
{
    transmit(&cmd, 1);
}

static bool isBusy(void)
{
    uint8_t stateByte = 0;
    W25Q64_CS_LOW();
    cmdTransmit(CMD_READ_STATUS_REG_1);
    receive(&stateByte, 1);
    W25Q64_CS_HIGH();

    // 判断busy位是否为1(第0位)
    return READ_BIT(stateByte, 0x01);
}

/**
 * @brief 阻塞等待busy标志位置0
 * 
 * @return true 等待超时
 * @return false busy置0
 */
static bool waitBusyTimeout(void)
{
    uint32_t timer = HAL_GetTick();
    while (isBusy()) {
        if (HAL_GetTick() - timer > W25Q64_BUSY_BLOCK_MS)
            return true;
    }
    return false;
}

// 判断是否需要读改写
static bool isNeedRMW(uint16_t page, uint16_t addrInPage, uint16_t size)
{
    uint8_t tmp;

    for (uint16_t i = 0; i < size; i++) {
        W25Q64_ReadByte(page, addrInPage + i, &tmp, 1);
        if (tmp != 0xFF)
            return true;
    }
    return false;
}

// 直接写入函数
static HAL_StatusTypeDef writeDirectly(uint32_t addr, const uint8_t *data, uint16_t size)
{
    // 等上一次
    if (waitBusyTimeout())
        return HAL_BUSY;

    W25Q64_CS_LOW();
    cmdTransmit(CMD_WRITE_ENABLE);
    W25Q64_CS_HIGH();

    W25Q64_CS_LOW();
    cmdTransmit(CMD_PAGE_PROGRAM);
    uint8_t addrByte[3] = { addr >> 16, addr >> 8, addr };
    transmit(addrByte, 3);
    transmit(data, size);
    W25Q64_CS_HIGH();

    // 等本次
    if (waitBusyTimeout())
        return HAL_BUSY;

    return HAL_OK;
}

void W25Q64_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    W25Q64_CS_HIGH();

    GPIO_InitTypeDef gpio = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pin = W25Q64_CS_Pin,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_MEDIUM
    };

    HAL_GPIO_Init(W25Q64_CS_GPIO_Port, &gpio);

    SPI1_Init();
	SPI2_Init();
}

HAL_StatusTypeDef W25Q64_Check(void)
{
    uint8_t byte = 0;

    W25Q64_CS_LOW();
    cmdTransmit(0x9F);
    receive(&byte, 1);
    W25Q64_CS_HIGH();

    if(byte == 0xEF) 
        return HAL_OK;

    return HAL_ERROR;
}

/**
 * @brief 从某一页的指定地址开始写
 * 
 * @param page 页编号:0-32767
 * @param addrInPage 页内起始地址:0-255
 * @param data 字节数组
 * @param size 写入大小:1-256
 * @return HAL_StatusTypeDef 返回的状态
 */
HAL_StatusTypeDef W25Q64_WriteByte(uint16_t page, uint16_t addrInPage, const uint8_t *data, uint16_t size)
{
    if (page >= W25Q64_PAGE_CNT) return HAL_ERROR;
    if ((addrInPage + size) > W25Q64_PAGE_SIZE) return HAL_ERROR;

    // 记录返回状态，期间出错即返回
    HAL_StatusTypeDef res;
    // 写入的起始地址
    uint32_t startAddr = (uint32_t)page * W25Q64_PAGE_SIZE + addrInPage;
    // 扇区编号
    uint16_t sector = startAddr / W25Q64_SECTOR_SIZE;
    // 扇区地址
    uint32_t sectorAddr = (uint32_t)sector * W25Q64_SECTOR_SIZE;
    // 静态数组地址内偏移
    uint16_t offset = startAddr - sectorAddr;

    // 判断是否需要读改写
    if (isNeedRMW(page, addrInPage, size)) {
        // 读整个扇区
        res = W25Q64_ReadSector(sector, w25q64_Buf, W25Q64_SECTOR_SIZE);
        if (res != HAL_OK)
            return res;
        // 修改缓冲
        for (uint16_t i = 0; i < size; i++)
            w25q64_Buf[offset + i] = data[i];
        // 擦除扇区
        res = W25Q64_EraseSector(sector);
        if (res != HAL_OK)
            return res;
        // 写回整个扇区
        res = W25Q64_WriteSector(sector, w25q64_Buf, W25Q64_SECTOR_SIZE);

        return res;
    }

    // 不需要读改写
    return writeDirectly(startAddr, data, size);
}

/**
 * @brief 从页起始地址开始写
 * 
 * @param page 页编号
 * @param data 字节数组
 * @param size 写入大小
 * @return HAL_StatusTypeDef 返回的状态
 */
HAL_StatusTypeDef W25Q64_WritePage(uint16_t page, const uint8_t *data, uint16_t size)
{
    return W25Q64_WriteByte(page, 0, data, size);
}

/**
 * @brief 从指定扇区开始写, 不带擦除
 * 
 * @param sector 扇区号：0 - 2047
 * @param data 数组
 * @param size 大小：1 - 8,388,608
 * @return HAL_StatusTypeDef 状态
 */
HAL_StatusTypeDef W25Q64_WriteSector(uint16_t sector, const uint8_t *data, uint32_t size)
{
    if (sector >= W25Q64_SECTOR_CNT)
        return HAL_ERROR;
    if (size > W25Q64_SECTOR_SIZE * W25Q64_SECTOR_CNT)
        return HAL_ERROR;

    HAL_StatusTypeDef res;
    uint32_t addr = (uint32_t)sector * W25Q64_SECTOR_SIZE;

    // 一页一页写，每次256字节
    for (uint16_t i = 0; i < size; i += W25Q64_PAGE_SIZE) {
        // 计算剩下大小是否不足一页
        uint16_t writeSize = (size - i) > W25Q64_PAGE_SIZE ? W25Q64_PAGE_SIZE : (size - i);
        res = writeDirectly(addr + i, data + i, writeSize);
        if (res != HAL_OK)
            return res;
    }

    return HAL_OK;
}

// 从页内的指定地址开始读
HAL_StatusTypeDef W25Q64_ReadByte(uint16_t page, uint16_t addrInPage, uint8_t *data, uint16_t size)
{
    if (page >= W25Q64_PAGE_CNT)
        return HAL_ERROR;
    if ((size + addrInPage) > W25Q64_PAGE_SIZE)
        return HAL_ERROR;
    if (waitBusyTimeout())
        return HAL_BUSY;

    uint32_t addr = (uint32_t)page * W25Q64_PAGE_SIZE + addrInPage;
    uint8_t addrByte[3] = { addr >> 16, addr >> 8, addr };
    HAL_StatusTypeDef state;

    W25Q64_CS_LOW();
    cmdTransmit(CMD_READ_DATA);
    transmit(addrByte, 3);
    state = receive(data, size);
    W25Q64_CS_HIGH();

    return state;
}

// 从页起始地址开始读
HAL_StatusTypeDef W25Q64_ReadPage(uint16_t page, uint8_t *data, uint16_t size)
{
    return W25Q64_ReadByte(page, 0, data, size);
}

/**
 * @brief 指定字节大小, 按照扇区读
 * 
 * @param sector 扇区号：0-2047
 * @param data 字节数组
 * @param size 数据大小：1 - 8,388,608
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef W25Q64_ReadSector(uint16_t sector, uint8_t *data, uint32_t size)
{
    if (sector >= W25Q64_SECTOR_CNT)
        return HAL_ERROR;
    if (size > W25Q64_SECTOR_SIZE * W25Q64_SECTOR_CNT)
        return HAL_ERROR;
    if (waitBusyTimeout())
        return HAL_BUSY;

    uint32_t sectorAddr = (uint32_t)sector * W25Q64_SECTOR_SIZE;
    uint8_t addrByte[3] = { (uint8_t)(sectorAddr >> 16), (uint8_t)(sectorAddr >> 8), (uint8_t)(sectorAddr) };
    HAL_StatusTypeDef state;

    W25Q64_CS_LOW();
    cmdTransmit(CMD_READ_DATA);
    transmit(addrByte, 3);
    state = receive(data, size);
    W25Q64_CS_HIGH();

    return state;
}

/**
 * @brief 扇区擦除
 * 
 * @param sector 扇区号：0-2047
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef W25Q64_EraseSector(uint16_t sector)
{
    if (sector >= W25Q64_SECTOR_CNT)
        return HAL_ERROR;
    if (waitBusyTimeout())
        return HAL_BUSY;

    uint32_t addr = (uint32_t)sector * W25Q64_SECTOR_SIZE;
    uint8_t addrByte[3] = { addr >> 16, addr >> 8, addr };
    HAL_StatusTypeDef state;

    W25Q64_CS_LOW();
    cmdTransmit(CMD_WRITE_ENABLE);
    W25Q64_CS_HIGH();

    W25Q64_CS_LOW();
    cmdTransmit(CMD_SECTOR_ERASE);
    state = transmit(addrByte, 3);
    W25Q64_CS_HIGH();

    if (waitBusyTimeout())
        return HAL_BUSY;

    return state;
}
