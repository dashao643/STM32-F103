#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_def.h"
#include "esp8266_app.h"
#include "at24c64_app.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "led.h"
#include "dht11.h"
#include "ds18b20.h"

/********************* 命令指令 *******************/
static void Fun_LedRedToggle(void)
{
    LED_RED_TOGGLE();
}
static void Fun_LedGreenToggle(void)
{
    LED_GREEN_TOGGLE();
}
static void Fun_LedBlueToggle(void)
{
    LED_BLUE_TOGGLE();
}
static void Fun_LedAllToggle(void)
{
    LED_RED_TOGGLE();
    LED_GREEN_TOGGLE();
    LED_BLUE_TOGGLE();
}

CmdTable_t cmdTable[] = { 
    { "CMD_LED_RED_TOGGLE", Fun_LedRedToggle },            
    { "CMD_LED_GREEN_TOGGLE", Fun_LedGreenToggle },   
    { "CMD_LED_BLUE_TOGGLE", Fun_LedBlueToggle },
    { "CMD_LED_ALL_TOGGLE", Fun_LedAllToggle } 
};
#define CMD_CNT (sizeof(cmdTable) / sizeof(CmdTable_t))

/********************* 读指令 *******************/
static void Fun_ReadDHT11(char *resStr)
{
    uint8_t temp = DHT11_GetTemperature();
    uint8_t humi = DHT11_GetHumidity();

    // 返回json格式
    sprintf(resStr, "{\"dht11_temp\": %d, \"dht11_humi\": %d}", temp, humi);
}

static void Fun_ReadDS18B20(char *resStr)
{
    int8_t tempInt;
    uint8_t tempDec;
    DS18B20_GetTemp(&tempInt, &tempDec);

    // 返回json格式
    sprintf(resStr, "{\"ds18b20_temp\": %d.%d}", tempInt, tempDec);
}

ReadTable_t readTable[] = { 
    { "READ_DHT11", Fun_ReadDHT11 }, 
    { "READ_DS18B20", Fun_ReadDS18B20 } 
};
#define READ_CNT (sizeof(readTable) / sizeof(ReadTable_t))

/********************* 写指令 *******************/
static bool Fun_WiFiConfig(const char *str)
{
    char ssid[33] = { 0 };
    char pwd[33] = { 0 };

    // 按照格式提取字符串
    if (sscanf(str, "WRITE_WiFi_CONFIG:\"%32[^\"]\",\"%32[^\"]\"", ssid, pwd) != 2) {
        // LED_RED_TOGGLE();
        return false;
    }

    AT24C64_App_WriteWiFiSSID(ssid, strlen(ssid) + 1);
    // 等待写入间隔
    HAL_Delay(5);
    AT24C64_App_WriteWiFiPassword(pwd, strlen(pwd) + 1);

    // LED_BLUE_TOGGLE();
    return true;
}

WriteTable_t writeTable[] = { 
    { "WRITE_WiFi_CONFIG:", Fun_WiFiConfig } 
};
#define WRITE_CNT (sizeof(writeTable) / sizeof(WriteTable_t))

/*-----------------------------------------------------------------*/

// 查询控制指令
bool ESP8266_APP_Cmd(const char *cmdStr, uint16_t size)
{
    UNUSED(size);
    // Modbus_Transmit((uint8_t*)string, size);

    for (uint8_t i = 0; i < CMD_CNT; i++) {
        if (strcmp(cmdStr, cmdTable[i].cmdStr) == 0) {
            cmdTable[i].pFunc();
            return true;
        }
    }

    return false;
}

// 查询读取指令，赋值resStr字符串
bool ESP8266_APP_Read(const char *readStr, uint16_t size, char *resStr, uint8_t resSize)
{
    UNUSED(size);
    // Modbus_Transmit((uint8_t*)string, size);

    if (resSize < 100)
        return false;

    for (uint8_t i = 0; i < READ_CNT; i++) {
        if (strcmp(readStr, readTable[i].readStr) == 0) {
            readTable[i].pFunc(resStr);
            return true;
        }
    }
    return false;
}

ESP8266_APP_Write_e ESP8266_APP_Write(const char *writeStr, uint16_t size)
{
    for (uint8_t i = 0; i < WRITE_CNT; i++) {
        if (strstr(writeStr, writeTable[i].writeStr)) {
            if (writeTable[i].pFunc(writeStr))
                return WiFi_CONFIG_OK;
        }
    }

    return WRITE_ERROR;
}
