#include "stm32f1xx_hal.h"
#include "uart1.h"

// Initializes the Global MSP.
void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);

    // NOJTAG: JTAG-DP Disabled and SW-DP Enabled
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio = {0};

#ifdef ENABLE_UART1
    if(huart->Instance == USART1) {
        /********************* PA9 PA10 *********************/
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_9;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);

        // F1 必须浮空输入
        gpio.Pin = GPIO_PIN_10;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);

        /********************* DMA TX *********************/
        DMA_HandleTypeDef *dmaTx = UART1_GetTxDMA();

        dmaTx->Instance = DMA1_Channel4;
        dmaTx->Init.Direction = DMA_MEMORY_TO_PERIPH;            // 内存 -> 外设
        dmaTx->Init.PeriphInc = DMA_PINC_DISABLE;                // 外设地址不自增
        dmaTx->Init.MemInc = DMA_MINC_ENABLE;                    // 内存地址自增
        dmaTx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;   // 外设数据宽度: Byte
        dmaTx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;      // 内存数据宽度: Byte
        dmaTx->Init.Mode = DMA_NORMAL;                           // 正常模式(不循环)
        dmaTx->Init.Priority = DMA_PRIORITY_LOW;                 // 优先级: 低

        HAL_DMA_Init(dmaTx);
        __HAL_LINKDMA(huart, hdmatx, *dmaTx);

        /********************* DMA RX *********************/
        DMA_HandleTypeDef *dmaRx = UART1_GetRxDMA();

        dmaRx->Instance = DMA1_Channel5;
        dmaRx->Init.Direction = DMA_PERIPH_TO_MEMORY;            // 外设 -> 内存
        dmaRx->Init.PeriphInc = DMA_PINC_DISABLE;
        dmaRx->Init.MemInc = DMA_MINC_ENABLE;
        dmaRx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        dmaRx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        dmaRx->Init.Mode = DMA_NORMAL;
        dmaRx->Init.Priority = DMA_PRIORITY_MEDIUM;              // 优先级: 中

        HAL_DMA_Init(dmaRx);
        __HAL_LINKDMA(huart, hdmarx, *dmaRx);

        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 4, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

        HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    }
#endif
}