#include "stm32f1xx_hal.h"
#include "modbus.h"
#include "general.h"
// #include "modbus_app.h"

#include <stdio.h>
#include <string.h>

#ifdef MODBUS_UART
#include "uart1.h"
#endif
#ifdef MODBUS_RS485
#include "rs485.h"
#endif

// #ifdef MODBUS_RS485
// #define uart rs485.uart // 字段别名映射
// #endif

static uint8_t *rxBuf;
static uint16_t size;

static Modbus_RecordTypeDef record = {0};

// static uint8_t txBuf[MODBUS_TX_BUFF_MAXLENTH];

// static Modbus_t modbus = { 0 };

// typedef enum {
//     STATE_IDLE,     // 空闲
//     STATE_ADDR,     // 校验从机地址
//     STATE_CRC,      // 校验CRC
//     STATE_FUNC,     // 功能码
//     STATE_REG_ADDR, // 寄存器地址
//     STATE_REG_CNT,  // 寄存器数量
//     STATE_DATA,     // 操作数
//     STATE_EXECUTE,  // 执行
//     STATE_REPLY,    // 回复帧
//     STATE_RESET     // 重置
// } StateTypeDef;

// static StateTypeDef state = STATE_IDLE;

// static void clearRecord(void);
// static bool lengthCheck(void);
// static bool addressCheck(void);
// static bool funcCheck(void);
// static bool regArrCheck(void);
// static bool regCntCheck(void);
// static bool opDataCheck(void);
// static bool crcCheck(void);
// static void errorReply(const uint8_t errorCode);
// static void frameProcess(void);
// static void frameExecute(void);
// static void frameReply(void);

static void errorReply(const uint8_t errorCode)
{
    // 发送长度固定为 5
    uint8_t reply[5] = {0};

    U16Union crcCal = {0};

    reply[0] = MODBUS_SLAVE_ADDR;
    reply[1] = rxBuf[1] | 0x80;
    reply[2] = errorCode;

    crcCal.word = CRC16_Modbus(reply, 3);

    reply[3] = crcCal.bytes[0];
    reply[4] = crcCal.bytes[1];

#if defined MODBUS_UART
    UART1_Transmit(reply, 5);
#elif defined MODBUS_RS485
    RS485_Transmit(&modbus.rs485, 5, DMA, MODBUS_UARTX_TIMEOUT);
#endif
}

static bool funcCheck(void)
{
    switch (rxBuf[1]) {
        // 读操作，数据长度为最小
        case MODBUS_FUNC_READ_COILS:
        case MODBUS_FUNC_READ_DISCRETE_INPUT:
        case MODBUS_FUNC_READ_HOLD_REGS:
        case MODBUS_FUNC_READ_INPUT_REGS:{
            if(size != MODBUS_RX_BUFF_MINLENTH)
                return false;

            record.isRead = true;
            break;
        }
        // 单写操作
        case MODBUS_FUNC_WRITE_SINGLE_COIL:
        case MODBUS_FUNC_WRITE_SINGLE_REG: {
            if(size != MODBUS_SINGLE_WRITE_LENTH)
                return false;
        }
        case MODBUS_FUNC_WRITE_MULTI_COILS:
        case MODBUS_FUNC_WRITE_MULTI_REGS:
        case MODBUS_FUNC_W25Q64_WRITE:
        case MODBUS_FUNC_W25Q64_IAP:
        case MODBUS_FUNC_IAP_HANDSHAKE:{
            record.isRead = false;
            break;
        }
        default: return false;
    }

    record.func = rxBuf[1];

    return true;
}

static bool frameCheck(void)
{
    /********************* no reply *********************/
    // length check
    if(size < MODBUS_RX_BUFF_MINLENTH) return false;

    // address check
    if(rxBuf[0] != MODBUS_SLAVE_ADDR) return false;
    
    // crc check
    U16Union crc = {0};

    crc.word = CRC16_Modbus(rxBuf, size - 2);

    if(crc.bytes[0] != rxBuf[size - 2] || crc.bytes[1] != rxBuf[size - 1])
        return false;

    /********************* error reply *********************/
    if(!funcCheck()) {
        errorReply(MODBUS_FUNC_ERROR);
        return false;
    }

    return true;
}

