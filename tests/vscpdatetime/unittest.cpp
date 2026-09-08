// unittest.cpp
//
// Unit tests for vscpdatetime class
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>

#include <vscpdatetime.h>
#include <vscp.h>
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
// Constructor tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, DefaultConstructor) 
{
    vscpdatetime dt;
    // Default constructor sets to current time, so we can't test exact values
    // Just verify it doesn't crash and produces valid time
    ASSERT_TRUE(dt.isValid());
}

TEST(VscpDateTime, ParameterizedConstructor) 
{
    vscpdatetime dt(2024, 6, 15, 14, 30, 45, 500);
    
    EXPECT_EQ(2024, dt.getYear());
    EXPECT_EQ(vscpdatetime::Jun, dt.getMonth());  // Month is 0-indexed internally (May = 4)
    EXPECT_EQ(15, dt.getDay());
    EXPECT_EQ(14, dt.getHour());
    EXPECT_EQ(30, dt.getMinute());
    EXPECT_EQ(45, dt.getSecond());
    EXPECT_EQ(500u, dt.getMilliSeconds());
}

TEST(VscpDateTime, StringConstructor) 
{
    vscpdatetime dt("2024-03-20T10:25:30");
    
    EXPECT_EQ(2024, dt.getYear());
    EXPECT_EQ(20, dt.getDay());
    EXPECT_EQ(10, dt.getHour());
    EXPECT_EQ(25, dt.getMinute());
    EXPECT_EQ(30, dt.getSecond());
}

TEST(VscpDateTime, VscpEventConstructor) 
{
    vscpEvent ev;
    memset(&ev, 0, sizeof(vscpEvent));
    ev.year = 2023;
    ev.month = 8;
    ev.day = 25;
    ev.hour = 16;
    ev.minute = 45;
    ev.second = 30;
    
    vscpdatetime dt(ev);
    
    EXPECT_EQ(2023, dt.getYear());
    EXPECT_EQ(25, dt.getDay());
    EXPECT_EQ(16, dt.getHour());
    EXPECT_EQ(45, dt.getMinute());
    EXPECT_EQ(30, dt.getSecond());
}

TEST(VscpDateTime, VscpEventExConstructor) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));
    ex.year = 2022;
    ex.month = 12;
    ex.day = 31;
    ex.hour = 23;
    ex.minute = 59;
    ex.second = 59;
    
    vscpdatetime dt(ex);
    
    EXPECT_EQ(2022, dt.getYear());
    EXPECT_EQ(31, dt.getDay());
    EXPECT_EQ(23, dt.getHour());
    EXPECT_EQ(59, dt.getMinute());
    EXPECT_EQ(59, dt.getSecond());
}

//-----------------------------------------------------------------------------
// Setter tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, SetDate) 
{
    vscpdatetime dt;
    
    ASSERT_TRUE(dt.setDate(2025, 7, 4));
    EXPECT_EQ(2025, dt.getYear());
    EXPECT_EQ(vscpdatetime::Jul, dt.getMonth());  // July = 6 (0-indexed)
    EXPECT_EQ(4, dt.getDay());
}

TEST(VscpDateTime, SetDateInvalid) 
{
    vscpdatetime dt;
    
    // Invalid month (0)
    EXPECT_FALSE(dt.setDate(2025, 0, 4));
    
    // Invalid month (13)
    EXPECT_FALSE(dt.setDate(2025, 13, 4));
    
    // Invalid day (0)
    EXPECT_FALSE(dt.setDate(2025, 7, 0));
    
    // Invalid day (32)
    EXPECT_FALSE(dt.setDate(2025, 7, 32));
}

TEST(VscpDateTime, SetTime) 
{
    vscpdatetime dt;
    
    ASSERT_TRUE(dt.setTime(12, 30, 45, 123));
    EXPECT_EQ(12, dt.getHour());
    EXPECT_EQ(30, dt.getMinute());
    EXPECT_EQ(45, dt.getSecond());
    EXPECT_EQ(123u, dt.getMilliSeconds());
}

TEST(VscpDateTime, SetTimeInvalid) 
{
    vscpdatetime dt;
    
    // Invalid hour
    EXPECT_FALSE(dt.setTime(24, 30, 45));
    
    // Invalid minute
    EXPECT_FALSE(dt.setTime(12, 60, 45));
    
    // Invalid second
    EXPECT_FALSE(dt.setTime(12, 30, 60));
}

