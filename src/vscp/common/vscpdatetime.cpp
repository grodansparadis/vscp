///////////////////////////////////////////////////////////////////////////////
// datetime.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <string>

#include <stdio.h>
#include <time.h>

#include "vscpdatetime.h"
#include <vscp.h>
#include <vscphelper.h>

///////////////////////////////////////////////////////////////////////////////
// ctor
//

vscpdatetime::vscpdatetime(void)
{
    setNow();
    m_millisecond = 0;
}

vscpdatetime::vscpdatetime(const std::string &dt)
{
    set(dt);
    m_millisecond = 0;
}

vscpdatetime::vscpdatetime(vscpEvent &ev)
{
    set(ev);
    m_millisecond = 0;
}

vscpdatetime::vscpdatetime(vscpEventEx &ex)
{
    set(ex);
    m_millisecond = 0;
}

vscpdatetime::vscpdatetime(uint16_t year,
                           uint8_t month,
                           uint8_t day,
                           uint8_t hour,
                           uint8_t minute,
                           uint8_t second,
                           uint32_t millisecond)
{
    setYear(year);
    setMonth(month);
    setDay(day);
    setHour(hour);
    setMinute(minute);
    setSecond(second);
    setMilliSecond(millisecond);
}

///////////////////////////////////////////////////////////////////////////////
// dstor
//

vscpdatetime::~vscpdatetime(void)
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// setDate
//

