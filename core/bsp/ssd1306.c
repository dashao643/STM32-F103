#include "stm32f1xx_hal.h"
#include "ssd1306.h"
#include "ssd1306_font.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// #ifdef I2C_HARDWARE
// #define SSD1306_INSTANCE            I2C1
// #endif

#if defined I2C_HARDWARE
#include "i2c1.h"
#elif defined I2C_SOFTWARE
#include "i2c_s.h"
#endif

#define ASCII_OFFSET                0x20
#define ASCII_LENGTH                95

#define SSD1306_I2C_SLAVE_ADDR      0x78
#define SSD1306_I2C_CMD             0x00    // 写命令
#define SSD1306_I2C_DATA            0x40    // 写数据

static void writeCmd(uint8_t cmd);
static void writeData(const uint8_t data[], uint16_t length);
static void writeCmdPos(uint8_t row, uint8_t col, uint8_t pageOffs);

static void writeCmd(uint8_t cmd)
{
#ifdef I2C_SOFTWARE
    I2C_Mem_Write(SSD1306_I2C_SLAVE_ADDR, SSD1306_I2C_CMD, 1, &cmd, 1);
#endif

#ifdef I2C_HARDWARE
    I2C1_Mem_Write(SSD1306_I2C_SLAVE_ADDR, SSD1306_I2C_CMD, 1, &cmd, 1);
#endif
}

static void writeData(const uint8_t data[], uint16_t length)
{
#ifdef I2C_SOFTWARE
    I2C_Mem_Write(SSD1306_I2C_SLAVE_ADDR, SSD1306_I2C_DATA, 1, data, length);
#endif

#ifdef I2C_HARDWARE
    I2C1_Mem_Write(SSD1306_I2C_SLAVE_ADDR, SSD1306_I2C_DATA, 1, data, length);
#endif
}

static void writeCmdPos(uint8_t row, uint8_t col, uint8_t pageOffs)
{
    uint8_t page = (row - 1) * 2 + pageOffs;
    uint8_t colIdx = (col - 1) * 8;

    if (page > 7)
        page = 7;
    if (colIdx > 127)
        colIdx = 127;

    writeCmd(0xB0 + page);            // 指定页地址
    writeCmd(0x00 + (colIdx & 0x0F)); // 列地址取低四位
    writeCmd(0x10 + (colIdx >> 4));   // 列地址取高四位
}

/*-----------------------------------------------------------------*/

void SSD1306_Init(void)
{
#if defined I2C_HARDWARE
    I2C1_Init();
#elif defined I2C_SOFTWARE
    I2C_Init();
#endif
    HAL_Delay(100);

    writeCmd(0xAE); // 关闭显示
    writeCmd(0xD5); // 设置时钟分频因子,震荡频率
    writeCmd(0x80); // 分频因子=1,震荡频率=默认
    writeCmd(0xA8); // 设置多路复用率
    writeCmd(0x3F); // 1/64 Duty
    writeCmd(0xD3); // 设置显示偏移
    writeCmd(0x00); // 偏移0
    writeCmd(0x40); // 设置显示开始行
    writeCmd(0x8D); // 电荷泵设置
    writeCmd(0x14); // 开启电荷泵
    writeCmd(0x20); // 设置内存地址模式
    writeCmd(0x02); // 页面寻址模式
    writeCmd(0xA1); // 段重定义设置,SEG0->列0
    writeCmd(0xC8); // COM扫描方向,COM63->行0
    writeCmd(0xDA); // 设置COM硬件引脚配置
    writeCmd(0x12); //
    writeCmd(0x81); // 对比度设置
    writeCmd(0x7F); // 对比度值
    writeCmd(0xD9); // 设置预充电周期
    writeCmd(0xF1); //
    writeCmd(0xDB); // 设置VCOMH电压倍率
    writeCmd(0x40); //
    writeCmd(0xA4); // 全局显示开启
    writeCmd(0xA6); // 正常显示
    writeCmd(0xAF); // 开启显示

    SSD1306_Clear();
}

void SSD1306_Clear(void)
{
    uint8_t data[128] = { 0 };

    for (uint8_t i = 0; i < 8; i++) {
        writeCmd(0xB0 + i);
        writeCmd(0x00);
        writeCmd(0x10);
        writeData(data, sizeof(data));
    }
}

void SSD1306_ShowALL(void)
{
    uint8_t line[128] = { 0 };

    for (uint8_t i = 0; i < 8; i++) {
        writeCmd(0xB0 + i);
        writeCmd(0x00);
        writeCmd(0x10);
        // memset按字节设置
        memset(line, 0xFF, sizeof(line));
        writeData(line, sizeof(line));
    }
}

void SSD1306_SetReverse(void)
{
    writeCmd(0xA7);
}

/**
 * @brief 显示字符
 * 
 * @param row 行号，1-4
 * @param col 列号，1-16
 * @param ch 字符
 */