TEST(VscpDateTime, SetYear) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setYear(2030));
    EXPECT_EQ(2030, dt.getYear());
}

TEST(VscpDateTime, SetMonth) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setMonth(3));  // March
    EXPECT_EQ(vscpdatetime::Mar, dt.getMonth());
    
    // Invalid month
    EXPECT_FALSE(dt.setMonth(0));
    EXPECT_FALSE(dt.setMonth(13));
}

TEST(VscpDateTime, SetDay) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setDay(15));
    EXPECT_EQ(15, dt.getDay());
    
    // Invalid day
    EXPECT_FALSE(dt.setDay(0));
    EXPECT_FALSE(dt.setDay(32));
}

TEST(VscpDateTime, SetHour) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setHour(23));
    EXPECT_EQ(23, dt.getHour());
    
    // Hour 0 should be valid
    ASSERT_TRUE(dt.setHour(0));
    EXPECT_EQ(0, dt.getHour());
    
    // Invalid hour
    EXPECT_FALSE(dt.setHour(24));
}

TEST(VscpDateTime, SetMinute) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setMinute(59));
    EXPECT_EQ(59, dt.getMinute());
    
    // Minute 0 should be valid
    ASSERT_TRUE(dt.setMinute(0));
    EXPECT_EQ(0, dt.getMinute());
    
    // Invalid minute
    EXPECT_FALSE(dt.setMinute(60));
}

TEST(VscpDateTime, SetSecond) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setSecond(59));
    EXPECT_EQ(59, dt.getSecond());
    
    // Second 0 should be valid
    ASSERT_TRUE(dt.setSecond(0));
    EXPECT_EQ(0, dt.getSecond());
    
    // Invalid second
    EXPECT_FALSE(dt.setSecond(60));
}

TEST(VscpDateTime, SetMilliSecond) 
{
    vscpdatetime dt;
    ASSERT_TRUE(dt.setMilliSecond(999));
    EXPECT_EQ(999u, dt.getMilliSeconds());
}

//-----------------------------------------------------------------------------
// ISO String tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, SetISODate) 
{
    vscpdatetime dt;
    
    ASSERT_TRUE(dt.setISODate("2024-06-15"));
    EXPECT_EQ(2024, dt.getYear());
    // Month is stored 0-indexed internally (June = 5)
    EXPECT_EQ(vscpdatetime::Jun, dt.getMonth());
    EXPECT_EQ(15, dt.getDay());
}

TEST(VscpDateTime, SetISOTime) 
{
    vscpdatetime dt;
    
    ASSERT_TRUE(dt.setISOTime("14:30:45"));
    EXPECT_EQ(14, dt.getHour());
    EXPECT_EQ(30, dt.getMinute());
    EXPECT_EQ(45, dt.getSecond());
}

TEST(VscpDateTime, SetFromISOString) 
{
    vscpdatetime dt;
    
    ASSERT_TRUE(dt.set("2024-06-15T14:30:45"));
    EXPECT_EQ(2024, dt.getYear());
    EXPECT_EQ(15, dt.getDay());
    EXPECT_EQ(14, dt.getHour());
    EXPECT_EQ(30, dt.getMinute());
    EXPECT_EQ(45, dt.getSecond());
}

TEST(VscpDateTime, SetFromConstChar) 
{
    vscpdatetime dt;
    const char* dateStr = "2024-06-15T14:30:45";
    
    ASSERT_TRUE(dt.set(dateStr));
    EXPECT_EQ(2024, dt.getYear());
    
    // NULL pointer should fail
    EXPECT_FALSE(dt.set(static_cast<const char*>(nullptr)));
}

//-----------------------------------------------------------------------------
// Getter tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, GetISODateTime) 
{
    vscpdatetime dt(2024, 6, 15, 14, 30, 45);
    
    std::string isoStr = dt.getISODateTime(true);
    EXPECT_TRUE(isoStr.find("2024") != std::string::npos);
    EXPECT_TRUE(isoStr.find("T") != std::string::npos);
}

TEST(VscpDateTime, GetISODate) 
{
    vscpdatetime dt(2024, 6, 15, 14, 30, 45);
    
    std::string dateStr = dt.getISODate();
    EXPECT_TRUE(dateStr.find("2024") != std::string::npos);
    EXPECT_TRUE(dateStr.find("-") != std::string::npos);
}

