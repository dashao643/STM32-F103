#include "stm32f1xx_hal.h"
#include "esp8266.h"
#include "uart2.h"
#include "esp8266_app.h"
#include "at24c64_app.h"
#include "at24c64.h"
#include "rtc.h"
#include "general.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LED_DEBUG
#define UART1_DEBUG

#ifdef UART1_DEBUG
#include "uart1.h"
#endif

#ifdef LED_DEBUG
#include "led.h"
#endif

// 与 esp8266 通信用 uart2, uart1 用于打印调试

static bool isConfig_ = false;       // AP或STA模式是否成功配置
static bool doClockSyn_ = false;     // 是否需要时钟校准
static uint32_t clockTimer_ = 0;     // 等待时钟访问

static uint8_t *rxBuf_;              // 全局数组缓冲区

/************************ 云端服务器 *************************/
const char SERVER_IP[] = {"\"192.168.31.155\""};
// const char SERVER_IP[] = { "\"39.96.50.211\"" };
// const char SERVER_PORT[] = { "60001" };
const char SERVER_PORT[] = {"6789"};

/********************** 接收数据固定帧头 **********************/
// 0D   0A  2B 49 50 44 2C  30  2C   33     3A
// 回车 换行 +  I  P  D  ,   id  ,  字节数    :

const uint8_t FRAME_HEAD[] = { 0x0D, 0x0A, 0x2B, 0x49, 0x50, 0x44, 0x2C };
#define FRAME_HEAD_LEN (sizeof(FRAME_HEAD)) // 长度 = 7

static HAL_StatusTypeDef transmitReceive(const char *tx, const char *rx, uint16_t timeout);
static bool connectWiFi(void);
static void clockSync(void);
static bool clockFrameParse(void);
static void frameReply(uint8_t id, const char *data);
static bool frameHeaderCheck(void);
static void frameExecute(void);
static void frameProcess(void);
static bool AT_STA_Config(void);
static bool AT_AP_Config(void);
static bool connectToServer(void);

static HAL_StatusTypeDef transmitReceive(const char *tx, const char *rx, uint16_t timeout)
{
    uint8_t retryCnt = 0;

    while (retryCnt++ < ESP8266_RETRY_COUNT) {
        ESP8266_AT_Transmit(tx);
        uint8_t state = ESP8266_AT_Receive(rx, timeout);

        if(state == HAL_OK)
            return HAL_OK;
        else if(state == HAL_BUSY) {
            if (ESP8266_AT_Receive(rx, 3000) == HAL_OK)
                return HAL_OK;
        }
        else if(state == HAL_ERROR) 
            return HAL_ERROR;
    }
    return HAL_TIMEOUT;
}

static bool connectWiFi(void)
{
    // 从EE读出用户名和密码
    uint8_t ssidByte = 0, pwdByte = 0;
    AT24C64_ReadPage(WIFI_SSID_PAGE, &ssidByte, 1);
    AT24C64_ReadPage(WIFI_PASSWORD_PAGE, &pwdByte, 1);
    if (ssidByte == AT24C64_BLANK_BYTE || pwdByte == AT24C64_BLANK_BYTE) {
        printf("WiFi not configured\n");
        return false;
    }

    char ssidRow[33] = { 0 };
    char pwdRow[33] = { 0 };
    AT24C64_App_ReadWiFiSSID(ssidRow, 33);
    AT24C64_App_ReadWiFiPassword(pwdRow, 33);
    // 补充引号
    char ssid[35] = { 0 };
    char pwd[35] = { 0 };
    sprintf(ssid, "\"%s\"", ssidRow);
    sprintf(pwd, "\"%s\"", pwdRow);
    char connectWiFi[100] = { 0 };
    sprintf(connectWiFi, "AT+CWJAP=%s,%s\r\n", ssid, pwd);

    if (transmitReceive(connectWiFi, "WIFI CONNECTED", 5000) != HAL_OK) {
        printf("WiFi connected fail\n");
        return false;
    }
    return true;
}

static void clockSync(void)
{
    transmitReceive("AT+CIPSNTPCFG=1,8\r\n", "OK", 500);

    doClockSyn_ = true;
    clockTimer_ = HAL_GetTick();
}

