#include "stm32f1xx_hal.h"
#include "modbus_app.h"
#include "modbus.h"
#include "uart1.h"
#include "general.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LED
// #define DHT11
#define MY_RTC
// #define ESP8266
// #define W25Q64

#ifdef LED
#include "led.h"
#endif

#ifdef DHT11
#include "dht11.h"
#endif

#ifdef MY_RTC
#include "rtc.h"
#endif

#ifdef ESP8266
#include "esp8266.h"
#endif

#ifdef W25Q64
#include "w25q64.h"
#endif

static bool waitFrameTimeout(void) UNUSED_FUNC;

// 检验寄存器地址
bool Modbus_App_RegAddrCheck(uint8_t func, uint16_t addr)
{
#ifdef W25Q64
    if (func == MODBUS_FUNC_W25Q64_WRITE && addr >= W25Q64_PAGE_CNT)
        return false;
#endif
    if (func == MODBUS_FUNC_IAP_HANDSHAKE && addr != 0x01)
        return false;

    return true;
}

// 校验寄存器数量
bool Modbus_App_RegCntCheck(uint8_t func, uint16_t cnt)
{
    if (cnt == 0)
        return false;
    if (func == MODBUS_FUNC_WRITE_SINGLE_COIL && cnt != 1)
        return false;
    if (func == MODBUS_FUNC_IAP_HANDSHAKE && cnt != 1)
        return false;
#ifdef W25Q64
    if (func == MODBUS_FUNC_W25Q64_WRITE && cnt >= W25Q64_PAGE_CNT)
        return false;
#endif
    return true;
}

// 如果是单写操作，检查操作数; 多写操作，检查寄存器数和字节数
bool Modbus_App_OpDataCheck(uint8_t func, uint16_t regCnt, uint8_t data)
{
    if (func == MODBUS_FUNC_WRITE_SINGLE_COIL) {
        if (data > 0x04)
            return false;
    }
    if (func == MODBUS_FUNC_WRITE_MULTI_REGS) {
        if (regCnt != data)
            return false;
    }

    return true;
}

uint16_t Modbus_App_ReadInputReg(uint16_t addr)
{
#ifdef DHT11
    if (addr == DHT11_TEMP)
        return DHT11_GetTemperature();
    if (addr == DHT11_HUMI)
        return DHT11_GetHumidity();
#endif
    return 0;
}

bool Modbus_App_WriteCoil(uint16_t addr, uint8_t value)
{
#ifdef LED
    if (addr == LED_RED) {
        if (value == MODBUS_RESET) LED_RED_OFF();
        else if (value == MODBUS_SET) LED_RED_ON();
        else LED_RED_TOGGLE();
        return true;
    }
    if(addr == LED_GREEN){
        if(value == MODBUS_RESET) LED_GREEN_OFF();
        else if(value == MODBUS_SET) LED_GREEN_ON();
        else LED_GREEN_TOGGLE();
        return true;
    }
    if(addr == LED_BLUE){
        if(value == MODBUS_RESET) LED_BLUE_OFF();
        else if(value == MODBUS_SET) LED_BLUE_ON();
        else LED_BLUE_TOGGLE();
        return true;
    }
#endif

#ifdef ESP8266
    if (addr == ESP8266_A) {
        if (value == CONNECT_SERVER) {
            ESP8266_ConnectToServer();
            return true;
        }
    }
#endif
    
    return false;
}

bool Modbus_App_WriteRegs(uint16_t addr, const uint8_t *value)
{
#ifdef MY_RTC
    // 设置日期和时间，地址从年开始
    if ((addr != RTC_DATE_TIME) || (value[0] != 6))
        return false;
    RTC_TimeTypeDef time = { 0 };
    RTC_DateTypeDef date = { 0 };
    date.Year = value[1];
    date.Month = value[2];
    date.Date = value[3];
    time.Hours = value[4];
    time.Minutes = value[5];
    time.Seconds = value[6];
    RTC_SetDateTime(&date, &time);
    return true;
#endif

    return false;
}

/**
 * @brief 阻塞等待空闲中断标志位
 * @return true 等待超时
 */
static bool waitFrameTimeout(void)
{
    uint8_t timer = 0;

    while (timer < W25Q64_WAIT_TIMEOUT) {
        if (*UART1_GetRxFlag()) {
            *UART1_GetRxFlag() = false;
            return false;
        }
        HAL_Delay(1);
        timer++;
    }
    return true;
}

// basePage: 0-65535
bool Modbus_App_W25Q64(uint16_t basePage, uint16_t pageCnt)
{
#ifdef W25Q64
    uint8_t ack = W25Q64_ACK;
    uint8_t curTimes = 0;

    int16_t sectorIdxLast = -1;
    while (curTimes < pageCnt) {
        Modbus_Transmit(&ack, 1);
        // 清除帧结束标志, 准备接收数据
        Modbus_Get_UART()->frameEnd = false;

        if (waitFrameTimeout())
            return false;
        // printf("size=%d\n", Modbus_Get_UART()->rxSize);
        if (Modbus_Get_UART()->rxSize != 256)
            return false;

        // 第一次跨页写的时候, 先执行页擦除
        uint16_t sectorIdx = basePage / 16;
        if (sectorIdxLast != sectorIdx) {
            W25Q64_EraseSector(sectorIdx);
            sectorIdxLast = sectorIdx;
            // printf("exec erase\n");
        }
        W25Q64_WritePage(basePage, Modbus_Get_UART()->rxBuf, 256);
        basePage++;
        curTimes++;
    }
    Modbus_Transmit(&ack, 1);

    // 通知状态机继续推进到 REPLY → RESET
    Modbus_Get_UART()->frameEnd = true;
    return true
#endif
    return false;
}

void Modbus_App_IAP_UART(void)
{
    *(uint32_t *)IAP_MAGIC_ADDR = IAP_MAGIC_VAL;

    HAL_NVIC_SystemReset();
}

void Modbus_App_IAP_SPI(uint16_t basePage, uint16_t pageCnt)
{
    *(uint32_t *)IAP_MAGIC_ADDR = IAP_MAGIC_VAL;

    uint32_t memoryVal = (pageCnt << 16) | basePage;
    *(uint32_t *)IAP_SPI_INFO_ADDR = memoryVal;

    HAL_NVIC_SystemReset();
}