TEST(VscpDateTime, GetISOTime) 
{
    vscpdatetime dt(2024, 6, 15, 14, 30, 45);
    
    std::string timeStr = dt.getISOTime();
    EXPECT_TRUE(timeStr.find(":") != std::string::npos);
}

//-----------------------------------------------------------------------------
// Julian date tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, JulianDateConversion) 
{
    vscpdatetime dt(2024, 1, 1, 0, 0, 0);
    
    long jd = dt.getJulian();
    EXPECT_GT(jd, 0);  // Julian date should be positive
    
    // Set from Julian and verify it's consistent
    vscpdatetime dt2;
    dt2.setFromJulian(jd);
    EXPECT_EQ(dt.getYear(), dt2.getYear());
    EXPECT_EQ(dt.getMonth(), dt2.getMonth());
    EXPECT_EQ(dt.getDay(), dt2.getDay());
}

TEST(VscpDateTime, JulianDateKnownValue) 
{
    // January 1, 2000 has a known Julian Day Number
    vscpdatetime dt(2000, 1, 1, 12, 0, 0);
    long jd = dt.getJulian();
    
    // JDN for Jan 1, 2000 should be around 2451545
    EXPECT_NEAR(jd, 2451545, 2);
}

//-----------------------------------------------------------------------------
// Day of year tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, GetDayOfYear) 
{
    // January 1st should be day 1
    vscpdatetime dt1(2024, 1, 1, 0, 0, 0);
    EXPECT_EQ(1, dt1.getDayOfYear());
    
    // February 1st should be day 32
    vscpdatetime dt2(2024, 2, 1, 0, 0, 0);
    EXPECT_EQ(32, dt2.getDayOfYear());
    
    // December 31st of a leap year (2024) should be day 366
    vscpdatetime dt3(2024, 12, 31, 0, 0, 0);
    EXPECT_EQ(366, dt3.getDayOfYear());
    
    // December 31st of a non-leap year should be day 365
    vscpdatetime dt4(2023, 12, 31, 0, 0, 0);
    EXPECT_EQ(365, dt4.getDayOfYear());
}

//-----------------------------------------------------------------------------
// Leap year tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, IsLeapYear) 
{
    // 2024 is a leap year
    vscpdatetime dt1(2024, 1, 1, 0, 0, 0);
    EXPECT_TRUE(dt1.isLeapYear());
    
    // 2023 is not a leap year
    vscpdatetime dt2(2023, 1, 1, 0, 0, 0);
    EXPECT_FALSE(dt2.isLeapYear());
    
    // 2000 is a leap year (divisible by 400)
    vscpdatetime dt3(2000, 1, 1, 0, 0, 0);
    EXPECT_TRUE(dt3.isLeapYear());
    
    // 1900 is not a leap year (divisible by 100 but not by 400)
    vscpdatetime dt4(1900, 1, 1, 0, 0, 0);
    EXPECT_FALSE(dt4.isLeapYear());
}

//-----------------------------------------------------------------------------
// Week tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, GetWeekNumber) 
{
    vscpdatetime dt(2024, 1, 1, 0, 0, 0);
    int weekNum = dt.getWeekNumber();
    
    // First week should be week 1 or 0 depending on the year
    EXPECT_GE(weekNum, 0);
    EXPECT_LE(weekNum, 53);
}

TEST(VscpDateTime, GetWeekDay) 
{
    // January 1, 2024 was a Monday
    vscpdatetime dt(2024, 1, 1, 0, 0, 0);
    vscpdatetime::weekDay wd = dt.getWeekDay();
    
    EXPECT_EQ(vscpdatetime::Mon, wd);
}

//-----------------------------------------------------------------------------
// Weekday name tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, GetWeekDayNameFull) 
{
    EXPECT_EQ("Sunday", vscpdatetime::getWeekDayName(vscpdatetime::Sun, vscpdatetime::name_Full));
    EXPECT_EQ("Monday", vscpdatetime::getWeekDayName(vscpdatetime::Mon, vscpdatetime::name_Full));
    EXPECT_EQ("Friday", vscpdatetime::getWeekDayName(vscpdatetime::Fri, vscpdatetime::name_Full));
    EXPECT_EQ("Saturday", vscpdatetime::getWeekDayName(vscpdatetime::Sat, vscpdatetime::name_Full));
}

