///////////////////////////////////////////////////////////////////////////////
// datetime.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, the VSCP project
// <info@vscp.org>
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

#ifdef WIN32
#include <pch.h>
#endif

#include <string>
#include <chrono>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vscpdatetime.h"
#include <vscp.h>
#include <vscphelper.h>

///////////////////////////////////////////////////////////////////////////////
// Helper: getTimeStruct
//

struct tm 
vscpdatetime::getTimeStruct(void) const
{
    struct tm result;
    time_t secs = static_cast<time_t>(m_timestamp / NS_PER_SECOND);
    
#ifdef WIN32
    gmtime_s(&result, &secs);
#else
    gmtime_r(&secs, &result);
#endif
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Helper: setFromTimeStruct
//

void 
vscpdatetime::setFromTimeStruct(const struct tm& tm, uint32_t nanoseconds)
{
    struct tm temp = tm;
    temp.tm_isdst = 0;
    
    // Use timegm (or _mkgmtime on Windows) to convert UTC tm to time_t
#ifdef WIN32
    time_t secs = _mkgmtime(&temp);
#else
    time_t secs = timegm(&temp);
#endif
    
    m_timestamp = static_cast<int64_t>(secs) * NS_PER_SECOND + nanoseconds;
}

///////////////////////////////////////////////////////////////////////////////
// ctor
//

vscpdatetime::vscpdatetime(void)
{
    setUTCNow();
}

vscpdatetime::vscpdatetime(const std::string &dt)
{
    m_timestamp = 0;
    set(dt);
}

vscpdatetime::vscpdatetime(vscpEvent &ev)
{
    m_timestamp = 0;
    set(ev);
}

vscpdatetime::vscpdatetime(vscpEventEx &ex)
{
    m_timestamp = 0;
    set(ex);
}

vscpdatetime::vscpdatetime(uint16_t year,
                           uint8_t month,
                           uint8_t day,
                           uint8_t hour,
                           uint8_t minute,
                           uint8_t second,
                           uint32_t millisecond)
{
    m_timestamp = 0;
    set(year, month, day, hour, minute, second, millisecond);
}

///////////////////////////////////////////////////////////////////////////////
// dtor
//

vscpdatetime::~vscpdatetime(void)
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Getters
//

uint16_t
vscpdatetime::getYear(void) const
{
    struct tm tm = getTimeStruct();
    return tm.tm_year + 1900;
}

vscpdatetime::month
vscpdatetime::getMonth(void) const
{
    struct tm tm = getTimeStruct();
    return static_cast<month>(tm.tm_mon);
}

uint8_t
vscpdatetime::getDay(void) const
{
    struct tm tm = getTimeStruct();
    return tm.tm_mday;
}

uint8_t
vscpdatetime::getHour(void) const
{
    struct tm tm = getTimeStruct();
    return tm.tm_hour;
}

uint8_t
vscpdatetime::getMinute(void) const
{
    struct tm tm = getTimeStruct();
    return tm.tm_min;
}

uint8_t
vscpdatetime::getSecond(void) const
{
    struct tm tm = getTimeStruct();
    return tm.tm_sec;
}

uint32_t
vscpdatetime::getMilliSeconds(void) const
{
    return static_cast<uint32_t>((m_timestamp % NS_PER_SECOND) / NS_PER_MILLISECOND);
}

uint32_t
vscpdatetime::getMicroSeconds(void) const
{
    return static_cast<uint32_t>((m_timestamp % NS_PER_SECOND) / NS_PER_MICROSECOND);
}

uint32_t
vscpdatetime::getNanoSeconds(void) const
{
    return static_cast<uint32_t>(m_timestamp % NS_PER_SECOND);
}

///////////////////////////////////////////////////////////////////////////////
// setDate
//

bool
vscpdatetime::setDate(uint16_t year, uint8_t month, uint8_t day)
{
    if (!month || (month > 12)) return false;
    if (!day || (day > 31)) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;  // Convert 1-12 to 0-11
    tm.tm_mday = day;
    
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setISODate
//

bool
vscpdatetime::setISODate(const std::string &strDate)
{
    size_t pos;
    std::string isodt = strDate;

    try {
        uint16_t year = std::stoi(isodt, &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        uint8_t month = std::stoi(isodt, &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        uint8_t day = std::stoi(isodt, &pos);

        return setDate(year, month, day);
    } catch (...) {
        return false;
    }
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
    if (hour > 23) return false;
    if (minute > 59) return false;
    if (second > 59) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    
    setFromTimeStruct(tm, millisecond * NS_PER_MILLISECOND);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setISOTime
//

bool
vscpdatetime::setISOTime(const std::string &strTime)
{
    size_t pos;
    std::string isodt = strTime;

    try {
        uint8_t hour = std::stoi(isodt, &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        uint8_t minute = std::stoi(isodt, &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        uint8_t second = std::stoi(isodt, &pos);

        return setTime(hour, minute, second, 0);
    } catch (...) {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// setYear
//

bool
vscpdatetime::setYear(uint16_t year)
{
    struct tm tm = getTimeStruct();
    tm.tm_year = year - 1900;
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setMonth
//

bool
vscpdatetime::setMonth(uint8_t month)
{
    if (!month || (month > 12)) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_mon = month - 1;  // Convert 1-12 to 0-11
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setDay
//

bool
vscpdatetime::setDay(uint8_t day)
{
    if (!day || (day > 31)) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_mday = day;
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setHour
//

bool
vscpdatetime::setHour(uint8_t hour)
{
    if (hour > 23) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_hour = hour;
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setMinute
//

bool
vscpdatetime::setMinute(uint8_t minute)
{
    if (minute > 59) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_min = minute;
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setSecond
//

bool
vscpdatetime::setSecond(uint8_t second)
{
    if (second > 59) return false;
    
    struct tm tm = getTimeStruct();
    tm.tm_sec = second;
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setMilliSecond
//

bool
vscpdatetime::setMilliSecond(uint32_t millisecond)
{
    // Clear sub-second portion and add new milliseconds
    uint64_t secs = m_timestamp / NS_PER_SECOND;
    m_timestamp = secs * NS_PER_SECOND + millisecond * NS_PER_MILLISECOND;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// SetMillisecond (alternate spelling)
//

bool
vscpdatetime::SetMillisecond(uint32_t ms)
{
    return setMilliSecond(ms);
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
    if (!month || (month > 12)) return false;
    if (!day || (day > 31)) return false;
    if (hour > 23) return false;
    if (minute > 59) return false;
    if (second > 59) return false;

    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;  // Convert 1-12 to 0-11
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = 0;

    setFromTimeStruct(tm, millisecond * NS_PER_MILLISECOND);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// set (from string)
//

bool
vscpdatetime::set(std::string dt)
{
    size_t pos;
    std::string isodt = dt;

    try {
        uint16_t year = std::stoi(isodt, &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        uint8_t month = std::stoi(isodt, &pos);
        pos++; // Move past '-'
        isodt = isodt.substr(pos);

        uint8_t day = std::stoi(isodt, &pos);
        pos++; // Move past 'T' or ' '
        isodt = isodt.substr(pos);

        uint8_t hour = std::stoi(isodt, &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        uint8_t minute = std::stoi(isodt, &pos);
        pos++; // Move past ':'
        isodt = isodt.substr(pos);

        uint8_t second = std::stoi(isodt, &pos);

        return set(year, month, day, hour, minute, second, 0);
    } catch (...) {
        return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
// set (from const char*)
//

bool
vscpdatetime::set(const char *pdt)
{
    if (NULL == pdt) return false;
    std::string str(pdt);
    return set(str);
}

///////////////////////////////////////////////////////////////////////////////
// set (from vscpEvent)
//

bool
vscpdatetime::set(vscpEvent &ev)
{
    return set(ev.year, ev.month, ev.day, ev.hour, ev.minute, ev.second, 0);
}

///////////////////////////////////////////////////////////////////////////////
// set (from vscpEventEx)
//

bool
vscpdatetime::set(vscpEventEx &ex)
{
    return set(ex.year, ex.month, ex.day, ex.hour, ex.minute, ex.second, 0);
}

///////////////////////////////////////////////////////////////////////////////
// set (from struct tm)
//

bool
vscpdatetime::set(const struct tm &tm)
{
    setFromTimeStruct(tm, 0);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setNow
//

void
vscpdatetime::setNow(void)
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    m_timestamp = static_cast<int64_t>(nanos.count());
}

///////////////////////////////////////////////////////////////////////////////
// setUTCNow
//

void
vscpdatetime::setUTCNow(void)
{
    // std::chrono::system_clock is always UTC
    setNow();
}

///////////////////////////////////////////////////////////////////////////////
// Now
//

vscpdatetime
vscpdatetime::Now(void)
{
    vscpdatetime dt;
    dt.setNow();
    return dt;
}

///////////////////////////////////////////////////////////////////////////////
// UTCNow
//

vscpdatetime
vscpdatetime::UTCNow(void)
{
    vscpdatetime dt;
    dt.setUTCNow();
    return dt;
}

///////////////////////////////////////////////////////////////////////////////
// zeroTime
//

vscpdatetime
vscpdatetime::zeroTime(void)
{
    vscpdatetime dt;
    dt.m_timestamp = 0;
    return dt;
}

///////////////////////////////////////////////////////////////////////////////
// zeroDate
//

vscpdatetime
vscpdatetime::zeroDate(void)
{
    vscpdatetime dt;
    dt.m_timestamp = 0;
    return dt;
}

///////////////////////////////////////////////////////////////////////////////
// dateTimeZero
//

vscpdatetime
vscpdatetime::dateTimeZero(void)
{
    vscpdatetime dt;
    dt.m_timestamp = 0;
    return dt;
}

///////////////////////////////////////////////////////////////////////////////
// getISODateTime
//

std::string
vscpdatetime::getISODateTime(bool bSeparator) const
{
    char buf[32];
    struct tm tm = getTimeStruct();
    
    if (bSeparator) {
        strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    } else {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%SZ", &tm);
    }
    
    return std::string(buf);
}

///////////////////////////////////////////////////////////////////////////////
// getISODate
//

std::string
vscpdatetime::getISODate(void)
{
    char buf[32];
    struct tm tm = getTimeStruct();
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

///////////////////////////////////////////////////////////////////////////////
// getISOTime
//

std::string
vscpdatetime::getISOTime(void)
{
    char buf[32];
    struct tm tm = getTimeStruct();
    strftime(buf, sizeof(buf), "%H:%M:%SZ", &tm);
    return std::string(buf);
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
    
    uint8_t day   = (uint8_t)((int)b - d - (long)(30.6001 * e));
    uint8_t month = (uint8_t)((int)(e < 13.5) ? e - 1 : e - 13);
    uint16_t year  = (uint16_t)((int)(month > 2) ? (c - 4716) : c - 4715);
    if (year <= 0) {
        year -= 1;
    }

    // Preserve time portion
    struct tm tm = getTimeStruct();
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    
    uint32_t nanos = getNanoSeconds();
    setFromTimeStruct(tm, nanos);
}

///////////////////////////////////////////////////////////////////////////////
// YmdToJd
//
// Copyright (C) 2000, 2002
// Todd T. Knarr <tknarr@silverglass.org>
//

static long
ymdToJd(const int iYear, const int iMonth, const int iDay)
{
    long jul_day;

    int a, b;
    int year = iYear, month = iMonth, day = iDay;
    double year_corr;

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

    return jul_day;
}

///////////////////////////////////////////////////////////////////////////////
// getJulian
//

long
vscpdatetime::getJulian(void)
{
    struct tm tm = getTimeStruct();
    // ymdToJd expects 1-based month (1-12)
    return ymdToJd(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

///////////////////////////////////////////////////////////////////////////////
// getDayOfYear
//

int
vscpdatetime::getDayOfYear(void) const
{
    struct tm tm = getTimeStruct();
    return tm.tm_yday + 1;  // tm_yday is 0-based, we want 1-based
}

///////////////////////////////////////////////////////////////////////////////
// toSysTime
//

time_t
vscpdatetime::toSysTime(void) const
{
    return static_cast<time_t>(m_timestamp / NS_PER_SECOND);
}

///////////////////////////////////////////////////////////////////////////////
// isValid
//

bool
vscpdatetime::isValid(void)
{
    struct tm tm = getTimeStruct();
    
    int days_in_month_normal[]   = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };
    int days_in_month_leapyear[] = { 31, 29, 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31 };

    if (tm.tm_mon > 11) return false;
    if (tm.tm_mday == 0) return false;

    if (isLeapYear()) {
        if (tm.tm_mday > days_in_month_leapyear[tm.tm_mon]) return false;
    } else {
        if (tm.tm_mday > days_in_month_normal[tm.tm_mon]) return false;
    }

    if (tm.tm_hour > 23) return false;
    if (tm.tm_min > 59) return false;
    if (tm.tm_sec > 59) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// isLeapYear
//

bool
vscpdatetime::isLeapYear(void)
{
    struct tm tm = getTimeStruct();
    int year = tm.tm_year + 1900;
    
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

///////////////////////////////////////////////////////////////////////////////
// getWeekNumber
//

int
vscpdatetime::getWeekNumber(void) const
{
    struct tm tm = getTimeStruct();
    constexpr int DAYS_PER_WEEK = 7;
    
    const int wday  = tm.tm_wday;
    const int delta = wday ? wday - 1 : DAYS_PER_WEEK - 1;
    return (tm.tm_yday + DAYS_PER_WEEK - delta) / DAYS_PER_WEEK;
}

///////////////////////////////////////////////////////////////////////////////
// getWeekDay
//

vscpdatetime::weekDay
vscpdatetime::getWeekDay(void) const
{
    struct tm tm = getTimeStruct();
    return static_cast<weekDay>(tm.tm_wday);
}

///////////////////////////////////////////////////////////////////////////////
// getWeekDayName
//

std::string
vscpdatetime::getWeekDayName(weekDay weekday, nameFlags flags)
{
    std::string strday;

    if (name_Full == flags) {
        switch (weekday) {
            case Sun: strday = "Sunday"; break;
            case Mon: strday = "Monday"; break;
            case Tue: strday = "Tuesday"; break;
            case Wed: strday = "Wednesday"; break;
            case Thu: strday = "Thursday"; break;
            case Fri: strday = "Friday"; break;
            case Sat: strday = "Saturday"; break;
            default: strday = "---"; break;
        }
    } else {
        switch (weekday) {
            case Sun: strday = "Sun"; break;
            case Mon: strday = "Mon"; break;
            case Tue: strday = "Tue"; break;
            case Wed: strday = "Wed"; break;
            case Thu: strday = "Thu"; break;
            case Fri: strday = "Fri"; break;
            case Sat: strday = "Sat"; break;
            default: strday = "---"; break;
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
            case Jan: strmonth = "January"; break;
            case Feb: strmonth = "February"; break;
            case Mar: strmonth = "March"; break;
            case Apr: strmonth = "April"; break;
            case May: strmonth = "May"; break;
            case Jun: strmonth = "June"; break;
            case Jul: strmonth = "July"; break;
            case Aug: strmonth = "August"; break;
            case Sep: strmonth = "September"; break;
            case Oct: strmonth = "October"; break;
            case Nov: strmonth = "November"; break;
            case Dec: strmonth = "December"; break;
            default: strmonth = "---"; break;
        }
    } else {
        switch (month) {
            case Jan: strmonth = "Jan"; break;
            case Feb: strmonth = "Feb"; break;
            case Mar: strmonth = "Mar"; break;
            case Apr: strmonth = "Apr"; break;
            case May: strmonth = "May"; break;
            case Jun: strmonth = "Jun"; break;
            case Jul: strmonth = "Jul"; break;
            case Aug: strmonth = "Aug"; break;
            case Sep: strmonth = "Sep"; break;
            case Oct: strmonth = "Oct"; break;
            case Nov: strmonth = "Nov"; break;
            case Dec: strmonth = "Dec"; break;
            default: strmonth = "---"; break;
        }
    }

    return strmonth;
}

///////////////////////////////////////////////////////////////////////////////
// tzOffset2LocalTime
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
    ptm->tm_isdst = -1;
    gmt = mktime(ptm);

    return (int)difftime(rawtime, gmt);
}

///////////////////////////////////////////////////////////////////////////////
// tz_offset_second
//

long
vscpdatetime::tz_offset_second(time_t t)
{
    struct tm local;
    struct tm utc;

#ifdef _WIN32
    localtime_s(&local, &t);
    gmtime_s(&utc, &t);
#else    
    struct tm tbuf;
    local = *localtime_r(&t, &tbuf);
    utc   = *gmtime_r(&t, &tbuf);
#endif    
    
    long diff =
      ((local.tm_hour - utc.tm_hour) * 60 + (local.tm_min - utc.tm_min)) * 60L +
      (local.tm_sec - utc.tm_sec);
    int delta_day = local.tm_mday - utc.tm_mday;
    
    if ((delta_day == 1) || (delta_day < -1)) {
        diff += 24L * 60 * 60;
    } else if ((delta_day == -1) || (delta_day > 1)) {
        diff -= 24L * 60 * 60;
    }
    return diff;
}