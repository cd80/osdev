#include "rtc.h"
#include "helper_asm.h"

void read_rtc_time(BYTE *hour, BYTE *minute, BYTE *second) {
    BYTE data;

    out1(RTC_CMOSADDRESS, RTC_ADDRESS_HOUR);
    *hour = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));

    out1(RTC_CMOSADDRESS, RTC_ADDRESS_MINUTE);
    *minute = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));

    out1(RTC_CMOSADDRESS, RTC_ADDRESS_SECOND);
    *second = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));
}

void read_rtc_date(WORD *year, BYTE *month, BYTE *day_of_month,
                    BYTE *day_of_week) {
    out1(RTC_CMOSADDRESS, RTC_ADDRESS_YEAR);
    *year = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));

    out1(RTC_CMOSADDRESS, RTC_ADDRESS_MONTH);
    *month = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));

    out1(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFMONTH);
    *day_of_month = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));

    out1(RTC_CMOSADDRESS, RTC_ADDRESS_DAYOFWEEK);
    *day_of_week = RTC_BCDTOBINARY(in1(RTC_CMOSDATA));
}
char *convert_day_to_string(BYTE day_of_week) {
    static char *day_of_week_str[8] = {"Error", "Sunday", "Monday",
                    "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    
    if (day_of_week > 8) {
        return day_of_week_str[0];
    }

    return day_of_week_str[day_of_week];
}