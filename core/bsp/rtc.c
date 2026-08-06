#include "rtc.h"

static RTC_HandleTypeDef rtc;

// 把日期写入BKP寄存器，掉电后VBAT保持数据
static void saveDateToBKP(const RTC_DateTypeDef *sDate)
{
    HAL_RTCEx_BKUPWrite(&rtc, BKUP_REG_YEAR, sDate->Year);
    HAL_RTCEx_BKUPWrite(&rtc, BKUP_REG_MONTH, sDate->Month);
    HAL_RTCEx_BKUPWrite(&rtc, BKUP_REG_DATE, sDate->Date);
}

// 从BKP寄存器恢复日期
static void loadDateFromBKP(RTC_DateTypeDef *sDate)
{
    sDate->Year = HAL_RTCEx_BKUPRead(&rtc, BKUP_REG_YEAR);
    sDate->Month = HAL_RTCEx_BKUPRead(&rtc, BKUP_REG_MONTH);
    sDate->Date = HAL_RTCEx_BKUPRead(&rtc, BKUP_REG_DATE);
}

/*-----------------------------------------------------------------*/

void RTC_Init(void)
{
    rtc.Instance = RTC;
    rtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
    rtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;

    HAL_RTC_Init(&rtc);

    if(RTC_IsConfigured()) return;

    // 设置默认时间
    RTC_DateTypeDef date = {
        .Year = 10,
        .Month = 1,
        .Date = 1
    };
    RTC_TimeTypeDef time = {
        .Hours = 12,
        .Minutes = 0,
        .Seconds = 0
    };

    RTC_SetDateTime(&date, &time);
}

void RTC_MspInit(RTC_HandleTypeDef *rtc)
{
    if(rtc->Instance == RTC) {
        HAL_PWR_EnableBkUpAccess();
        __HAL_RCC_BKP_CLK_ENABLE();
        __HAL_RCC_RTC_ENABLE();

        // HAL_NVIC_SetPriority(RTC_IRQn, 1, 0);
        // HAL_NVIC_EnableIRQ(RTC_IRQn);
    }
}

// 检查RTC是否被校准过(BKP中是否有标志),在 MX_RTC_Init 中调用
bool RTC_IsConfigured(void)
{
    if (HAL_RTCEx_BKUPRead(&rtc, BKUP_REG_FLAG) != BKUP_FLAG_MAGIC)
        return false;

    RTC_DateTypeDef sDate;
    loadDateFromBKP(&sDate);
    // 恢复日期影子寄存器，这样后续GetTime检测到天溢出时能正确推进日期
    HAL_RTC_SetDate(&rtc, &sDate, RTC_FORMAT_BIN);
    return true;
}

// 读取当前日期时间:必须先GetTime再GetDate，GetTime内部会检测天溢出并更新日期影子
void RTC_GetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
    HAL_RTC_GetTime(&rtc, time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&rtc, date, RTC_FORMAT_BIN);
}

// 设置日期时间:必须先SetDate(写入日期影子)再SetTime(写入CNT计数器),否则日期不生效
void RTC_SetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
    HAL_RTC_SetDate(&rtc, date, RTC_FORMAT_BIN);
    HAL_RTC_SetTime(&rtc, time, RTC_FORMAT_BIN);
    saveDateToBKP(date);
    HAL_RTCEx_BKUPWrite(&rtc, BKUP_REG_FLAG, BKUP_FLAG_MAGIC);
}

// 清除BKP标志
void RTC_ResetConfig(void)
{
    HAL_RTCEx_BKUPWrite(&rtc, BKUP_REG_FLAG, 0);
}

// 在 RTC_IRQHandler 中调用 HAL_RTC_AlarmIRQHandler
// 能够唤醒 F1 待机模式 不能唤醒 停止模式
// 传入 NULL 为默认 5s 后触发闹钟中断
void RTC_SetAlarm_IT(const RTC_TimeTypeDef *time)
{
    // The HAL_RTC_SetTime() must be called before enabling the Alarm feature.
    RTC_DateTypeDef curDate;
    RTC_TimeTypeDef curTime;
    RTC_GetDateTime(&curDate, &curTime);
    RTC_SetDateTime(&curDate, &curTime);

    RTC_AlarmTypeDef alarm;
    alarm.Alarm = RTC_ALARM_A;

    if (time) {
        alarm.AlarmTime.Hours = time->Hours;
        alarm.AlarmTime.Minutes = time->Minutes;
        alarm.AlarmTime.Seconds = time->Seconds;
    } else {
        alarm.AlarmTime.Hours = curTime.Hours;
        alarm.AlarmTime.Minutes = curTime.Minutes;
        alarm.AlarmTime.Seconds = curTime.Seconds + 5; // 不考虑溢出
    }
    HAL_RTC_SetAlarm_IT(&rtc, &alarm, RTC_FORMAT_BIN);
}

//
#include "ssd1306.h"
#include "stdio.h"

void RTC_Task(void)
{
    static uint32_t rtcTimer;

    if(HAL_GetTick() - rtcTimer < 1000) return;

    rtcTimer = HAL_GetTick();

    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    RTC_GetDateTime(&date, &time);

    char dateBuf[16] = {0};
    char timeBuf[16] = {0};

    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);

    // 一行显示
    snprintf(dateBuf, sizeof(dateBuf), "%02d-%02d", date.Month, date.Date);
    SSD1306_ShowString(4, 1, dateBuf);
    SSD1306_ShowString(4, 7, timeBuf);

    // 两行显示
    // snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", 2000 + date.Year, date.Month,date.Date);
    // snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
    // SSD1306_ShowString(3, 1, dateBuf);
    // SSD1306_ShowString(4, 1, timeBuf);

    // ST7735_ShowString(1, 1, dateBuf, ST7735_WHITE);
    // ST7735_ShowString(2, 1, timeBuf, ST7735_WHITE);
}