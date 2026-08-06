#ifndef __RTC_H__
#define __RTC_H__

#include "stm32f1xx_hal.h"
#include <stdbool.h>

// F1 RTC 专用 数据格式为二进制
// F1 只有32位秒计数器(不支持日历)
// HAL使用软件 DateToUpdate 影子来跟踪日历翻转时的年/月/日
// 日期数据被保存在备份(BKP)寄存器中

#define BKUP_FLAG_MAGIC     0xAA55
#define BKUP_REG_FLAG       RTC_BKP_DR1
#define BKUP_REG_YEAR       RTC_BKP_DR2
#define BKUP_REG_MONTH      RTC_BKP_DR3
#define BKUP_REG_DATE       RTC_BKP_DR4

void RTC_Init(void);
void RTC_MspInit(RTC_HandleTypeDef *hrtc);

bool RTC_IsConfigured(void);
void RTC_GetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
void RTC_SetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
void RTC_ResetConfig(void);
void RTC_SetAlarm_IT(const RTC_TimeTypeDef *time);


void RTC_Task(void);

#endif
