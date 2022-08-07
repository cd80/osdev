#ifndef __rtc_h__
#define __rtc_h__

#include "types.h"

#define RTC_CMOSADDRESS     0x70
#define RTC_CMOSDATA        0x71

#define RTC_ADDRESS_SECOND      0x00
#define RTC_ADDRESS_MINUTE      0x02
#define RTC_ADDRESS_HOUR        0x04
#define RTC_ADDRESS_DAYOFWEEK   0x06
#define RTC_ADDRESS_DAYOFMONTH  0x07
#define RTC_ADDRESS_MONTH       0x08
#define RTC_ADDRESS_YEAR        0x09

#define RTC_BCDTOBINARY(x)      ((((x) >> 4) * 10) + ((x) & 0x0F))

void read_rtc_time(BYTE *hour, BYTE *minute, BYTE *second);
void read_rtc_date(WORD *year, BYTE *month, BYTE *day_of_month, 
                    BYTE *day_of_week);
char *convert_day_to_string(BYTE day_of_week);

#endif