bool
vscpdatetime::setDate(uint16_t year, uint8_t month, uint8_t day)
{
    if (!setYear(year)) return false;
    if (!setMonth(month)) return false;
    if (!setDay(day)) return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setISODate
//

bool
vscpdatetime::setISODate(const std::string &strDate)
{
    size_t pos;
    std::string isodt = strDate.c_str();

    try {
        // year
        m_year = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        // month
        m_month = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        // day
        m_day = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past 'T' or ' '
        isodt = isodt.substr(pos);

    } catch (...) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setTime
//

bool
vscpdatetime::setTime(uint8_t hour,
                      uint8_t minute,
                      uint8_t second,
                      uint32_t millisecond)
{
    if (!setHour(hour)) return false;
    if (!setMinute(minute)) return false;
    if (!setSecond(second)) return false;
    setMilliSecond(millisecond);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setISOTime
//

bool
vscpdatetime::setISOTime(const std::string &strTime)
{
    size_t pos;
    std::string isodt = strTime.c_str();

    try {
        // hour
        m_hour = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        // minute
        m_minute = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        // second
        m_second = std::stoi(isodt.c_str(), &pos);
    } catch (...) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setYear
//

bool
vscpdatetime::setYear(uint16_t year)
{
    m_year = year;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setMonth
//

bool
vscpdatetime::setMonth(uint8_t month)
{
    if (!month || (month > 12)) return false;
    m_month = month;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setDay
//

bool
vscpdatetime::setDay(uint8_t day)
{
    if (!day || (day > 31)) return false;
    m_day = day;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setHour
//

bool
vscpdatetime::setHour(uint8_t hour)
{
    if (hour > 23) return false;
    m_hour = hour;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setMinute
//

bool
vscpdatetime::setMinute(uint8_t minute)
{
    if (minute > 59) return false;
    m_minute = minute;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setSecond
//

bool
vscpdatetime::setSecond(uint8_t second)
{
    if (second > 59) return false;
    m_second = second;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setMilliSecond
//

bool
vscpdatetime::setMilliSecond(uint32_t millisecond)
{
    m_millisecond = millisecond;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// set
//

bool
vscpdatetime::set(uint16_t year,
                  uint8_t month,
                  uint8_t day,
                  uint8_t hour,
                  uint8_t minute,
                  uint8_t second,
                  uint32_t millisecond)
{
    if (!setYear(year)) return false;
    if (!setMonth(month)) return false;
    if (!setDay(day)) return false;
    if (!setHour(hour)) return false;
    if (!setMinute(minute)) return false;
    if (!setSecond(second)) return false;
    setMilliSecond(millisecond);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// set
//

bool
vscpdatetime::set(std::string dt)
{
    size_t pos;
    std::string isodt = dt.c_str();

    try {
        // year
        m_year = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        // month
        m_month = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        // day
        m_day = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past 'T' or ' '
        isodt = isodt.substr(pos);

        // hour
        m_hour = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        // minute
        m_minute = std::stoi(isodt.c_str(), &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        // second
        m_second = std::stoi(isodt.c_str(), &pos);
    } catch (...) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// set
//

bool
vscpdatetime::set(const char *pdt)
{
    if (NULL == pdt) return false;
    std::string str(pdt);
    return set(str);
}

///////////////////////////////////////////////////////////////////////////////
// set
//

bool
vscpdatetime::set(vscpEvent &ev)
{
    if (!set(ev.year, ev.month, ev.day, ev.hour, ev.minute, ev.second))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// set
//

bool
vscpdatetime::set(vscpEventEx &ex)
{
    if (!set(ex.year, ex.month, ex.day, ex.hour, ex.minute, ex.second))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// set
//

bool
vscpdatetime::set(const struct tm &tm)
{
    m_year   = tm.tm_year + 1900;
    m_month  = tm.tm_mon;
    m_day    = tm.tm_mday;
    m_hour   = tm.tm_hour;
    m_minute = tm.tm_min;
    m_second = tm.tm_sec;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setNow
//

void
vscpdatetime::setNow(void)
{
    time_t rawtime;
    struct tm *ptm;

    time(&rawtime);
    ptm = localtime(&rawtime);

    m_year = ptm->tm_year + 1900;
    m_month = ptm->tm_mon;
    m_day = ptm->tm_mday;
    m_hour = ptm->tm_hour;
    m_minute = ptm->tm_min;
    m_second = ptm->tm_sec;

}

///////////////////////////////////////////////////////////////////////////////
// setUTCNow
//

void
vscpdatetime::setUTCNow(void)
{
    time_t rawtime;
    struct tm *ptm;

    time(&rawtime);
    ptm = gmtime(&rawtime);

    m_year = ptm->tm_year + 1900;
    m_month = ptm->tm_mon;
    m_day = ptm->tm_mday;
    m_hour = ptm->tm_hour;
    m_minute = ptm->tm_min;
    m_second = ptm->tm_sec;

}

///////////////////////////////////////////////////////////////////////////////
// Now
//

vscpdatetime
vscpdatetime::Now(void)
{
    time_t rawtime;
    struct tm *ptm;

    time(&rawtime);
    ptm = localtime(&rawtime);

    /*m_year = ptm->tm_year + 1900;
    m_month = ptm->tm_mon;
    m_day = ptm->tm_mday;
    m_hour = ptm->tm_hour;
    m_minute = ptm->tm_min;
    m_second = ptm->tm_sec;*/

    return vscpdatetime(*ptm);
}

///////////////////////////////////////////////////////////////////////////////
// UTCNow
//

vscpdatetime
vscpdatetime::UTCNow(void)
{
    time_t rawtime;
    struct tm *ptm;

    time(&rawtime);
    ptm = gmtime(&rawtime);

    /*m_year = ptm->tm_year + 1900;
    m_month = ptm->tm_mon;
    m_day = ptm->tm_mday;
    m_hour = ptm->tm_hour;
    m_minute = ptm->tm_min;
    m_second = ptm->tm_sec;*/

    return vscpdatetime(*ptm);
}

///////////////////////////////////////////////////////////////////////////////
// zeroTime
//

vscpdatetime
vscpdatetime::zeroTime(void)
{
    // m_hour = m_minute = m_second = m_millisecond = 0;
    return vscpdatetime(0, 0, 0, 0, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// zeroDate
//

vscpdatetime
vscpdatetime::zeroDate(void)
{
    // m_year = m_month = m_day = 0;
    return vscpdatetime(0, 0, 0, 0, 0, 0, 0);
    ;
};

///////////////////////////////////////////////////////////////////////////////
// dateTimeZero
//

vscpdatetime
vscpdatetime::dateTimeZero(void)
{
    // set(0, 0, 0, 0, 0, 0, 0);
    return vscpdatetime(0, 0, 0, 0, 0, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
// getISODateTime
//

std::string
vscpdatetime::getISODateTime(bool bSeparator) const
{
    char buf[32];
    std::string isodt;

    const time_t t = vscpdatetime::toSysTime();
    if (bSeparator) {
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
    } else {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%SZ", gmtime(&t));
    }
    isodt = buf;

    return isodt;
}

///////////////////////////////////////////////////////////////////////////////
// getISODate
//

std::string
vscpdatetime::getISODate(void)
{
    char buf[32];
    std::string isodt;

    const time_t t = toSysTime();
    strftime(buf, sizeof(buf), "%Y-%m-%d", gmtime(&t));
    isodt = buf;

    return isodt;
}

///////////////////////////////////////////////////////////////////////////////
// getISOTime
//

std::string
vscpdatetime::getISOTime(void)
{
    char buf[32];
    std::string isodt;

    const time_t t = toSysTime();
    strftime(buf, sizeof(buf), "%H:%M:%SZ", gmtime(&t));
    isodt = buf;

    return isodt;
}

///////////////////////////////////////////////////////////////////////////////
// setFromJulian
//

void
vscpdatetime::setFromJulian(const long ljd)
{
    long a, b, c, d, e, z, alpha;

    z = ljd;
    if (z < 2299161L)
        a = z;
    else {
        alpha = (long)((z - 1867216.25) / 36524.25);
        a     = z + 1 + alpha - alpha / 4;
    }
    b       = a + 1524;
    c       = (long)((b - 122.1) / 365.25);
    d       = (long)(365.25 * c);
    e       = (long)((b - d) / 30.6001);
    m_day   = (int)b - d - (long)(30.6001 * e);
    m_month = (int)(e < 13.5) ? e - 1 : e - 13;
    m_year  = (int)(m_month > 2.5) ? (c - 4716) : c - 4715;
    if (m_year <= 0) {
        m_year -= 1;
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// YmdToJd
//
// Copyright (C) 2000, 2002
// Todd T. Knarr <tknarr@silverglass.org>
//

long
ymdToJd(const int iYear, const int iMonth, const int iDay)
{
    long jul_day;

#ifndef JULDATE_USE_ALTERNATE_METHOD

    int a, b;
    int year = iYear, month = iMonth, day = iDay;
    float year_corr;

    if (year < 0) year++;
    year_corr = (year > 0 ? 0.0 : 0.75);
    if (month <= 2) {
        year--;
        month += 12;
    }
    b = 0;
    if (year * 10000.0 + month * 100.0 + day >= 15821015.0) {
        a = year / 100;
        b = 2 - a + a / 4;
    }
    jul_day = (long)(365.25 * year - year_corr) +
              (long)(30.6001 * (month + 1)) + day + 1720995L + b;

#else

    long lmonth = (long)iMonth, lday = (long)iDay, lyear = (long)iYear;

    // Adjust BC years
    if (lyear < 0) lyear++;

    jul_day = lday - 32075L +
              1461L * (lyear + 4800L + (lmonth - 14L) / 12L) / 4L +
              367L * (lmonth - 2L - (lmonth - 14L) / 12L * 12L) / 12L -
              3L * ((lyear + 4900L + (lmonth - 14L) / 12L) / 100L) / 4L;

#endif

    return jul_day;
}

///////////////////////////////////////////////////////////////////////////////
// JdToYmd
//
// Copyright (C) 2000, 2002
// Todd T. Knarr <tknarr@silverglass.org>
//

void
JdToYmd(const long lJD, int *piYear, int *piMonth, int *piDay)
{

#ifndef JULDATE_USE_ALTERNATE_METHOD

    long a, b, c, d, e, z, alpha;

    z = lJD;
    if (z < 2299161L)
        a = z;
    else {
        alpha = (long)((z - 1867216.25) / 36524.25);
        a     = z + 1 + alpha - alpha / 4;
    }
    b        = a + 1524;
    c        = (long)((b - 122.1) / 365.25);
    d        = (long)(365.25 * c);
    e        = (long)((b - d) / 30.6001);
    *piDay   = (int)b - d - (long)(30.6001 * e);
    *piMonth = (int)(e < 13.5) ? e - 1 : e - 13;
    *piYear  = (int)(*piMonth > 2.5) ? (c - 4716) : c - 4715;
    if (*piYear <= 0) *piYear -= 1;

#else

    long t1, t2, yr, mo;

    t1       = lJD + 68569L;
    t2       = 4L * t1 / 146097L;
    t1       = t1 - (146097L * t2 + 3L) / 4L;
    yr       = 4000L * (t1 + 1L) / 1461001L;
    t1       = t1 - 1461L * yr / 4L + 31L;
    mo       = 80L * t1 / 2447L;
    *piDay   = (int)(t1 - 2447L * mo / 80L);
    t1       = mo / 11L;
    *piMonth = (int)(mo + 2L - 12L * t1);
    *piYear  = (int)(100L * (t2 - 49L) + yr + t1);

    // Correct for BC years
    if (*piYear <= 0) *piYear -= 1;

#endif

    return;
}

///////////////////////////////////////////////////////////////////////////////
// getJulian
//

long
vscpdatetime::getJulian(void)
{
    return ymdToJd(m_year, m_month, m_day);
}

///////////////////////////////////////////////////////////////////////////////
// DayOfYear
//

int
vscpdatetime::getDayOfYear(void) const
{
    long soy;
    long lJulianDay = ymdToJd(m_year, m_month, m_day);
    soy             = ymdToJd(m_year, 1, 1);
    return (int)(lJulianDay - soy + 1);
}

///////////////////////////////////////////////////////////////////////////////
// toSysTime
//

time_t
vscpdatetime::toSysTime(void) const
{
    struct tm tmRep;
    time_t t;

    tmRep.tm_year  = m_year - 1900;
    tmRep.tm_mon   = m_month - 1;
    tmRep.tm_mday  = m_day;
    tmRep.tm_hour  = m_hour;
    tmRep.tm_min   = m_minute;
    tmRep.tm_sec   = m_second;
    tmRep.tm_isdst = 0;

    t = mktime(&tmRep);
    return t;
}

///////////////////////////////////////////////////////////////////////////////
// valid
//

bool
vscpdatetime::isValid(void)
{
    int days_in_month_normal[]   = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };
    int days_in_month_leapyear[] = { 31, 29, 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31 };

    if (m_month > 12) return false;
    if (0 == m_month) return false;
    if (0 == m_day) return false;

    if (isLeapYear()) {
        if (m_day > days_in_month_leapyear[m_month]) return false;
    } else {
        if (m_day > days_in_month_normal[m_month]) return false;
    }

    if (m_hour > 23) return false;
    if (m_minute > 59) return false;
    if (m_second > 59) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isLeapYear
//

bool
vscpdatetime::isLeapYear(void)
{
    long jd1, jd2;
    jd1 = ymdToJd(m_year, 2, 28);
    jd2 = ymdToJd(m_year, 3, 1);
    return ((jd2 - jd1) > 1);
}

///////////////////////////////////////////////////////////////////////////////
// getWeekNumber
//

int
vscpdatetime::getWeekNumber(void) const
{
    constexpr int DAYS_PER_WEEK = 7;

    time_t t       = toSysTime();
    struct tm *ptm = localtime(&t);

    const int wday  = ptm->tm_wday;
    const int delta = wday ? wday - 1 : DAYS_PER_WEEK - 1;
    return (ptm->tm_yday + DAYS_PER_WEEK - delta) / DAYS_PER_WEEK;
}

///////////////////////////////////////////////////////////////////////////////
// getWeekDay
//

vscpdatetime::weekDay
vscpdatetime::getWeekDay(void) const
{
    time_t t       = toSysTime();
    struct tm *ptm = localtime(&t);
    return static_cast<weekDay>(ptm->tm_wday);
}

///////////////////////////////////////////////////////////////////////////////
// GetWeekDayName
//

std::string
vscpdatetime::getWeekDayName(weekDay weekday, nameFlags flags)
{
    std::string strday;

    if (name_Full == flags) {
        switch (weekday) {

            case Sun:
                strday = "Sunday";
                break;

            case Mon:
                strday = "Monday";
                break;

            case Tue:
                strday = "Thursday";
                break;

            case Wed:
                strday = "Wednesday";
                break;

            case Thu:
                strday = "Thursday";
                break;

            case Fri:
                strday = "Friday";
                break;

            case Sat:
                strday = "Saturday";
                break;

            default:
                strday = "---";
                break;
        }
    } else {
        switch (weekday) {

            case Sun:
                strday = "Sun";
                break;

            case Mon:
                strday = "Mon";
                break;

            case Tue:
                strday = "Thu";
                break;

            case Wed:
                strday = "Wed";
                break;

            case Thu:
                strday = "Thu";
                break;

            case Fri:
                strday = "Fri";
                break;

            case Sat:
                strday = "Sat";
                break;

            default:
                strday = "---";
                break;
        }
    }

    return strday;
}

///////////////////////////////////////////////////////////////////////////////
// getMonthName
//

std::string
vscpdatetime::getMonthName(month month, nameFlags flags)
{
    std::string strmonth;

    if (name_Full == flags) {
        switch (month) {

            case Jan:
                strmonth = "January";
                break;

            case Feb:
                strmonth = "February";
                break;

            case Mar:
                strmonth = "Mars";
                break;

            case Apr:
                strmonth = "April";
                break;

            case May:
                strmonth = "May";
                break;

            case Jun:
                strmonth = "June";
                break;

            case Jul:
                strmonth = "July";
                break;

            case Aug:
                strmonth = "August";
                break;

            case Sep:
                strmonth = "September";
                break;

            case Oct:
                strmonth = "October";
                break;

            case Nov:
                strmonth = "November";
                break;

            case Dec:
                strmonth = "December";
                break;

            default:
                strmonth = "---";
                break;
        }
    } else {
        switch (month) {

            case Jan:
                strmonth = "Jan";
                break;

            case Feb:
                strmonth = "Feb";
                break;

            case Mar:
                strmonth = "Mar";
                break;

            case Apr:
                strmonth = "Apr";
                break;

            case May:
                strmonth = "May";
                break;

            case Jun:
                strmonth = "Jun";
                break;

            case Jul:
                strmonth = "Jul";
                break;

            case Aug:
                strmonth = "Aug";
                break;

            case Sep:
                strmonth = "Sep";
                break;

            case Oct:
                strmonth = "Oct";
                break;

            case Nov:
                strmonth = "Nov";
                break;

            case Dec:
                strmonth = "Dec";
                break;

            default:
                strmonth = "---";
                break;
        }
    }

    return strmonth;
}

///////////////////////////////////////////////////////////////////////////////
// getMonthName
//
// https://stackoverflow.com/questions/13804095/get-the-time-zone-gmt-offset-in-c
//
//

int
vscpdatetime::tzOffset2LocalTime(void)
{
    time_t gmt, rawtime = time(NULL);
    struct tm *ptm;

#if !defined(WIN32)
    struct tm gbuf;
    ptm = gmtime_r(&rawtime, &gbuf);
#else
    ptm = gmtime(&rawtime);
#endif
    // Request that mktime() looksup dst in timezone database
    ptm->tm_isdst = -1;
    gmt           = mktime(ptm);

    return (int)difftime(rawtime, gmt);
}

///////////////////////////////////////////////////////////////////////////////
// tz_offset_second
//
// return difference in **seconds** of the tm_mday, tm_hour, tm_min, tm_sec
// members.
//
// https://stackoverflow.com/questions/32424125/c-code-to-get-local-time-offset-in-minutes-relative-to-utc
//

long
vscpdatetime::tz_offset_second(time_t t)
{
    struct tm local = *localtime(&t);
    struct tm utc   = *gmtime(&t);
    long diff =
      ((local.tm_hour - utc.tm_hour) * 60 + (local.tm_min - utc.tm_min)) * 60L +
      (local.tm_sec - utc.tm_sec);
    int delta_day = local.tm_mday - utc.tm_mday;
    // If |delta_day| > 1, then end-of-month wrap
    if ((delta_day == 1) || (delta_day < -1)) {
        diff += 24L * 60 * 60;
    } else if ((delta_day == -1) || (delta_day > 1)) {
        diff -= 24L * 60 * 60;
    }
    return diff;
}