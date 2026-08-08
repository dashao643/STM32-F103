#ifndef __ESP8266_APP_H__
#define __ESP8266_APP_H__

#include <stdint.h>
#include <stdbool.h>

// 三种指令：控制指令、写指令、读指令

// 命令指令，字符串对应功能函数
typedef struct {
    const char *cmdStr;
    void (*pFunc)(void);
} CmdTable_t;

// 读数据指令
typedef struct {
    const char *readStr;
    void (*pFunc)(char *resStr);
} ReadTable_t;

// 写数据指令
typedef struct {
    const char *writeStr;
    bool (*pFunc)(const char *);
} WriteTable_t;

typedef enum {
    WRITE_OK = 0,
    WRITE_ERROR,
    WiFi_CONFIG_OK,
    // CMD_ERROR,
    // READ_OK,
    // READ_ERROR,
} ESP8266_APP_Write_e;

bool ESP8266_APP_Cmd(const char *cmdStr, uint16_t size);
bool ESP8266_APP_Read(const char *readStr, uint16_t size, char *resStr, uint8_t resSize);
ESP8266_APP_Write_e ESP8266_APP_Write(const char *writeStr, uint16_t size);

#endif
