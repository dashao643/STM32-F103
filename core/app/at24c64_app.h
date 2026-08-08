#ifndef __AT24C64_ADDR_H__
#define __AT24C64_ADDR_H__

#include "stm32f1xx_hal_def.h"

#include <stdint.h>
#include <stdbool.h>

// at24c64 地址映射表

/******************** 数据页索引：255 ********************/
#define MY_DATA_PAGE                 255
#define USER_ID_ADDR                 0
#define USER_ID_SIZE                 6

#define USER_PASSWORD_ADDR           6
#define USER_PASSWORD_SIZE           6

#define POWER_ON_COUNT_ADDR          12    // 开机次数
#define POWER_ON_COUNT_SIZE          2

/******************** WiFi ID页索引：254 ********************/
#define WIFI_SSID_PAGE               254   // ssid最大长度为32字节
#define WIFI_SSID_SIZE               32

/******************** WiFi密码页索引：253 ********************/
#define WIFI_PASSWORD_PAGE           253   // pwd最大长度为32字节
#define WIFI_PASSWORD_SIZE           32

void AT24C64_App_Add(void);

void AT24C64_App_ReadUserId(void);
void AT24C64_App_ReadUserPassword(void);

void AT24C64_App_ReadWiFiSSID(char *ssid, uint8_t maxLen);
void AT24C64_App_ReadWiFiPassword(char *pwd, uint8_t maxLen);
HAL_StatusTypeDef AT24C64_App_WriteWiFiSSID(const char *ssid, uint8_t size);
HAL_StatusTypeDef AT24C64_App_WriteWiFiPassword(const char *pwd, uint8_t size);

#endif