// static void clearRecord(void)
// {
//     modbus.record.func = 0;
//     modbus.record.regArr = 0;
//     modbus.record.regCnt = 0;
//     modbus.record.isRead = false;
//     modbus.record.data = 0;
//     modbus.record.txIndex = 0;
// }

// static bool lengthCheck(void)
// {
//     if (modbus.uart.rxSize < MODBUS_RX_BUFF_MINLENTH)
//         return false;
//     return true;
// }

// static bool addressCheck(void)
// {
//     if (modbus.uart.rxBuf[0] != MODBUS_SLAVE_ADDR)
//         return false;
//     return true;
// }

// /**
//  * @brief 功能码校验
//  * 
//  * @return true 校验成功
//  * @return false 校验失败
//  */
// static bool funcCheck(void)
// {
//     switch (modbus.uart.rxBuf[1]) {
//     // 读操作，数据长度为最小
//     case MODBUS_FUNC_READ_COILS:
//     case MODBUS_FUNC_READ_DISCRETE_INPUT:
//     case MODBUS_FUNC_READ_HOLD_REGS:
//     case MODBUS_FUNC_READ_INPUT_REGS: {
//         if (modbus.uart.rxSize != MODBUS_RX_BUFF_MINLENTH)
//             return false;
//         modbus.record.isRead = true;
//         break;
//     }
//     // 单写操作
//     case MODBUS_FUNC_WRITE_SINGLE_COIL:
//     case MODBUS_FUNC_WRITE_SINGLE_REG:
//         if (modbus.uart.rxSize != MODBUS_SINGLE_WRITE_LENTH)
//             return false;
//     case MODBUS_FUNC_WRITE_MULTI_COILS:
//     case MODBUS_FUNC_WRITE_MULTI_REGS:
//     case MODBUS_FUNC_W25Q64_WRITE:
//     case MODBUS_FUNC_W25Q64_IAP:
//     case MODBUS_FUNC_IAP_HANDSHAKE: {
//         modbus.record.isRead = false;
//         break;
//     }
//     default:
//         return false;
//     }
//     modbus.record.func = modbus.uart.rxBuf[1];
//     return true;
// }

// static bool regArrCheck(void)
// {
//     U16Union regArr = { 0 };
//     // 先赋值低字节，再赋值高字节
//     regArr.bytes[0] = modbus.uart.rxBuf[3];
//     regArr.bytes[1] = modbus.uart.rxBuf[2];

//     if (!Modbus_App_Check_Address(modbus.record.func, regArr.word))
//         return false;

//     modbus.record.regArr = regArr.word;
//     return true;
// }

// // 检验功能码对应的数据长度和寄存器数量
// static bool regCntCheck(void)
// {
//     U16Union regCnt = { 0 };
//     // 先赋值低字节，再赋值高字节
//     regCnt.bytes[0] = modbus.uart.rxBuf[5];
//     regCnt.bytes[1] = modbus.uart.rxBuf[4];

//     // 指令长度固定
//     if (modbus.record.func == MODBUS_FUNC_WRITE_MULTI_REGS) {
//         if (modbus.uart.rxSize != (MODBUS_SINGLE_WRITE_LENTH + modbus.uart.rxBuf[6]))
//             return false;
//     }
//     if (modbus.record.func == MODBUS_FUNC_IAP_HANDSHAKE || modbus.record.func == MODBUS_FUNC_W25Q64_WRITE) {
//         if (modbus.uart.rxSize != MODBUS_RX_BUFF_MINLENTH)
//             return false;
//     }
//     if (!Modbus_App_Check_RegCount(modbus.record.func, regCnt.word))
//         return false;

//     modbus.record.regCnt = regCnt.word;
//     return true;
// }

