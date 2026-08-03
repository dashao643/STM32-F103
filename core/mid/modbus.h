#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdint.h>
#include <stdbool.h>

/********************* ↓选择mobus硬件层协议↓ *******************/
#define MODBUS_UART
// #define MODBUS_RS485
/********************* ↑选择mobus硬件层协议↑ *******************/

#ifdef MODBUS_UART
#include "my_uart.h"
#endif
#ifdef MODBUS_RS485
#include "rs485.h"
#endif

#define MODBUS_INSTANCE                 USART1
#define MODBUS_HANDLE                   &huart1
// #define MODBUS_INSTANCE                 USART3
// #define MODBUS_HANDLE                   &huart3

#define MODBUS_UARTX_TIMEOUT            200
#define MODBUS_RX_BUFF_MAXLENTH         256     // 最大帧长度
#define MODBUS_RX_BUFF_MINLENTH         8       // 最小帧长度
#define MODBUS_SINGLE_WRITE_LENTH       9       // 单写操作帧长
#define MODBUS_TX_BUFF_MAXLENTH         50      // 回复帧最大帧长

#define MODBUS_SLAVE_ADDR               0x01    // 从机地址

#define MODBUS_FUNC_READ_COILS          0x01    // 读线圈（输出IO）
#define MODBUS_FUNC_READ_DISCRETE_INPUT 0x02    // 读离散输入（输入IO）
#define MODBUS_FUNC_READ_HOLD_REGS      0x03    // 读保持寄存器（可读可写变量）
#define MODBUS_FUNC_READ_INPUT_REGS     0x04    // 读输入寄存器（只读变量）       // 支持
#define MODBUS_FUNC_WRITE_SINGLE_COIL   0x05    // 写单个线圈                    // 支持
#define MODBUS_FUNC_WRITE_SINGLE_REG    0x06    // 写单个保持寄存器              
#define MODBUS_FUNC_WRITE_MULTI_COILS   0x0F    // 写多个线圈
#define MODBUS_FUNC_WRITE_MULTI_REGS    0x10    // 写多个保持寄存器               // 支持

/********************************** 自定义功能码 **********************************/
#define MODBUS_FUNC_W25Q64_WRITE        0x41    // 写w25q64FLASH                 // 支持
#define MODBUS_FUNC_IAP_HANDSHAKE       0x42    // 进入升级模式                   // 支持
#define MODBUS_FUNC_W25Q64_IAP          0x43    // 写w25q64FLASH并跳转bootloader  // 支持

/********************************** 错误码 **********************************/
#define MODBUS_FUNC_ERROR               0x01    // 非法功能码
#define MODBUS_REGS_ARR_ERROR           0x02    // 非法寄存器地址
#define MODBUS_REGS_CNT_ERROR           0x03    // 非法寄存器数量
#define MODBUS_OP_DATA_ERROR            0x04    // 非法操作数

typedef enum {
  MODBUS_STATE_IDLE,            // 空闲，等待一帧开始
  MODBUS_STATE_ADDR,            // 接收并校验从机地址
  MODBUS_STATE_CRC,             // 校验CRC
  MODBUS_STATE_FUNC,            // 接收功能码
  MODBUS_STATE_REG_ADDR,        // 寄存器地址
  MODBUS_STATE_REG_CNT,         // 寄存器数量
  MODBUS_STATE_DATA,            // 接收数据段（写操作用到）（一个字节）
  MODBUS_STATE_EXECUTE,         // 执行功能（读取数据/控制IO）
  MODBUS_STATE_REPLY,           // 构造回复帧
  MODBUS_STATE_RESET            // 重置状态机，准备下一帧
} Modbus_State_e;

typedef struct {
  uint8_t func;
  uint16_t regArr;
  uint16_t regCnt;
  bool isRead;
  uint8_t data;
  uint16_t txIndex;
} Modbus_Record_t;

#ifdef MODBUS_UART
typedef struct {
  My_UART_t uart;
  Modbus_State_e state;
  Modbus_Record_t record;
} Modbus_t;
#endif

#ifdef MODBUS_RS485
typedef struct {
  RS485_t rs485;
  Modbus_State_e state;
  Modbus_Record_t record;
} Modbus_t;
#endif

void Modbus_Init(void);
void Modbus_Task(void);

My_UART_t* Modbus_Get_UART(void);
bool Modbus_GetFrameFlag(void);

void Modbus_Transmit(const uint8_t *data, uint8_t size);

#endif