/*
时间格式：4D 6F 6E 20 4A 75 6E 20 30 31 20 31 35 3A 30 33 3A 34 32 20 32 30 32 36 0D 0A 4F 4B 0D 0A
            周    空     月    空  日   空  时    ：  分   ：  秒   空     年       /r /n   ok   /r/n
          "AT+CIPSNTPTIME? +CIPSNTPTIME:Thu Jan 01 00:00:00 1970 OK"
*/
static bool clockFrameParse(void)
{
    transmitReceive("AT+CIPSNTPTIME?\r\n", "OK", 500);

    char monthStr[4] = { 0 };
    int day, hour, minute, second, year;

    uint8_t ret = sscanf((char *)rxBuf_, "%*[^:]:%*s %3s %d %d:%d:%d %d", 
        monthStr, &day, &hour, &minute, &second, &year);

    if (ret != 6)
        return false;

    int8_t month = monthMatch3c(monthStr, 3);
    if (month <= 0)
        return false;

    // printf("month=%d, day=%d, hour=%d, minute=%d, second=%d, year=%d\n", month, day, hour, minute, second, year);

    RTC_TimeTypeDef time = { 0 };
    RTC_DateTypeDef date = { 0 };
    date.Year = year % 2000;
    date.Month = month;
    date.Date = day;
    time.Hours = hour;
    time.Minutes = minute;
    time.Seconds = second;

    RTC_SetDateTime(&date, &time);

    UART2_GetRxClear();

    return true;
}

static void frameReply(uint8_t id, const char *data)
{
    uint8_t len = strlen(data);
    if (len > 100)
        return;

    char txBuf[50] = { 0 };
    sprintf(txBuf, "AT+CIPSEND=%u,%u\r\n", id, len);

    ESP8266_AT_Transmit(txBuf);
    HAL_Delay(5);

    ESP8266_AT_Transmit(data);
}

static bool frameHeaderCheck(void)
{
    // 最短帧长
    if (UART2_GetRxStruct()->size < ESP8266_RX_MINLENTH)
        return false;

    // 补充结尾符
    if(UART2_GetRxStruct()->size < UART2_RX_BUFF_MAX_SIZE)
        rxBuf_[UART2_GetRxStruct()->size] = 0;

    // 校验帧头
    if (memcmp(FRAME_HEAD, rxBuf_, FRAME_HEAD_LEN) != 0)
        return false;

    return true;
}

static void frameExecute(void)
{
    // 提取客户端id用于回复
    uint8_t id = rxBuf_[7] - '0';

    // 找冒号,冒号后是数据
    char *colon = strchr((char *)rxBuf_, ':');
    if (colon == NULL) {
        frameReply(id, "frame error");
        return;
    };
    char *data = colon + 1;
    // 总长度减去固定帧头长度
    uint16_t size = UART2_GetRxStruct()->size - 11;

    // 校验指令头是否是 CMD
    if (strncmp(data, "CMD", 3) == 0) {
        if (ESP8266_APP_Cmd(data, size))
            frameReply(id, "CMD OK");
        else
            frameReply(id, "CMD ERROR");
    }
    // 校验指令头是否是 READ
    else if (strncmp(data, "READ", 4) == 0) {
        char resString[100] = { 0 };
        if (ESP8266_APP_Read(data, size, resString, sizeof(resString)))
            frameReply(id, resString);
        else
            frameReply(id, "READ ERROR");
    }
    // 校验指令头是否是 WRITE
    else if (strncmp(data, "WRITE", 5) == 0) {
        uint8_t state = ESP8266_APP_Write(data, size);
        if (state == WiFi_CONFIG_OK) {
            frameReply(id, "WiFi reconnecting...");
            transmitReceive("AT+RST", NULL, 1000);
            ESP8266_Init();
        } else if (state == WRITE_OK)
            frameReply(id, "WRITE OK");
        else
            frameReply(id, "WRITE ERROR");
    }
    // 错误指令
    else {
        frameReply(id, "ERROR");
    }
}

static void frameProcess(void)
{
    if (!frameHeaderCheck()) {
        return;
    }

    frameExecute();
}

static bool AT_STA_Config(void)
{
    // 测试AT
    if (transmitReceive("AT\r\n", "OK", 20) != HAL_OK)
        return false;

    // 设置为STA模式
    if (transmitReceive("AT+CWMODE=1\r\n", "OK", 100) != HAL_OK)
        return false;

    // 开启多连接
    if (transmitReceive("AT+CIPMUX=1\r\n", NULL, 100) != HAL_OK)
        return false;
    // HAL_Delay(500);

    // 查询网络连接状态,
    if (transmitReceive("AT+CIPSTATUS\r\n", "OK", 100) != HAL_OK)
        return false;

    // 若为未连接，连接WiFi
    if (!strstr((char *)rxBuf_, "STATUS:2")) {
        if (!connectWiFi())
            return false;
    }

    // 查询本机IP地址并打印
    transmitReceive("AT+CIPSTA?\r\n", "ip", 5000);
    printf("%s\n", rxBuf_);

    // 联网时钟校准
    clockSync();

    // 开启本地TCP服务器(未实现)
    // if(!transmitReceive("AT+CIPSERVER=1,80\r\n", "OK", 10)) return false;
    // HAL_Delay(500);

    return true;
}

