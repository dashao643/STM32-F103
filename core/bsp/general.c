#include "general.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UART1_PRINTF

#ifdef UART1_PRINTF
#include "uart1.h"
#endif

// clang-format off
const char monthTable3c[12][4] = {
    { "Jan" }, { "Feb" }, { "Mar" }, 
    { "Apr" }, { "May" }, { "Jun" }, 
    { "Jul" }, { "Aug" }, { "Sep" }, 
    { "Oct" }, { "Nov" }, { "Dec" },
};
// clang-format on

/**
 * @brief 微秒级延时，最长900us
 *
 * @param us 微秒
 */
void Delay_Us(__IO uint32_t delay)
{
    int last, cur, val;
    int temp;

    while (delay != 0) {
        temp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        cur = last - CLOCK_FREQUENCY_MHZ * temp;
        if (cur >= 0) {
            do
                val = SysTick->VAL;
            while ((val < last) && (val >= cur));
        } else {
            cur += CLOCK_FREQUENCY_MHZ * 1000;
            do
                val = SysTick->VAL;
            while ((val <= last) || (val > cur));
        }
        delay -= temp;
    }
}

#if defined(__GNUC__)
// ========== CMake / GCC 编译器用 ==========
#include <sys/stat.h>

int _write(int file, char *ptr, int len)
{
#ifdef UART1_PRINTF
    HAL_UART_Transmit(UART1_GetHandle(), (uint8_t *)ptr, len, UART1_TX_TIMEOUT_MS);
#endif
    return len;
}

void *_sbrk(int incr) {
    extern char _end;
    static char *heap_end;
    char *prev;

    if (heap_end == NULL)
        heap_end = &_end;
    prev = heap_end;
    heap_end += incr;
    return (void *)prev;
}

int _close(int file)   { (void)file; return -1; }
int _fstat(int file, struct stat *st) { (void)file; (void)st; return 0; }
int _isatty(int file)  { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
void _exit(int status) { (void)status; while(1); }

#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
// ========== MDK-ARM 编译器用 ==========
/* 关闭半主机模式 */
#pragma import(__use_no_semihosting)

/* 标准库需要的文件结构 */
struct __FILE
{
    int handle;
};

int _ttywrch(int ch)
{
    return ch;
}

/* 避免半主机的空函数 */
void _sys_exit(int x)
{
    (void)x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

FILE __stdout;

/* 重定向 fputc —— printf 最终会调用这个函数 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(UART1_GetHandle(), (uint8_t *)&ch, 1, UART1_TX_TIMEOUT_MS);

    return ch;
}
#endif

// Modbus CRC16 标准查表
const uint16_t CRC16_TABLE[256] = { 
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241, 
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440, 
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40, 
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841, 
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40, 
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41, 
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641, 
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040, 
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240, 
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441, 
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41, 
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840, 
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41, 
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40, 
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640, 
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041, 
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240, 
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441, 
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840, 
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41, 
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40, 
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640, 
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041, 
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241, 
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440, 
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40, 
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841, 
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40, 
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41, 
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641, 
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 
};

uint16_t CRC16_Modbus(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    
    while (len--) {
        crc ^= *buf++;
        crc = CRC16_TABLE[crc & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

uint8_t CRC8_Maxim(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;

    while (len--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x01)
                crc = (crc >> 1) ^ 0x8C; // 多项式 0x8C (0x31反转后)
            else
                crc >>= 1;
        }
    }
    return crc;
}

// 设置中断向量表偏移
void NVIC_SetVectorTable(uint32_t offset)
{
    SCB->VTOR = offset;
    __enable_irq();
}

// 4位整数转成字符串显示(X.XXX)
void IntToString_1(int32_t val, char *str, uint8_t strlen)
{
    snprintf(str, strlen, "%ld.%03ld", val / 1000, val % 1000);
}

// 4位整数转成字符串显示(XX.XX)
void IntToString_2(int32_t val, char *str, uint8_t strlen)
{
    int32_t integer = val / 100;
    uint32_t decimal = abs(val) % 100;
    snprintf(str, strlen, "%ld.%02ld", integer, decimal);
}

/**
 * @brief 月份字符串转整数，对比前三个字符，失败返回-1
 *
 * @param monthStr 月份字符串
 * @param size 字符串大小，至少为3
 * @return int8_t int8_t 月份整数
 */
int8_t monthMatch3c(const char *monthStr, uint8_t size)
{
    if (size < 3)
        return -2;

    for (uint8_t i = 0; i < 12; i++) {
        if (strncmp(monthTable3c[i], monthStr, 3) == 0)
            return i + 1;
    }
    return -1;
}