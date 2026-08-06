#include "spi.h"

#ifdef ENABLE_SPI1
SPI_HandleTypeDef spi1;
#ifdef REMAP_SPI1
// PA5 ------> SPI1_SCK
// PA6 ------> SPI1_MISO
// PA7 ------> SPI1_MOSI
#else
// PB3 ------> SPI1_SCK
// PB4 ------> SPI1_MISO
// PB5 ------> SPI1_MOSI
#endif
#endif

#ifdef ENABLE_SPI2
SPI_HandleTypeDef spi2;
// PB13 ------> SPI2_SCK
// PB14 ------> SPI2_MISO
// PB15 ------> SPI2_MOSI
#endif

void SPI1_Init(void)
{
#ifdef ENABLE_SPI1
    spi1.Instance = SPI1;
    spi1.Init.Mode = SPI_MODE_MASTER;
    spi1.Init.Direction = SPI_DIRECTION_2LINES;
    spi1.Init.DataSize = SPI_DATASIZE_8BIT;
    spi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    spi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    spi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;  // 72 / 4 = 18(max)
    spi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi1.Init.TIMode = SPI_TIMODE_DISABLE;
    spi1.Init.NSS = SPI_NSS_SOFT;
    spi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi1.Init.CRCPolynomial = 10;

    HAL_SPI_Init(&spi1);
#endif
}

void SPI2_Init(void)
{
#ifdef ENABLE_SPI2
    spi2.Instance = SPI2;
    spi2.Init.Mode = SPI_MODE_MASTER;
    spi2.Init.Direction = SPI_DIRECTION_2LINES;
    spi2.Init.DataSize = SPI_DATASIZE_8BIT;
    spi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    spi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    spi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    spi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    spi2.Init.TIMode = SPI_TIMODE_DISABLE;
    spi2.Init.NSS = SPI_NSS_SOFT;
    spi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    spi2.Init.CRCPolynomial = 10;

    HAL_SPI_Init(&spi2);
#endif
}

void SPI1_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio;

    if(hspi->Instance == SPI1) {
        __HAL_RCC_SPI1_CLK_ENABLE();

#ifdef REMAP_SPI1
        __HAL_RCC_GPIOA_CLK_ENABLE();
        gpio.Pin = GPIO_PIN_5 | GPIO_PIN_7;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio);
        
        gpio.Pin = GPIO_PIN_6;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);

        __HAL_AFIO_REMAP_SPI1_ENABLE();
#else
        __HAL_RCC_GPIOB_CLK_ENABLE();
        gpio.Pin = GPIO_PIN_3 | GPIO_PIN_5;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &gpio);

        gpio.Pin = GPIO_PIN_4;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &gpio);
#endif
    }
}

void SPI2_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio;

    if(hspi->Instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_13 | GPIO_PIN_15;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &gpio);

        gpio.Pin = GPIO_PIN_14;
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &gpio);
    }
}

HAL_StatusTypeDef SPI_Transmit(SPI_TypeDef *Instance, const uint8_t *data, uint16_t size)
{
#ifdef ENABLE_SPI1
    if(Instance == SPI1)
        return HAL_SPI_Transmit(&spi1, data, size, SPI_TIMEOUT_MS);
#endif
#ifdef ENABLE_SPI2
    if(Instance == SPI2)
        return HAL_SPI_Transmit(&spi2, data, size, SPI_TIMEOUT_MS);
#endif

    return HAL_ERROR;
}

HAL_StatusTypeDef SPI_Receive(SPI_TypeDef *Instance, uint8_t *data, uint16_t size)
{
#ifdef ENABLE_SPI1
    if(Instance == SPI1)
        return HAL_SPI_Receive(&spi1, data, size, SPI_TIMEOUT_MS);
#endif
#ifdef ENABLE_SPI2
    if(Instance == SPI2)
        return HAL_SPI_Receive(&spi2, data, size, SPI_TIMEOUT_MS);
#endif

    return HAL_ERROR;
}