static bool AT_AP_Config(void)
{
    // 设置为AP模式
    if (transmitReceive("AT+CWMODE=2\r\n", "OK", 50) != HAL_OK)
        return false;
    // 配置WiFi
    if (transmitReceive("AT+CWSAP=\"dashao\",\"12345678\",6,4\r\n", "OK", 50) != HAL_OK)
        return false;
    // 设置多连接
    if (transmitReceive("AT+CIPMUX=1\r\n", "OK", 50) != HAL_OK)
        return false;
    // 开启服务 IP：192.168.4.1 Port:80
    if (transmitReceive("AT+CIPSERVER=1,80\r\n", "OK", 50) != HAL_OK)
        return false;

    return true;
}

static bool connectToServer(void)
{
    // 查询是否连接WiFi
    transmitReceive("AT+CIPSTATUS\r\n", NULL, 1000);
    if (strstr((char *)rxBuf_, "STATUS:5"))
        return false;

    // 与云端服务器建立TCP连接
    char tcpConnect[50] = { 0 };
    sprintf(tcpConnect, "AT+CIPSTART=0,\"TCP\",%s,%s\r\n", SERVER_IP, SERVER_PORT);
    transmitReceive(tcpConnect, "OK", 5000);

    // 查询是否连接云端
    if (transmitReceive("AT+CIPSTATUS\r\n", "STATUS:3", 1000) != HAL_OK) {
        printf("server connect fail\n");
        return false;
    }
    printf("server connect success\n");

    return true;
}

/*-----------------------------------------------------------------*/

/**
 * @brief 发送AT指令，不限长度
 * 
 * @param cmd AT指令
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef ESP8266_AT_Transmit(const char *cmd)
{
    // printf("%s\n",cmd);

    uint16_t len = strlen(cmd);

    // 发新指令前重置缓冲区
    UART2_GetRxClear();

    return UART2_Transmit((uint8_t *)cmd, len);
}

/**
 * @brief 在超时时间内接收指定AT回复指令
 * 
 * @param res 指定回复指令，NULL代表数据无限制
 * @param timeout 超时时间（单位ms）
 * @return HAL_StatusTypeDef 返回状态
 */
HAL_StatusTypeDef ESP8266_AT_Receive(const char *res, uint16_t timeout)
{
    uint16_t cnt = 0;

    while (cnt < timeout) {
        if (UART2_GetRxStruct()->rxFlag) {
            UART2_GetRxStruct()->rxFlag = 0;

            // 缓冲区索引处补充\0结束符
            if(UART2_GetRxStruct()->size < UART2_RX_BUFF_MAX_SIZE)
                rxBuf_[UART2_GetRxStruct()->size] = 0;
#ifdef UART1_DEBUG
            UART1_Transmit(rxBuf_, UART2_GetRxStruct()->size);
#endif
            if (!res)
                return HAL_OK;

            if (strstr((char *)rxBuf_, "busy"))
                return HAL_BUSY;

            if (strstr((char *)rxBuf_, "ERROR"))
                return HAL_ERROR;

            if (strstr((char *)rxBuf_, res))
                return HAL_OK;
        }
        HAL_Delay(1);
        cnt++;
    }
    return HAL_TIMEOUT;
}

bool ESP8266_ConnectToServer(void)
{
    bool res = connectToServer();
    UART2_GetRxClear();

    return res;
}

void ESP8266_Init(void)
{
    rxBuf_ = UART2_GetRxStruct()->rxBuf;

    // 默认STA模式
    if (AT_STA_Config()) {
        printf("STA model\n");
        isConfig_ = true;
    }
    // 失败则配置AP模式用于局域网通信和配网
    else if (AT_AP_Config()) {
        printf("AP model\n");
        isConfig_ = true;
    } 
    else
        printf("STA/AP Config fail\n");

    UART2_GetRxClear();
}

void ESP8266_Task(void)
{
    if (!isConfig_) return;

    if (doClockSyn_) {
        if ((HAL_GetTick() - clockTimer_) > ESP8266_CLOCK_SYN_MS) {
            clockFrameParse();
            doClockSyn_ = false;
        }
    }

    if (UART2_GetRxStruct()->rxFlag) {
#ifdef UART1_DEBUG
        UART1_Transmit(rxBuf_, UART2_GetRxStruct()->size);
#endif
        frameProcess();
        UART2_GetRxClear();
    }
}
