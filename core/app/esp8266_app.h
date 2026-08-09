#ifndef __ESP8266_APP_H__
#define __ESP8266_APP_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    WRITE_OK = 0,
    WRITE_ERROR,
    WiFi_CONFIG_OK,
    // CMD_ERROR,
    // READ_OK,
    // READ_ERROR,
} ESP8266_APP_WriteStatus;

bool ESP8266_APP_Cmd(const char *cmdStr, uint16_t size);
bool ESP8266_APP_Read(const char *readStr, uint16_t size, char *resStr, uint8_t resSize);
ESP8266_APP_WriteStatus ESP8266_APP_Write(const char *writeStr, uint16_t size);

#endif