TEST(VscpDateTime, GetWeekDayNameAbbr) 
{
    EXPECT_EQ("Sun", vscpdatetime::getWeekDayName(vscpdatetime::Sun, vscpdatetime::name_Abbr));
    EXPECT_EQ("Mon", vscpdatetime::getWeekDayName(vscpdatetime::Mon, vscpdatetime::name_Abbr));
    EXPECT_EQ("Fri", vscpdatetime::getWeekDayName(vscpdatetime::Fri, vscpdatetime::name_Abbr));
    EXPECT_EQ("Sat", vscpdatetime::getWeekDayName(vscpdatetime::Sat, vscpdatetime::name_Abbr));
}

//-----------------------------------------------------------------------------
// Month name tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, GetMonthNameFull) 
{
    EXPECT_EQ("January", vscpdatetime::getMonthName(vscpdatetime::Jan, vscpdatetime::name_Full));
    EXPECT_EQ("February", vscpdatetime::getMonthName(vscpdatetime::Feb, vscpdatetime::name_Full));
    EXPECT_EQ("June", vscpdatetime::getMonthName(vscpdatetime::Jun, vscpdatetime::name_Full));
    EXPECT_EQ("December", vscpdatetime::getMonthName(vscpdatetime::Dec, vscpdatetime::name_Full));
}

TEST(VscpDateTime, GetMonthNameAbbr) 
{
    EXPECT_EQ("Jan", vscpdatetime::getMonthName(vscpdatetime::Jan, vscpdatetime::name_Abbr));
    EXPECT_EQ("Feb", vscpdatetime::getMonthName(vscpdatetime::Feb, vscpdatetime::name_Abbr));
    EXPECT_EQ("Jun", vscpdatetime::getMonthName(vscpdatetime::Jun, vscpdatetime::name_Abbr));
    EXPECT_EQ("Dec", vscpdatetime::getMonthName(vscpdatetime::Dec, vscpdatetime::name_Abbr));
}

//-----------------------------------------------------------------------------
// Comparison tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, IsEqualTo) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 45);
    vscpdatetime dt2(2024, 6, 15, 14, 30, 45);
    
    EXPECT_TRUE(dt1.isEqualTo(dt2));
}

TEST(VscpDateTime, IsEarlierThan) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 45);
    vscpdatetime dt2(2024, 6, 15, 14, 30, 46);
    
    EXPECT_TRUE(dt1.isEarlierThan(dt2));
    EXPECT_FALSE(dt2.isEarlierThan(dt1));
}

TEST(VscpDateTime, IsLaterThan) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 46);
    vscpdatetime dt2(2024, 6, 15, 14, 30, 45);
    
    EXPECT_TRUE(dt1.isLaterThan(dt2));
    EXPECT_FALSE(dt2.isLaterThan(dt1));
}

TEST(VscpDateTime, IsBetween) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 40);
    vscpdatetime dt2(2024, 6, 15, 14, 30, 45);
    vscpdatetime dt3(2024, 6, 15, 14, 30, 50);
    
    EXPECT_TRUE(dt2.isBetween(dt1, dt3));
    EXPECT_FALSE(dt1.isBetween(dt2, dt3));
}

TEST(VscpDateTime, IsStrictlyBetween) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 40);
    vscpdatetime dt2(2024, 6, 15, 14, 30, 45);
    vscpdatetime dt3(2024, 6, 15, 14, 30, 50);
    
    EXPECT_TRUE(dt2.isStrictlyBetween(dt1, dt3));
    
    // Edge case - equal to boundary should not be strictly between
    vscpdatetime dt4(2024, 6, 15, 14, 30, 40);
    EXPECT_FALSE(dt4.isStrictlyBetween(dt1, dt3));
}

TEST(VscpDateTime, IsSameDate) 
{
    vscpdatetime dt1(2024, 6, 15, 10, 0, 0);
    vscpdatetime dt2(2024, 6, 15, 20, 30, 45);
    vscpdatetime dt3(2024, 6, 16, 10, 0, 0);
    
    EXPECT_TRUE(dt1.isSameDate(dt2));
    EXPECT_FALSE(dt1.isSameDate(dt3));
}