// static bool opDataCheck(void)
// {
//     if (!Modbus_App_Check_WriteValue(modbus.record.func, modbus.record.regCnt, modbus.uart.rxBuf[6]))
//         return false;
//     modbus.record.data = modbus.uart.rxBuf[6];
//     return true;
// }

// static bool crcCheck(void)
// {
//     U16Union crcRes = { 0 };
//     // 最后两字节是收到的CRC，不参与计算
//     crcRes.word = CRC16_Modbus(modbus.uart.rxBuf, modbus.uart.rxSize - 2);
//     // crc16低位 -- 数据传输先传低字节
//     if (crcRes.bytes[0] == modbus.uart.rxBuf[modbus.uart.rxSize - 2] && crcRes.bytes[1] == modbus.uart.rxBuf[modbus.uart.rxSize - 1])
//         return true;
//     return false;
// }

// static void errorReply(const uint8_t errorCode)
// {
//     U16Union crcCal = { 0 };
//     // 用全局 txBuf
//     modbus.uart.txBuf[0] = MODBUS_SLAVE_ADDR;
//     modbus.uart.txBuf[1] = modbus.uart.rxBuf[1] | 0x80;
//     modbus.uart.txBuf[2] = errorCode;

//     // CRC
//     crcCal.word = CRC16_Modbus(modbus.uart.txBuf, 3);
//     modbus.uart.txBuf[3] = crcCal.bytes[0];
//     modbus.uart.txBuf[4] = crcCal.bytes[1];

//     // 发送长度 5
// #ifdef MODBUS_UART
//     UART_Transmit(&modbus.uart, 5, DMA, MODBUS_UARTX_TIMEOUT);
// #endif
// #ifdef MODBUS_RS485
//     RS485_Transmit(&modbus.rs485, 5, DMA, MODBUS_UARTX_TIMEOUT);
// #endif
// }

// // 状态机实现帧处理
// static void frameProcess(void)
// {
//     switch (modbus.state) {
//     /********************* 校验 *********************/
//     /************** 不回复帧 **************/
//     case MODBUS_STATE_IDLE: {
//         if (lengthCheck())
//             modbus.state = MODBUS_STATE_ADDR;
//         else
//             modbus.state = MODBUS_STATE_RESET;
//         break;
//     }
//     case MODBUS_STATE_ADDR: {
//         if (addressCheck())
//             modbus.state = MODBUS_STATE_CRC;
//         else
//             modbus.state = MODBUS_STATE_RESET;
//         break;
//     }
//     case MODBUS_STATE_CRC: {
//         if (crcCheck())
//             modbus.state = MODBUS_STATE_FUNC;
//         else
//             modbus.state = MODBUS_STATE_RESET;
//         break;
//     }
//     /************** 回复帧 **************/
//     case MODBUS_STATE_FUNC: {
//         if (funcCheck())
//             modbus.state = MODBUS_STATE_REG_ADDR;
//         else {
//             errorReply(MODBUS_FUNC_ERROR);
//             modbus.state = MODBUS_STATE_RESET;
//         }
//         break;
//     }
//     case MODBUS_STATE_REG_ADDR: {
//         if (regArrCheck())
//             modbus.state = MODBUS_STATE_REG_CNT;
//         else {
//             errorReply(MODBUS_REGS_ARR_ERROR);
//             modbus.state = MODBUS_STATE_RESET;
//         }
//         break;
//     }
//     case MODBUS_STATE_REG_CNT: {
//         if (regCntCheck()) {
//             if (modbus.record.isRead)
//                 modbus.state = MODBUS_STATE_EXECUTE;
//             else
//                 modbus.state = MODBUS_STATE_DATA;
//         } else {
//             errorReply(MODBUS_REGS_CNT_ERROR);
//             modbus.state = MODBUS_STATE_RESET;
//         }
//         break;
//     }
//     case MODBUS_STATE_DATA: {
//         if (opDataCheck())
//             modbus.state = MODBUS_STATE_EXECUTE;
//         else {
//             errorReply(MODBUS_OP_DATA_ERROR);
//             modbus.state = MODBUS_STATE_RESET;
//         }
//         break;
//     }
//     /********************* 执行 *********************/
//     case MODBUS_STATE_EXECUTE: {
//         frameExecute();
//         modbus.state = MODBUS_STATE_REPLY;
//         break;
//     }
//     /********************* 回复 *********************/
//     case MODBUS_STATE_REPLY: {
//         frameReply();
//         modbus.state = MODBUS_STATE_RESET;
//         break;
//     }
//     /********************* 重置 *********************/
//     case MODBUS_STATE_RESET: {
//         UART_Clear(&modbus.uart);
//         clearRecord();
//         modbus.state = MODBUS_STATE_IDLE;
//         break;
//     }
//     default:
//         break;
//     }
// }

