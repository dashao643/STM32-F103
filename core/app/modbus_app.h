#ifndef __MODBUS_APP_H__
#define __MODBUS_APP_H__

#include <stdint.h>
#include <stdbool.h>

/********************* 线圈地址定义 *********************/
#define LED_RED		        0x0001
#define LED_GREEN 	        0x0002
#define LED_BLUE 	        0x0003
#define PWM_LED	            0x0004
#define ESP8266_A           0x0005

/********************* 输入寄存器地址定义 *********************/
#define DHT11_TEMP	        0x0001
#define DHT11_HUMI	        0x0002

/********************* 保持寄存器地址定义 *********************/
#define RTC_DATE_TIME       0xFFFF      // 对应寄存器数量固定为6

// 支持的操作数
#define MODBUS_RESET 	    0x00    // 关闭
#define MODBUS_SET 	        0x01    // 开启
#define MODBUS_TOGGLE 	    0x02    // 翻转
#define CONNECT_SERVER      0x03
#define DISCONNECT_SERVER   0x04

// #define REG_ADDR_MAX	      0x0006   // 最大寄存器地址
// #define REG_CNT_MAX        6       // 寄存器地址数量

// IAP升级
#define IAP_MAGIC_ADDR        0x20004FFC  // 标志存储地址，RAM最后四个字节
#define IAP_MAGIC_VAL         0xA5A5A5A5
#define IAP_SPI_INFO_ADDR     0x20004FF8

// W25Q64写操作
#define W25Q64_PAGE_MAX       0x7FFF      // 页索引: 0 - 32768
#define W25Q64_SECTOR_MAX     0x07FF      // 扇区索引: 0 - 2047
#define W25Q64_ACK            0x81
#define W25Q64_WAIT_TIMEOUT   200

bool Modbus_App_RegAddrCheck(uint8_t func, uint16_t addr);
bool Modbus_App_RegCntCheck(uint8_t func, uint16_t cnt);
bool Modbus_App_OpDataCheck(uint8_t func, uint16_t regCnt, uint8_t data);

uint16_t Modbus_App_ReadInputReg(uint16_t addr);
bool Modbus_App_WriteCoil(uint16_t addr, uint8_t value);
bool Modbus_App_WriteRegs(uint16_t addr, const uint8_t *value);
bool Modbus_App_W25Q64(uint16_t basePage, uint16_t pageCnt);
void Modbus_App_IAP_UART(void);
void Modbus_App_IAP_SPI(uint16_t basePage, uint16_t pageCnt);

#endif
