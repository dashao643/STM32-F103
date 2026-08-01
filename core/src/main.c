#include "stm32f1xx_hal.h"
#include "general.h"
#include "uart1.h"
#include "led.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void SystemClock_Config(void);
void Error_Handler(void);

// static uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};

int main(void)
{
	// NVIC_SetVectorTable(FLASH_BASE | BOOTLOADER_SIZE);

	HAL_Init();
	SystemClock_Config();

	LED_Init();
	UART1_Init();

	// uint32_t timer = HAL_GetTick();

	UART1_Transmit_DMA(data, sizeof(data));

	printf("f1 project\n");

	while(1) 
	{
		if(*UART1_GetRxFlag()) {
			*UART1_GetRxFlag() = false;
			uint16_t size;
			uint8_t *data = UART1_GetRxBuf(&size);
			UART1_Transmit_DMA(data, size);
			// printf("recevie\n");
		}
		// UART1_Task();
		// if(HAL_GetTick() - timer > 500) {
		// 	timer = HAL_GetTick();
		// 	LED_RED_Toggle();
		// 	// LED_GREEN_Toggle();
		// 	// LED_BLUE_Toggle();
		// 	// LED_PC13_Toggle();
		// }
		// HAL_Delay(500);
	}
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};

	// RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
	// 						|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	// RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		Error_Handler();

	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
		Error_Handler();

	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
		Error_Handler();
}

void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
	}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
