#include "stm32f1xx_hal.h"
#include "at24c64_app.h"
#include "at24c64.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// 上电时调用, 记录总上电次数, 最大 65535
void AT24C64_App_Add(void)
{
    uint8_t buf[2] = { 0 };
    AT24C64_ReadByte(MY_DATA_PAGE, POWER_ON_COUNT_ADDR, buf, POWER_ON_COUNT_SIZE);

    uint16_t cnt = ((uint16_t)buf[0] << 8) | buf[1];

    if (cnt == 0xFFFF)
        cnt = 0;

    cnt++;

    printf("powerOnCnt=%d\n", cnt);

    buf[0] = cnt >> 8;
    buf[1] = cnt;

    AT24C64_WriteBytes(MY_DATA_PAGE, POWER_ON_COUNT_ADDR, buf, POWER_ON_COUNT_SIZE);
}

void AT24C64_App_ReadUserId(void)
{
    uint8_t readData[USER_ID_SIZE] = { 0 };

    AT24C64_ReadByte(MY_DATA_PAGE, USER_ID_ADDR, readData, USER_ID_SIZE);

    printf("%s\n", readData);
}

void AT24C64_App_ReadUserPassword(void)
{
    uint8_t readData[USER_PASSWORD_SIZE] = { 0 };

    AT24C64_ReadByte(MY_DATA_PAGE, USER_PASSWORD_ADDR, readData, USER_ID_SIZE);

    printf("%s\n", readData);
}

// 定义至少33字节数组接收
void AT24C64_App_ReadWiFiSSID(char *ssid, uint8_t maxLen)
{
    if (maxLen < (WIFI_SSID_SIZE + 1))
        return;

    AT24C64_ReadPage(WIFI_SSID_PAGE, (uint8_t *)ssid, WIFI_SSID_SIZE);
    ssid[maxLen - 1] = 0;
}

// 定义至少33字节数组接收
void AT24C64_App_ReadWiFiPassword(char *pwd, uint8_t maxLen)
{
    if (maxLen < (WIFI_SSID_SIZE + 1))
        return;

    AT24C64_ReadPage(WIFI_PASSWORD_PAGE, (uint8_t *)pwd, WIFI_PASSWORD_SIZE);
    pwd[maxLen - 1] = 0;
}

// ssid最大支持32字节,字符串传入\0
HAL_StatusTypeDef AT24C64_App_WriteWiFiSSID(const char *ssid, uint8_t size)
{
    if (size > (WIFI_SSID_SIZE + 1))
        return false;

    uint8_t state;
    // ssid刚好为32字节，一页大小，写入一页，不存\0
    if (size == (WIFI_SSID_SIZE + 1))
        state = AT24C64_WritePage(WIFI_SSID_PAGE, (uint8_t *)ssid, WIFI_SSID_SIZE);
    // ssid小于32字节，写入最后\0
    else
        state = AT24C64_WritePage(WIFI_SSID_PAGE, (uint8_t *)ssid, size);

    return state;
}

// pwd最大支持32字节
HAL_StatusTypeDef AT24C64_App_WriteWiFiPassword(const char *pwd, uint8_t size)
{
    if (size > (WIFI_SSID_SIZE + 1))
        return false;

    uint8_t state;
    if (size == (WIFI_SSID_SIZE + 1))
        state = AT24C64_WritePage(WIFI_PASSWORD_PAGE, (uint8_t *)pwd, WIFI_SSID_SIZE);
    else
        state = AT24C64_WritePage(WIFI_PASSWORD_PAGE, (uint8_t *)pwd, size);

    return state;
}