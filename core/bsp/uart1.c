#include "stm32f1xx_hal.h"
#include "uart1.h"

#include <stdint.h>
#include <string.h>

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

// 非环形缓冲区
static void idleProcess(void)
{
    if (__HAL_UART_GET_FLAG(&uart1, UART_FLAG_IDLE) == SET) {
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

void UART1_Init(void)
{
    uart1.Instance = USART1;                        // 外设基地址指针
    uart1.Init.BaudRate = 115200;                   // 波特率
    uart1.Init.WordLength = UART_WORDLENGTH_8B;     // 8位字长
    uart1.Init.StopBits = UART_STOPBITS_1;          // 1位停止位
    uart1.Init.Parity = UART_PARITY_NONE;           // 无校验
    uart1.Init.Mode = UART_MODE_TX_RX;              // 发送和接收模式
    uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;     // 无硬件控制流
    uart1.Init.OverSampling = UART_OVERSAMPLING_16; // 采样频率

    HAL_UART_Init(&uart1);

    HAL_UART_Receive_DMA(&uart1, rxBuf, UART1_RX_BUFF_MAX_SIZE);
    __HAL_UART_ENABLE_IT(&uart1, UART_IT_IDLE);
}

void UART1_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = { 0 };

    if (huart->Instance == USART1) {
        /********************* PA9 PA10 *********************/
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_9;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);

        // F1 RX 必须浮空输入
        gpio.Pin = GPIO_PIN_10;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);

        /********************* DMA TX *********************/
        dmaTx.Instance = DMA1_Channel4;
        dmaTx.Init.Direction = DMA_MEMORY_TO_PERIPH;          // 内存 -> 外设
        dmaTx.Init.PeriphInc = DMA_PINC_DISABLE;              // 外设地址不自增
        dmaTx.Init.MemInc = DMA_MINC_ENABLE;                  // 内存地址自增
        dmaTx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; // 外设数据宽度: Byte
        dmaTx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    // 内存数据宽度: Byte
        dmaTx.Init.Mode = DMA_NORMAL;                         // 正常模式(不循环)
        dmaTx.Init.Priority = DMA_PRIORITY_LOW;               // 优先级: 低

        HAL_DMA_Init(&dmaTx);
        __HAL_LINKDMA(huart, hdmatx, dmaTx);

        /********************* DMA RX *********************/
        dmaRx.Instance = DMA1_Channel5;
        dmaRx.Init.Direction = DMA_PERIPH_TO_MEMORY; // 外设 -> 内存
        dmaRx.Init.PeriphInc = DMA_PINC_DISABLE;
        dmaRx.Init.MemInc = DMA_MINC_ENABLE;
        dmaRx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        dmaRx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        dmaRx.Init.Mode = DMA_NORMAL;
        dmaRx.Init.Priority = DMA_PRIORITY_MEDIUM; // 优先级: 中

        HAL_DMA_Init(&dmaRx);
        __HAL_LINKDMA(huart, hdmarx, dmaRx);

        /********************* NVIC enable *********************/
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    }
}

HAL_StatusTypeDef UART1_Transmit(const uint8_t *data, uint16_t size)
{
    if (!data) return HAL_ERROR;

    if (size > UART1_TX_BUFF_MAX_SIZE)
        size = UART1_TX_BUFF_MAX_SIZE;

    return HAL_UART_Transmit(&uart1, data, size, UART1_TX_TIMEOUT_MS);
}

HAL_StatusTypeDef UART1_Transmit_DMA(const uint8_t *data, uint16_t size)
{
    if (!data) return HAL_ERROR;

    if (size > UART1_TX_BUFF_MAX_SIZE)
        size = UART1_TX_BUFF_MAX_SIZE;

    memcpy(txBuf, data, size);

    return HAL_UART_Transmit_DMA(&uart1, txBuf, size);
}

UART_HandleTypeDef *UART1_GetHandle(void)
{
    return &uart1;
}

volatile bool *UART1_GetRxFlag(void)
{
    return &rxFlag;
}

uint8_t *UART1_GetRxBuf(uint16_t *size)
{
    *size = rxSize;

    return rxBuf;
}