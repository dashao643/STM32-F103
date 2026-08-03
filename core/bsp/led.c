#include "stm32f1xx_hal.h"
#include "led.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/********************* pbx led *********************/
	__HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(LED_GPIO_Port, LED_RED_Pin | LED_GREEN_Pin | LED_BLUE_Pin, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = LED_RED_Pin | LED_GREEN_Pin | LED_BLUE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

	/********************* pc13 led *********************/
	__HAL_RCC_GPIOC_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
