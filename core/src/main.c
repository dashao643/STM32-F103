#include "stm32f1xx_hal.h"
#include "general.h"
#include "key.h"
#include "led.h"
#include "uart1.h"
#include "w25q64.h"
#include "rtc.h"
#include "ssd1306.h"
// #include "ssd1306_image.h"
#include "modbus.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// #define RTC_LSE_ON
#define RTC_LSI_ON

void SystemClock_Config(void);
void Error_Handler(void);


// static uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
static uint8_t data[4096];

int main(void)
{
	// NVIC_SetVectorTable(FLASH_BASE | BOOTLOADER_SIZE);
	HAL_Init();
	SystemClock_Config();

	LED_Init();
	Key_Init();
	UART1_Init();
	W25Q64_Init();
	SSD1306_Init();
	RTC_Init();
	// uint32_t timer = HAL_GetTick();

	printf("f1 project\n");
	// SSD1306_ShowString(1, 1, "test01");
	// SSD1306_ShowString(1, 1, "wo shi 浣犲ソ");
	// SSD1306_ShowString(2, 1, "浣犲ソ澶у嫼, 涓栫晫dashao");
	// SSD1306_ShowImage(IMAGE_BOQI);

	while(1) 
	{
		KeyNumTypeDef keyNum = Key_Read();
		Modbus_Task();
		RTC_Task();

		// 鎿﹂櫎鍓?4涓墖鍖?
		if (READ_BIT(keyNum, KEY_1)) {
			for(uint8_t i = 0; i < 4; i++) {
				W25Q64_EraseSector(i);
			}
		} 
		if (READ_BIT(keyNum, KEY_2)) {
			uint8_t state = W25Q64_ReadSector(0, data, 4096);
			printf("state=%d\n", state);

			for(uint16_t i = 0; i < 4096; i++) {
				printf("%02X ", data[i]);
			}
			printf("\n");
		} 
		if (READ_BIT(keyNum, KEY_3)) {
			SSD1306_ShowFont(1, 1, "是h你y吗z大*勺2有");
			SSD1306_ShowFont(2, 1, "擦随风您付款了什么的卢卡斯");
			SSD1306_ShowFont(3, 1, "破格发送弹幕克劳福德");
			SSD1306_ShowFont(4, 1, "企鹅请问如果会影响农村");
		} 
		if (READ_BIT(keyNum, KEY_4)) {
			uint8_t state = W25Q64_Check();
			printf("state=%d\n", state);
		}
	}
}

void SystemClock_Config(void)
{
	// 鍒濆鍖栨櫠鎸?
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};

#if defined RTC_LSE_ON
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
#elif defined RTC_LSI_ON
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
#endif

	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;		// 8MHz
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;				// 72MHz
	// RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;				// 16MHz

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		Error_Handler();

	// 鍒濆鍖栫郴缁熶富鏃堕挓
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
								| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
		Error_Handler();

	// 鍒濆鍖栧璁炬椂閽?
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
#if defined RTC_LSE_ON
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#elif defined RTC_LSI_ON
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
#endif
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
