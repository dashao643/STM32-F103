#include "stm32f1xx_hal.h"
#include "uart1.h"
#include "i2c1.h"

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
    UART1_MspInit(huart);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    I2C1_MspInit(hi2c);
}