// /**
//  * @brief 根据记录的帧，实现帧处理，帧回复
//  * 
//  */
// static void frameExecute(void)
// {
//     modbus.record.txIndex = 0;
//     modbus.uart.txBuf[modbus.record.txIndex++] = MODBUS_SLAVE_ADDR;
//     modbus.uart.txBuf[modbus.record.txIndex++] = modbus.record.func;
//     // 读操作需要返回数据
//     if (modbus.record.isRead) {
//         // 纯数据长度，每个寄存器 2 字节
//         uint8_t byteCnt = modbus.record.regCnt * 2;
//         modbus.uart.txBuf[modbus.record.txIndex++] = byteCnt;
//     }
//     // 写操作直接拷贝原始帧
//     U16Union regData = { 0 };
//     switch (modbus.record.func) {
//     /*********************** 读 *************************/
//     // case MODBUS_FUNC_READ_COILS:{
//     //   break;
//     // }
//     // case MODBUS_FUNC_READ_DISCRETE_INPUT:{
//     //   break;
//     // }
//     // case MODBUS_FUNC_READ_HOLD_REGS:{
//     //   break;
//     // }
//     case MODBUS_FUNC_READ_INPUT_REGS: {
//         for (uint8_t i = 0; i < modbus.record.regCnt; i++) {
//             regData.word = Modbus_App_Read_InputReg(modbus.record.regArr + i);
//             // 数据先传高字节，再传低字节
//             modbus.uart.txBuf[modbus.record.txIndex++] = regData.bytes[1];
//             modbus.uart.txBuf[modbus.record.txIndex++] = regData.bytes[0];
//         }
//         break;
//     }
//     /*********************** 写 *************************/
//     case MODBUS_FUNC_WRITE_SINGLE_COIL: {
//         Modbus_App_Write_Coil(modbus.record.regArr, modbus.record.data);
//         break;
//     }
//     // case MODBUS_FUNC_WRITE_SINGLE_REG:{
//     //
//     //   break;
//     // }
//     // case MODBUS_FUNC_WRITE_MULTI_COILS:{
//     //   break;
//     // }
//     case MODBUS_FUNC_WRITE_MULTI_REGS: {
//         Modbus_App_Write_Reg(modbus.record.regArr, modbus.uart.rxBuf + 6);
//         break;
//     }
//     case MODBUS_FUNC_W25Q64_WRITE: {
//         if (!Modbus_App_W25Q64(modbus.record.regArr, modbus.record.regCnt))
//             printf("w25q64 exec error\n");
//         break;
//     }
//     case MODBUS_FUNC_IAP_HANDSHAKE: {
//         Modbus_App_IAP_UART();
//         break;
//     }
//     case MODBUS_FUNC_W25Q64_IAP: {
//         if (!Modbus_App_W25Q64(modbus.record.regArr, modbus.record.regCnt)) {
//             printf("w25q64 exec error\n");
//             return;
//         }
//         Modbus_App_IAP_SPI(modbus.record.regArr, modbus.record.regCnt);
//         break;
//     }
//     }
//     return;
// }

