#ifndef __LED_H__
#define __LED_H__

#define LED_GPIO_Port			GPIOB

#define LED_RED_GPIO_Port 		GPIOB
#define LED_RED_Pin 			GPIO_PIN_12

#define LED_GREEN_GPIO_Port 	GPIOB
#define LED_GREEN_Pin 			GPIO_PIN_13

#define LED_BLUE_GPIO_Port 		GPIOB
#define LED_BLUE_Pin 			GPIO_PIN_14

void LED_Init(void);

#ifdef LED_RED_Pin
#define LED_RED_ON()        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET)
#define LED_RED_OFF()       HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET)
#define LED_RED_TOGGLE()    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin)
#endif

#ifdef LED_GREEN_Pin
#define LED_GREEN_ON()      HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET)
#define LED_GREEN_OFF()     HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET)
#define LED_GREEN_TOGGLE()  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin)
#endif

#ifdef LED_BLUE_Pin
#define LED_BLUE_ON()       HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET)
#define LED_BLUE_OFF()      HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET)
#define LED_BLUE_TOGGLE()   HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin)
#endif

#ifdef LED_YELLOW_Pin
#define LED_YELLOW_ON()     HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET)
#define LED_YELLOW_OFF()    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET)
#define LED_YELLOW_TOGGLE() HAL_GPIO_TogglePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin)
#endif

#endif