void SSD1306_ShowChar(uint8_t row, uint8_t col, char ch)
{
    if (row == 0 || row > 4) return;
    if (col == 0 || col > 16) return;

    uint8_t *chIdx;

    // 范围有误，显示为 ◼
    if ((ch < ASCII_OFFSET) || (ch > ASCII_LENGTH - 1 + ASCII_OFFSET))
        chIdx = (uint8_t *)SSD1306_ERROR_ASCII;
    else
        chIdx = (uint8_t *)SSD1306_ASCII_08x16[ch - ASCII_OFFSET];

    writeCmdPos(row, col, 0);
    writeData(chIdx, 8);

    writeCmdPos(row, col, 1);
    writeData(chIdx + 8, 8);
}

void SSD1306_ShowString(uint8_t row, uint8_t col, const char font[])
{
    if (font == NULL) return;
    if (row == 0 || row > 4) return;
    if (col == 0 || col > 16) return;

    while (*font != '\0') {
        if (col > 16) return;

        // 先看第 bit7 是否为 0, 若为 0 则以 ascii 字符显示
        if (!(*font & 0x80)) {
            SSD1306_ShowChar(row, col, *font);
            col++;
            font++;
            continue;
        }

        uint8_t cellCnt = 0;

#if defined SSD1306_FONT_UTF8
        // 统计 utf8 中文所占字节数
        for (uint8_t i = 4; i < 8; i++) {
            if ((font[0] >> i) & 0x01) {
                cellCnt = 8 - i;
                break;
            }
        }
#elif defined SSD1306_FONT_GB2312
        // GB2312 中文字节数固定为2
        cellCnt = 2;
#endif
        bool find = false;

        for (uint8_t i = 0; i < CHINESE_FONT_COUNT; i++) {
            // 此文字匹配数组, 显示
            if (memcmp(font, SSD1306_CHINESE_16x16[i].index, cellCnt) == 0) {
                writeCmdPos(row, col, 0);
                writeData(SSD1306_CHINESE_16x16[i].cell, 16);

                writeCmdPos(row, col, 1);
                writeData(SSD1306_CHINESE_16x16[i].cell + 16, 16);

                find = true;
                break;
            }
        }
        // 没找到文字, 显示为 ■
        if (!find) {
            writeCmdPos(row, col, 0);
            writeData(SSD1306_ERROR_FONT, 16);

            writeCmdPos(row, col, 1);
            writeData(SSD1306_ERROR_FONT + 16, 16);
        }
        // font 指针偏移 cellCnt 字节
        font += cellCnt;
        // 移动列指针
        col += 2;
    }
}

// 需要传入数据的位数，从低位开始显示，目的是更新显示区域 numLen: 1 - 11
void SSD1306_ShowDecNumber(uint8_t row, uint8_t col, int32_t number, uint8_t numLen)
{
    if (row == 0 || row > 4) return;
    if (col == 0 || col > 16) return;
    if (numLen == 0) return;

    if (numLen > 11) 
        numLen = 11;

    char buf[12] = { 0 }; // int32_t 最大值 + 负号 + \0

    snprintf(buf, sizeof(buf), "%0*d", numLen, (int)number);

    SSD1306_ShowString(row, col, buf);
}

/**
 * @brief 按字节显示
 * 
 * @param row 行号，1-4
 * @param col 列号，1-16
 * @param data 原始字节数组
 * @param size 数组大小 1-5 Byte
 */
void SSD1306_ShowHexNumber(uint8_t row, uint8_t col, const uint8_t data[], uint8_t size)
{
    if (row == 0 || row > 4) return;
    if (col == 0 || col > 16) return;
    if (size == 0) return;

    if (size > 5)
        size = 5;

    char buf[3] = { 0 };
    for (int i = 0; i < size; i++) {
        snprintf(buf, sizeof(buf), "%02X", data[i]);
        SSD1306_ShowString(row, col + (i * 3), buf);
    }
}

void SSD1306_ShowImage(const uint8_t *image)
{
    for (uint8_t row = 0; row < 8; row++) {
        writeCmd(0xB0 + row);
        writeCmd(0x00);
        writeCmd(0x10);

        writeData(image + row * 128, 128);
    }
}

// void SSD1306_StrWriteLine(uint8_t row, const char str[])
// {
//     uint8_t topRow[128] = { 0 };
//     uint8_t bottomRow[128] = { 0 };

//     for (uint8_t col = 0; col < 16; col++) {
//         char ch = str[col];
//         if (ch == 0)
//             break;

//         uint8_t index = col * 8;
//         // 放上半字符
//         for (uint8_t j = 0; j < 8; j++) {
//             topRow[index + j] = SSD1306_ASCII_08x16[ch - ASCII_OFFSET][j];
//         }
//         // 放下半字符
//         for (uint8_t j = 0; j < 8; j++) {
//             bottomRow[index + j] = SSD1306_ASCII_08x16[ch - ASCII_OFFSET][j + 8];
//         }
//     }

//     writeCmdPos(row, 0, 0);
//     writeData(topRow, 128);
//     writeCmdPos(row, 0, 1);
//     writeData(bottomRow, 128);
// }