// static void frameReply(void)
// {
//     // 如果是写操作，再加上寄存器地址、寄存器数量(4个字节)
//     if (!modbus.record.isRead) {
//         modbus.uart.txBuf[modbus.record.txIndex++] = modbus.uart.rxBuf[2];
//         modbus.uart.txBuf[modbus.record.txIndex++] = modbus.uart.rxBuf[3];
//         modbus.uart.txBuf[modbus.record.txIndex++] = modbus.uart.rxBuf[4];
//         modbus.uart.txBuf[modbus.record.txIndex++] = modbus.uart.rxBuf[5];
//     }

//     U16Union CRC16 = { 0 };
//     // 计算CRC，低字节在前，高字节在后
//     CRC16.word = CRC16_Modbus(modbus.uart.txBuf, modbus.record.txIndex);
//     modbus.uart.txBuf[modbus.record.txIndex++] = CRC16.bytes[0];
//     modbus.uart.txBuf[modbus.record.txIndex++] = CRC16.bytes[1];

// #ifdef MODBUS_UART
//     UART_Transmit(&modbus.uart, modbus.record.txIndex, BLOCK, MODBUS_UARTX_TIMEOUT);
// #endif

// #ifdef MODBUS_RS485
//     RS485_Transmit(&modbus.rs485, modbus.record.txIndex, BLOCK, MODBUS_UARTX_TIMEOUT);
// #endif
// }

/**
 * @brief 初始化Modbus结构体，开启DMA和空闲中断
 * 
 */
// void Modbus_Init(void)
// {
//     /******************* UART *******************/
//     modbus.uart.instance = MODBUS_INSTANCE;
//     modbus.uart.handle = MODBUS_HANDLE;
//     modbus.uart.rxBuf = rxBuf;
//     modbus.uart.rxMaxSize = MODBUS_RX_BUFF_MAXLENTH;
//     modbus.uart.txBuf = txBuf;
//     modbus.uart.txMaxSize = MODBUS_TX_BUFF_MAXLENTH;

//     UART_Clear(&modbus.uart);

//     HAL_UART_Receive_DMA(MODBUS_HANDLE, modbus.uart.rxBuf, modbus.uart.rxMaxSize);
//     __HAL_UART_ENABLE_IT(MODBUS_HANDLE, UART_IT_IDLE);

//     /******************* RS485 *******************/
// #ifdef MODBUS_RS485
//     RS485_RECEIVE_MODE();
//     modbus.rs485.transmitFlag = false;
// #endif
//     /******************* Modbus *******************/
//     modbus.state = MODBUS_STATE_IDLE;
// }

// 主循环调用
void Modbus_Task(void)
{
    if (*UART1_GetRxFlag() == false) return;

    *UART1_GetRxFlag() = false;

    rxBuf = UART1_GetRxBuf(&size);
    // UART1_Transmit(rxBuf, size);

    if (!frameCheck()) return;
        
    // frameReply();

    // frameExecute();
    uint8_t data = 0xAA;
    UART1_Transmit(&data, 1);
    // printf("0xAA\n");
// #ifdef MODBUS_RS485
//     RS485_Task(&modbus.rs485);
// #endif
}

// My_UART_t *Modbus_Get_UART(void)
// {
//     return &modbus.uart;
// }

// bool Modbus_GetFrameFlag(void)
// {
//     return modbus.uart.frameEnd;
// }

// void Modbus_Transmit(const uint8_t *data, uint8_t size)
// {
//     if (size > MODBUS_TX_BUFF_MAXLENTH)
//         size = MODBUS_TX_BUFF_MAXLENTH;

//     memcpy(modbus.uart.txBuf, data, size);

// #if defined(MODBUS_UART)
//     UART_Transmit(&modbus.uart, size, BLOCK, MODBUS_UARTX_TIMEOUT);
// #elif defined(MODBUS_RS485)
//     RS485_Transmit(&modbus.rs485, size, BLOCK, MODBUS_UARTX_TIMEOUT);
// #endif
// }