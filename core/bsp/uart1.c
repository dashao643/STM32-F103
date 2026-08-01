#include "stm32f1xx_hal.h"
#include "uart1.h"

#include <stdint.h>

#include <string.h>

#include "led.h"

// PA9  ------> USART1_TX
// PA10 ------> USART1_RX
static UART_HandleTypeDef uart1;
static DMA_HandleTypeDef dmaTx;
static DMA_HandleTypeDef dmaRx;

static uint8_t rxBuf[UART1_RX_BUFF_MAX_SIZE];
static uint8_t txBuf[UART1_TX_BUFF_MAX_SIZE];
static volatile bool rxFlag = false;
static volatile uint16_t rxSize;

/********************* private *********************/

static void idleProcess(void)
{
    if(__HAL_UART_GET_FLAG(&uart1, UART_FLAG_IDLE) == SET) {
        __HAL_UART_CLEAR_IDLEFLAG(&uart1);

        rxSize = UART1_RX_BUFF_MAX_SIZE - __HAL_DMA_GET_COUNTER(uart1.hdmarx);

        rxFlag = true;

        HAL_UART_DMAStop(&uart1);
        HAL_UART_Receive_DMA(&uart1, rxBuf, UART1_RX_BUFF_MAX_SIZE);
    }
}

/********************* override *********************/

void DMA1_Channel4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&dmaTx);
}

void DMA1_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&dmaRx);
}

void USART1_IRQHandler(void)
{
    idleProcess();

    HAL_UART_IRQHandler(&uart1);
}

/********************* public *********************/

HAL_StatusTypeDef UART1_Init(void)
{
    uart1.Instance = USART1;                                // 外设基地址指针
    uart1.Init.BaudRate = 115200;                           // 波特率
    uart1.Init.WordLength = UART_WORDLENGTH_8B;             // 8位字长
    uart1.Init.StopBits = UART_STOPBITS_1;                  // 1位停止位
    uart1.Init.Parity = UART_PARITY_NONE;                   // 无校验
    uart1.Init.Mode = UART_MODE_TX_RX;                      // 发送和接收模式
    uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;             // 无硬件控制流
    uart1.Init.OverSampling = UART_OVERSAMPLING_16;         // 采样频率

    uint8_t state = HAL_UART_Init(&uart1);

    HAL_UART_Receive_DMA(&uart1, rxBuf, UART1_RX_BUFF_MAX_SIZE);
    __HAL_UART_ENABLE_IT(&uart1, UART_IT_IDLE);

    return state;
}

HAL_StatusTypeDef UART1_Transmit(const uint8_t *data, uint16_t size)
{
    if(!data) return HAL_ERROR;

    if(size > UART1_TX_BUFF_MAX_SIZE) 
        size = UART1_TX_BUFF_MAX_SIZE;
    
    return HAL_UART_Transmit(&uart1, data, size, UART1_TX_TIMEOUT_MS);
}

HAL_StatusTypeDef UART1_Transmit_DMA(const uint8_t *data, uint16_t size)
{
    if(!data) return HAL_ERROR;

    if(size > UART1_TX_BUFF_MAX_SIZE) 
        size = UART1_TX_BUFF_MAX_SIZE;

    memcpy(txBuf, data, size);
    
    return HAL_UART_Transmit_DMA(&uart1, txBuf, size);
}

UART_HandleTypeDef* UART1_GetHandle(void)
{
    return &uart1;
}

DMA_HandleTypeDef* UART1_GetTxDMA(void)
{
    return &dmaTx;
}

DMA_HandleTypeDef* UART1_GetRxDMA(void)
{
    return &dmaRx;
}

volatile bool* UART1_GetRxFlag(void)
{
    return &rxFlag;
}

uint8_t* UART1_GetRxBuf(uint16_t *size)
{
    *size = rxSize;

    return rxBuf;
}