TEST(VscpDateTime, IsSameTime) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 45);
    vscpdatetime dt2(2025, 7, 20, 14, 30, 45);
    vscpdatetime dt3(2024, 6, 15, 14, 30, 46);
    
    EXPECT_TRUE(dt1.isSameTime(dt2));
    EXPECT_FALSE(dt1.isSameTime(dt3));
}

//-----------------------------------------------------------------------------
// Diff seconds test
//-----------------------------------------------------------------------------

TEST(VscpDateTime, DiffSeconds) 
{
    vscpdatetime dt1(2024, 6, 15, 14, 30, 0);
    vscpdatetime dt2(2024, 6, 15, 14, 31, 0);
    
    double diff = vscpdatetime::diffSeconds(dt2, dt1);
    EXPECT_EQ(60.0, diff);
}

//-----------------------------------------------------------------------------
// Operator tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, AdditionOperator) 
{
    vscpdatetime dt(2024, 6, 15, 0, 0, 0);
    vscpdatetime result = dt + 1;  // Add one day
    
    EXPECT_EQ(16, result.getDay());
}

TEST(VscpDateTime, SubtractionOperator) 
{
    vscpdatetime dt(2024, 6, 15, 0, 0, 0);
    vscpdatetime result = dt - 1;  // Subtract one day
    
    EXPECT_EQ(14, result.getDay());
}

TEST(VscpDateTime, AdditionAssignmentOperator) 
{
    vscpdatetime dt(2024, 6, 15, 0, 0, 0);
    dt += 5;  // Add 5 days
    
    EXPECT_EQ(20, dt.getDay());
}

TEST(VscpDateTime, SubtractionAssignmentOperator) 
{
    vscpdatetime dt(2024, 6, 15, 0, 0, 0);
    dt -= 5;  // Subtract 5 days
    
    EXPECT_EQ(10, dt.getDay());
}

TEST(VscpDateTime, IncrementOperator) 
{
    vscpdatetime dt(2024, 6, 15, 0, 0, 0);
    
    // Prefix increment
    ++dt;
    EXPECT_EQ(16, dt.getDay());
    
    // Postfix increment
    vscpdatetime old = dt++;
    EXPECT_EQ(16, old.getDay());
    EXPECT_EQ(17, dt.getDay());
}

TEST(VscpDateTime, DecrementOperator) 
{
    vscpdatetime dt(2024, 6, 15, 0, 0, 0);
    
    // Prefix decrement
    --dt;
    EXPECT_EQ(14, dt.getDay());
    
    // Postfix decrement
    vscpdatetime old = dt--;
    EXPECT_EQ(14, old.getDay());
    EXPECT_EQ(13, dt.getDay());
}

TEST(VscpDateTime, DifferenceBetweenDates) 
{
    vscpdatetime dt1(2024, 6, 10, 0, 0, 0);
    vscpdatetime dt2(2024, 6, 15, 0, 0, 0);
    
    long diff = dt2 - dt1;
    EXPECT_EQ(5, diff);
}

//-----------------------------------------------------------------------------
// toSysTime tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, ToSysTime) 
{
    vscpdatetime dt(2024, 6, 15, 12, 0, 0);
    time_t t = dt.toSysTime();
    
    // Verify it's a reasonable value (after year 2000)
    EXPECT_GT(t, 946684800);  // 2000-01-01 00:00:00 UTC
}

//-----------------------------------------------------------------------------
// Static factory tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, Now) 
{
    vscpdatetime now = vscpdatetime::Now();
    
    // Just verify it doesn't crash and produces a valid year
    EXPECT_GE(now.getYear(), 2024);
}

TEST(VscpDateTime, UTCNow) 
{
    vscpdatetime utcNow = vscpdatetime::UTCNow();
    
    // Just verify it doesn't crash and produces a valid year
    EXPECT_GE(utcNow.getYear(), 2024);
}

TEST(VscpDateTime, ZeroTime) 
{
    vscpdatetime zero = vscpdatetime::zeroTime();
    
    // Zero timestamp means Unix epoch (1970-01-01 00:00:00 UTC)
    EXPECT_EQ(0, zero.getHour());
    EXPECT_EQ(0, zero.getMinute());
    EXPECT_EQ(0, zero.getSecond());
}

