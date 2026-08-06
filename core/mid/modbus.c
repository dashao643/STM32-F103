#include "stm32f1xx_hal.h"
#include "modbus.h"
#include "general.h"
#include "modbus_app.h"

#include <stdio.h>
#include <string.h>
#include <led.h>

#ifdef MODBUS_UART
#include "uart1.h"
#endif
#ifdef MODBUS_RS485
#include "rs485.h"
#endif

// #ifdef MODBUS_RS485
// #define uart rs485.uart // 字段别名映射
// #endif

static uint8_t *rxBuf_;
static uint16_t size_;
// static uint16_t index_;

static Modbus_RecordTypeDef record_ = {0};

static void errorReply(const uint8_t errorCode)
{
    // 发送长度固定为 5
    uint8_t reply[5] = {0};

    U16Union crcCal = {0};

    reply[0] = MODBUS_SLAVE_ADDR;
    reply[1] = rxBuf_[1] | 0x80;
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

static bool funcCheck(uint8_t funcCode)
{
    if(funcCode >= 0x01 && funcCode <= 0x04) {
        if(size_ != MODBUS_RX_BUFF_MINLENTH)
            return false;
    }
    else if(funcCode >= 0x05 && funcCode <= 0x06) {
        if(size_ != MODBUS_SINGLE_WRITE_LENTH)
            return false;
    }
    else if(funcCode >= 0x41 && funcCode <= 0x43) {
        if(size_ != MODBUS_RX_BUFF_MINLENTH)
            return false;
    } 
    else if(funcCode != 0x0F && funcCode != 0x10)
        return false;

    return true;
}

static bool frameCheck(void)
{
    /********************* no reply *********************/
    // length check
    if(size_ < MODBUS_RX_BUFF_MINLENTH) return false;

    // address check
    if(rxBuf_[0] != MODBUS_SLAVE_ADDR) return false;
    
    // crc check
    U16Union crc = {0};

    crc.word = CRC16_Modbus(rxBuf_, size_ - 2);

    if(crc.bytes[0] != rxBuf_[size_ - 2] || crc.bytes[1] != rxBuf_[size_ - 1])
        return false;

    /********************* error reply *********************/
    // function code check
    record_.func = rxBuf_[1];

    if(!funcCheck(record_.func)) {
        errorReply(MODBUS_FUNC_ERROR);
        return false;
    }

    if(rxBuf_[1] >= 0x01 && rxBuf_[1] <= 0x04)
        record_.isRead = true;
    else
        record_.isRead = false;

    // register address check
    record_.regAddr = (rxBuf_[2] << 8) | rxBuf_[3];
    if(!Modbus_App_RegAddrCheck(record_.func, record_.regAddr)) {
        errorReply(MODBUS_REGS_ADDR_ERROR);
        return false;
    }

    // register count check
    record_.regCnt = (rxBuf_[4] << 8) | rxBuf_[5];
    if(!Modbus_App_RegCntCheck(record_.func, record_.regCnt)) {
        errorReply(MODBUS_REGS_CNT_ERROR);
        return false;
    }

    // write cmd, operate data check
    if(!record_.isRead) {
        record_.opData = rxBuf_[6];
        if(!Modbus_App_OpDataCheck(record_.func, record_.regCnt, record_.opData)) {
            errorReply(MODBUS_OP_DATA_ERROR);
            return false;
        }
    }

    return true;
}

static bool frameExecute(void)
{
    if(record_.isRead) return true;

    if(record_.func == MODBUS_FUNC_WRITE_SINGLE_COIL) {
        return Modbus_App_WriteCoil(record_.regAddr, record_.opData);
    } 
    else if(record_.func == MODBUS_FUNC_WRITE_SINGLE_REG) {
        
    } 
    else if(record_.func == MODBUS_FUNC_WRITE_MULTI_COILS) {

    } 
    else if(record_.func == MODBUS_FUNC_WRITE_MULTI_REGS) {
        // 寄存器地址, 数据长度
        return Modbus_App_WriteRegs(record_.regAddr, rxBuf_ + 6);
    } 
    else if(record_.func == MODBUS_FUNC_W25Q64_WRITE) {
        return Modbus_App_W25Q64(record_.regAddr, record_.regCnt);
    } 
    else if(record_.func == MODBUS_FUNC_IAP_HANDSHAKE) {
        Modbus_App_IAP_UART();
    } 
    else if(record_.func == MODBUS_FUNC_W25Q64_IAP) {
        if(!Modbus_App_W25Q64(record_.regAddr, record_.regCnt))
            return false;

        Modbus_App_IAP_SPI(record_.regAddr, record_.regCnt);
    }

    return false;
}

static void frameReply(void)
{
    uint8_t replyArr[20] = {0};
    uint8_t txIdx = 0;

    replyArr[txIdx++] = MODBUS_SLAVE_ADDR;
    replyArr[txIdx++] = record_.func;

    if(record_.isRead) {
        uint8_t byteCnt = record_.regCnt * 2;
        replyArr[txIdx++] = byteCnt;

        if(record_.func == MODBUS_FUNC_READ_COILS) {
    
        } 
        else if(record_.func == MODBUS_FUNC_READ_DISCRETE_INPUT) {
            
        } 
        else if(record_.func == MODBUS_FUNC_READ_HOLD_REGS) {
    
        } 
        else if(record_.func == MODBUS_FUNC_READ_INPUT_REGS) {
            for(uint8_t i = 0; i < record_.regCnt; i++) {
                uint16_t res = Modbus_App_ReadInputReg(record_.regAddr + i);
    
                replyArr[txIdx++] = res >> 8;
                replyArr[txIdx++] = (uint8_t)res;
            }
        }
    } else {
        replyArr[txIdx++] = record_.regAddr >> 8;
        replyArr[txIdx++] = (uint8_t)record_.regAddr;

        replyArr[txIdx++] = record_.regCnt >> 8;
        replyArr[txIdx++] = (uint8_t)record_.regCnt;
    }

    U16Union CRC16;
    // 计算CRC，低字节在前，高字节在后
    CRC16.word = CRC16_Modbus(replyArr, txIdx);
    replyArr[txIdx++] = CRC16.bytes[0];
    replyArr[txIdx++] = CRC16.bytes[1];

    UART1_Transmit(replyArr, txIdx);
}

// 主循环调用
void Modbus_Task(void)
{
    if (*UART1_GetRxFlag() == false) return;

    *UART1_GetRxFlag() = false;

    rxBuf_ = UART1_GetRxBuf(&size_);

    if (!frameCheck()) return;
        
    if (!frameExecute()) {
        errorReply(MODBUS_EXECUTE_ERROR);
        return;
    }

    frameReply();

// #ifdef MODBUS_RS485
//     RS485_Task(&modbus.rs485);
// #endif
}
