#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <stdint.h>

// #define I2C_HARDWARE
#define I2C_SOFTWARE

#define GB2312_FONT_LIBRARY

void SSD1306_Init(void);
void SSD1306_Clear(void);
void SSD1306_ShowALL(void);
void SSD1306_SetReverse(void);

// 4行，16列
void SSD1306_ShowChar(uint8_t row, uint8_t col, char ch);
void SSD1306_ShowString(uint8_t row, uint8_t col, const char str[]);
void SSD1306_ShowFont(uint8_t row, uint8_t col, const char font[]);
void SSD1306_ShowDecNumber(uint8_t row, uint8_t col, int32_t number, uint8_t numLen);
void SSD1306_ShowHexNumber(uint8_t row, uint8_t col, const uint8_t data[], uint8_t size);
void SSD1306_ShowImage(const uint8_t *image);

#endif