TEST(VscpDateTime, ZeroDate) 
{
    vscpdatetime zero = vscpdatetime::zeroDate();
    
    // Zero timestamp means Unix epoch (1970-01-01 00:00:00 UTC)
    EXPECT_EQ(1970, zero.getYear());
    EXPECT_EQ(vscpdatetime::Jan, zero.getMonth());
    EXPECT_EQ(1, zero.getDay());
}

TEST(VscpDateTime, DateTimeZero) 
{
    vscpdatetime zero = vscpdatetime::dateTimeZero();
    
    // Zero timestamp means Unix epoch (1970-01-01 00:00:00 UTC)
    EXPECT_EQ(1970, zero.getYear());
    EXPECT_EQ(0, zero.getHour());
}

//-----------------------------------------------------------------------------
// Validity tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, IsValidTrue) 
{
    vscpdatetime dt(2024, 6, 15, 14, 30, 45);
    EXPECT_TRUE(dt.isValid());
}

//-----------------------------------------------------------------------------
// Set from tm struct tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, SetFromTmStruct) 
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = 124;  // Years since 1900
    tm.tm_mon = 5;     // June (0-indexed)
    tm.tm_mday = 15;
    tm.tm_hour = 14;
    tm.tm_min = 30;
    tm.tm_sec = 45;
    
    vscpdatetime dt(tm);
    
    EXPECT_EQ(2024, dt.getYear());
    EXPECT_EQ(15, dt.getDay());
    EXPECT_EQ(14, dt.getHour());
    EXPECT_EQ(30, dt.getMinute());
    EXPECT_EQ(45, dt.getSecond());
}

//-----------------------------------------------------------------------------
// Constants tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, Constants) 
{
    // Use values directly to avoid ODR-use issues with static const members
    int months = vscpdatetime::MONTHS_IN_YEAR;
    int secMin = vscpdatetime::SEC_PER_MIN;
    int minHour = vscpdatetime::MIN_PER_HOUR;
    long secHour = vscpdatetime::SECONDS_PER_HOUR;
    long secDay = vscpdatetime::SECONDS_PER_DAY;
    int daysWeek = vscpdatetime::DAYS_PER_WEEK;
    long msDay = vscpdatetime::MILLISECONDS_PER_DAY;
    long epochJdn = vscpdatetime::EPOCH_JDN;
    
    EXPECT_EQ(12, months);
    EXPECT_EQ(60, secMin);
    EXPECT_EQ(60, minHour);
    EXPECT_EQ(3600, secHour);
    EXPECT_EQ(86400, secDay);
    EXPECT_EQ(7, daysWeek);
    EXPECT_EQ(86400000, msDay);
    EXPECT_EQ(2440587, epochJdn);
}

//-----------------------------------------------------------------------------
// setNowIfTimeStampRelative tests
//-----------------------------------------------------------------------------

TEST(VscpDateTime, SetNowIfTimeStampRelative_ZeroTimestamp) 
{
    vscpdatetime dt;
    dt.setTimestamp(0);
    
    EXPECT_TRUE(dt.setNowIfTimeStampRelative());
    
    // Should now be a valid current date (year >= 2024)
    EXPECT_GE(dt.getYear(), 2024);
}

TEST(VscpDateTime, SetNowIfTimeStampRelative_SubSecondPreserved) 
{
    vscpdatetime dt;
    int64_t nanoseconds = 123456789;  // Sub-second portion to preserve
    dt.setTimestamp(nanoseconds);
    
    EXPECT_TRUE(dt.setNowIfTimeStampRelative());
    
    // Should preserve the sub-second nanosecond portion
    EXPECT_EQ(nanoseconds, dt.getNanoSeconds());
    
    // Should now be a valid current date
    EXPECT_GE(dt.getYear(), 2024);
}

TEST(VscpDateTime, SetNowIfTimeStampRelative_ValidTimestampUnchanged) 
{
    // Set to a known valid timestamp (2024-06-15 14:30:45 UTC)
    vscpdatetime dt(2024, 6, 15, 14, 30, 45);
    int64_t originalTimestamp = dt.getTimestamp();
    
    // Should return false and not modify the timestamp
    EXPECT_FALSE(dt.setNowIfTimeStampRelative());
    EXPECT_EQ(originalTimestamp, dt.getTimestamp());
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
