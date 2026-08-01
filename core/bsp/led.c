#include "stm32f1xx_hal.h"
#include "led.h"

#define LED_GPIO_Port			GPIOB

#define LED_RED_Pin 			GPIO_PIN_12
#define LED_RED_GPIO_Port 		GPIOB

#define LED_GREEN_Pin 			GPIO_PIN_13
#define LED_GREEN_GPIO_Port 	GPIOB

#define LED_BLUE_Pin 			GPIO_PIN_14
#define LED_BLUE_GPIO_Port 		GPIOB

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOB_CLK_ENABLE();
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_RED_Pin | LED_GREEN_Pin | LED_BLUE_Pin, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = LED_RED_Pin | LED_GREEN_Pin | LED_BLUE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

	__HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

inline void LED_RED_Toggle(void)
{
	HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
}

inline void LED_GREEN_Toggle(void)
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

inline void LED_BLUE_Toggle(void)
{
	HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
}

inline void LED_PC13_Toggle(void)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}