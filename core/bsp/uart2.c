#include "uart2.h"

#include <stdint.h>
#include <string.h>

// PA2 ------> USART1_TX
// PA3 ------> USART1_RX

// ESP8266: EN, 高电平有效

static UART2_RxTypeDef rx_ = {0};
static uint8_t txBuf_[UART2_TX_BUFF_MAX_SIZE];

static UART_HandleTypeDef uart2_ = {0};
static DMA_HandleTypeDef dmaTx_ = {0};
static DMA_HandleTypeDef dmaRx_ = {0};

// 追加型缓冲区
static void idleProcess(void)
{
    if (__HAL_UART_GET_FLAG(&uart2_, UART_FLAG_IDLE) == SET) {
        __HAL_UART_CLEAR_IDLEFLAG(&uart2_);

        rx_.size = UART2_RX_BUFF_MAX_SIZE - __HAL_DMA_GET_COUNTER(uart2_.hdmarx);
        rx_.rxFlag++;

        HAL_UART_DMAStop(&uart2_);
        HAL_UART_Receive_DMA(&uart2_, rx_.rxBuf + rx_.size, UART2_RX_BUFF_MAX_SIZE - rx_.size);
    }
}

/********************* override *********************/

void DMA1_Channel6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&dmaRx_);
}

void DMA1_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&dmaTx_);
}

void USART2_IRQHandler(void)
{
    idleProcess();

    HAL_UART_IRQHandler(&uart2_);
}

/********************* public *********************/

void UART2_Init(void)
{
    uart2_.Instance = USART2;
    uart2_.Init.BaudRate = 115200;
    uart2_.Init.WordLength = UART_WORDLENGTH_8B;
    uart2_.Init.StopBits = UART_STOPBITS_1;
    uart2_.Init.Parity = UART_PARITY_NONE;
    uart2_.Init.Mode = UART_MODE_TX_RX;
    uart2_.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart2_.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&uart2_);

    HAL_UART_Receive_DMA(&uart2_, rx_.rxBuf, UART2_RX_BUFF_MAX_SIZE);
    __HAL_UART_ENABLE_IT(&uart2_, UART_IT_IDLE);
}

void UART2_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = { 0 };

    if (huart->Instance == USART2) {
        /********************* PA9 PA10 *********************/
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_2;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);

        // F1 RX 必须浮空输入
        gpio.Pin = GPIO_PIN_3;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);

        /********************* DMA TX *********************/
        dmaTx_.Instance = DMA1_Channel7;
        dmaTx_.Init.Direction = DMA_MEMORY_TO_PERIPH;          // 内存 -> 外设
        dmaTx_.Init.PeriphInc = DMA_PINC_DISABLE;              // 外设地址不自增
        dmaTx_.Init.MemInc = DMA_MINC_ENABLE;                  // 内存地址自增
        dmaTx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; // 外设数据宽度: Byte
        dmaTx_.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    // 内存数据宽度: Byte
        dmaTx_.Init.Mode = DMA_NORMAL;                         // 正常模式(不循环)
        dmaTx_.Init.Priority = DMA_PRIORITY_LOW;               // 优先级: 低

        HAL_DMA_Init(&dmaTx_);
        __HAL_LINKDMA(huart, hdmatx, dmaTx_);

        /********************* DMA RX *********************/
        dmaRx_.Instance = DMA1_Channel6;
        dmaRx_.Init.Direction = DMA_PERIPH_TO_MEMORY; // 外设 -> 内存
        dmaRx_.Init.PeriphInc = DMA_PINC_DISABLE;
        dmaRx_.Init.MemInc = DMA_MINC_ENABLE;
        dmaRx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        dmaRx_.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        dmaRx_.Init.Mode = DMA_NORMAL;
        dmaRx_.Init.Priority = DMA_PRIORITY_MEDIUM; // 优先级: 中

        HAL_DMA_Init(&dmaRx_);
        __HAL_LINKDMA(huart, hdmarx, dmaRx_);

        /********************* NVIC enable *********************/
        HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    }
}

HAL_StatusTypeDef UART2_Transmit(const uint8_t *data, uint16_t size)
{
    if (!data) return HAL_ERROR;

    if (size > UART2_TX_BUFF_MAX_SIZE)
        size = UART2_TX_BUFF_MAX_SIZE;

    return HAL_UART_Transmit(&uart2_, data, size, UART2_TX_TIMEOUT_MS);
}

HAL_StatusTypeDef UART2_Transmit_DMA(const uint8_t *data, uint16_t size)
{
    if (!data) return HAL_ERROR;

    if (size > UART2_TX_BUFF_MAX_SIZE)
        size = UART2_TX_BUFF_MAX_SIZE;

    memcpy(txBuf_, data, size);

    return HAL_UART_Transmit_DMA(&uart2_, txBuf_, size);
}

UART_HandleTypeDef* UART2_GetHandle(void)
{
    return &uart2_;
}

UART2_RxTypeDef* UART2_GetRxStruct(void)
{
    return &rx_;
}

// 处理完缓冲区数据后调用
void UART2_GetRxClear(void)
{
    rx_.rxFlag = 0;

    HAL_UART_DMAStop(&uart2_);
    HAL_UART_Receive_DMA(&uart2_, rx_.rxBuf, UART2_RX_BUFF_MAX_SIZE);
}

// void UART2_Task(void)
// {
//     if(rx_.rxFlag > 0) {
//         uint8_t len = rx_.size;
//         UART2_Transmit(&len, 1);

//         UART2_GetRxClear();
//     }
// }
