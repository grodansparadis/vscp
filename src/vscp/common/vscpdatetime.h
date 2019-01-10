///////////////////////////////////////////////////////////////////////////////
// datetime.h:
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

#if !defined(VSCP_DATETIME_H__INCLUDED_)
#define VSCP_DATETIME_H__INCLUDED_

#include <string>
#include <vscp.h>

class vscpdatetime
{

  public:
    // some trivial ones
    static const int MONTHS_IN_YEAR = 12;

    static const int SEC_PER_MIN = 60;

    static const int MIN_PER_HOUR = 60;

    static const long SECONDS_PER_HOUR = 3600;

    static const long SECONDS_PER_DAY = 86400l;

    static const int DAYS_PER_WEEK = 7;

    static const long MILLISECONDS_PER_DAY = 86400000l;

    // this is the integral part of JDN of the midnight of Jan 1, 1970
    // (i.e. JDN(Jan 1, 1970) = 2440587.5)
    static const long EPOCH_JDN = 2440587l;

    enum weekDay
    {
        Sun,
        Mon,
        Tue,
        Wed,
        Thu,
        Fri,
        Sat,
        Inv_WeekDay
    };

    enum month
    {
        Jan,
        Feb,
        Mar,
        Apr,
        May,
        Jun,
        Jul,
        Aug,
        Sep,
        Oct,
        Nov,
        Dec,
        Inv_Month
    };

    enum nameFlags
    {
        name_Full = 0x01,
        name_Abbr = 0x02
    };

    enum weekFlags
    {
        Default_First,
        Monday_First,
        Sunday_First
    };

  public:
    // structors

    /*!
    Default constructor

    Initialize the datetime object to UTC
*/
    vscpdatetime(void);

    /*!
        Constructor

        Initialize the datetime with the string value which
        should be on the form "YYYY:MM:DDTHH:MM:SS"

        @param dt Datetime to initialize the object with
    */
    vscpdatetime(const std::string &dt);

    /*!
        Constructor to set dattime object from VSCP event.

        @param ev VSCP event to set datetime from.
    */
    vscpdatetime(vscpEvent &ev);

    /*!
        Constructor to set dattime object from VSCP eventex.

        @param ex VSCP eventex to set datetime from.
    */
    vscpdatetime(vscpEventEx &ex);

    /*!
        Constructor

        Initialize the datetime according to parameters

        @param year Year to initialize the object with.
        @param month Month to initialize the object with.
        @param day Day to initialize the object with.
        @param hour Hour to initialize the object with.
        @param minute Minute to initialize the object with.
        @param second second to initialize the object with.
    */
    vscpdatetime(uint16_t year,
                 uint8_t month,
                 uint8_t day,
                 uint8_t hour,
                 uint8_t minute,
                 uint8_t second,
                 uint32_t millisecond = 0);

    /*!
        Initialize from julian date/time
    */
    vscpdatetime(const long ljd) { setFromJulian(ljd); };

    /*!
        Initialize from julian date/time
    */
    vscpdatetime(const double ljd) { setFromJulian((long)ljd); };

    /*!
        Set from Unix date/time struct
    */
    vscpdatetime(const struct tm &tm) { set(tm); };

    ~vscpdatetime();

    /*!
        Set date

        @param year Year to initialize the object with.
        @param month Month to initialize the object with.
        @param day Day to initialize the object with.
        @return True on success.
    */
    bool setDate(uint16_t year, uint8_t month, uint8_t day);

    /*!
        Set ISO date from string

        @param strDate String with date on ISO format (YYYY-MM-DD).
        @return True on success.
    */
    bool setISODate(const std::string &strDate);

    bool setISODate(const char *pDate)
    {
        std::string str(pDate);
        return setISOTime(str);
    };

    /*!
        Set time

        @param hour Hour to initialize the object with.
        @param minute Minute to initialize the object with.
        @param second second to initialize the object with.
    */
    bool setTime(uint8_t hour,
                 uint8_t minute,
                 uint8_t second,
                 uint32_t millisecond = 0);

    /*!
        Set ISO date from string

        @param strTime String with time on ISO format (HH:MM:SS).
        @return True on success.
    */
    bool setISOTime(const std::string &strTime);

    bool setISOTime(const char *pTime)
    {
        std::string str(pTime);
        return setISOTime(str);
    };

    /*!
        Set datetime

        @param year Year to set
        @param month Month to set
        @parm day day to set
        @param hour Hour to set
        @parm minute Minutes to set
        @param second Seconds to set
        @return True on success.
    */
    bool set(uint16_t year,
             uint8_t month,
             uint8_t day,
             uint8_t hour,
             uint8_t minute,
             uint8_t second,
             uint32_t millisecond = 0);
    /*!
        Set dattime from string on format YYYY:MM:DDTHH:MM:SS
    */
    bool set(std::string dt);

    /*!
        Set dattime from const char pointer to format YYYY:MM:DDTHH:MM:SS
    */
    bool set(const char *pdt);

    /*!
        Set datetime object from VSCP event.

        @param ev VSCP event to set datetime from.
    */
    bool set(vscpEvent &ev);

    /*!
        Set date/time from Unix date/time struct
    */
    bool set(const struct tm &tm);

    /*!
        Set date from Julian date

        @param ljd Julian date as long
    */
    void setFromJulian(const long ljd);

    /*!
        Set dattime object from VSCP eventex.

        @param ex VSCP eventex to set datetime from.
    */
    bool set(vscpEventEx &ex);

    /*! 
        Set date/time to current local time
    */
    void setNow(void);

    /*!
        Set date/time to UTC
    */
    void setUTCNow(void);

    /*!
        Return current local time.
    */
    static vscpdatetime Now(void);

    /*!
        Return current UTC time.
    */
    static vscpdatetime UTCNow(void);

    /*!
        Get current set year

        @return Year is returned.
    */
    uint16_t getYear(void) const { return m_year; };

    /*!
        Get current set month

        @return Month is returned.
    */
    month getMonth(void) const { return static_cast<month>(m_month); };

    /*!
        Get current set day

        @return Day is returned.
    */
    uint8_t getDay(void) const { return m_day; };

    /*!
        Get current set hours

        @return Hours is returned.
    */
    uint8_t getHour(void) const { return m_hour; };

    /*!
        Get current set minutes

        @return Minutes is returned.
    */
    uint8_t getMinute(void) const { return m_minute; };

    /*!
        Get current set seconds

        @return Seconds is returned.
    */
    uint8_t getSecond(void) const { return m_second; };

    /*!
        Set year

        @param year Year to set.
        @return True on success.
    */
    bool setYear(uint16_t year);

    /*!
        Set month

        @param month Month to set.
        @return True on success.
    */
    bool setMonth(uint8_t month);

    /*!
        Set day

        @param day Day to set.
        @return True on success.
    */
    bool setDay(uint8_t day);

    /*!
        Set hour

        @param hour Hour to set.
        @return True on success.
    */
    bool setHour(uint8_t hour);

    /*!
        Set minutes

        @param minute Minutes to set.
        @return True on success.
    */
    bool setMinute(uint8_t minute);

    /*!
        Set seconds

        @param seconds Seconds to set.
        @return True on success.
    */
    bool setSecond(uint8_t second);

    /*!
        Set milliseconds

        @param milliseconds Milliseconds to set.
        @return True on success.
    */
    bool setMilliSecond(uint32_t millisecond);

    /*!
        Set millisecond

        @param millisecond Milliseconds to set.
        @return True on success.
    */
    bool SetMillisecond(uint32_t ms)
    {
        m_millisecond = ms;
        return true;
    };

    /*!
        Reset time to midnight
    */
    static vscpdatetime zeroTime(void);

    /*!
        Reset date to first date
    */
    static vscpdatetime zeroDate(void);

    /*!
        Reset date and time to zero
    */
    static vscpdatetime dateTimeZero(void);

    /*!
        Get milliseconds

        @return Milliseconds as uin32_t
    */
    uint32_t getMilliSeconds(void) const { return m_millisecond; };

    /*!
        Get ISO datetime as standard string on format YYYY-MM-DDTHH:MM:SS

        @Return ISO datetime string.
    */
    std::string getISODateTime(bool bSeparator = true) const;

    /*!
        Get ISO date as standard string on format YYYY-MM-DD

        @Return ISO date string.
    */
    std::string getISODate(void);

    /*!
        Get ISO time as standard string on format HH:MM:SS

        @Return ISO time string.
    */
    std::string getISOTime(void);

    /*!
        Get julian day

        @return Julian day as long
    */
    long getJulian(void);

    /*!
        Get day of year

        @return day of year
    */
    int getDayOfYear(void) const;

    /*!
        Convert to system time

        @return system time (time_t).
    */
    time_t toSysTime(void) const;

    /*!
        Check if time is valid

        @return True if time is valid
    */
    bool isValid(void);

    /*!
        Check if leap year

        @return Returns true if leap year
    */
    bool isLeapYear(void);

    /*!
        Get week of year
    */
    int getWeekNumber(void) const;

    /*!
        Get weekday for set date
    */
    vscpdatetime::weekDay getWeekDay(void) const;

    /*!
        Get English weekday name for a given week day
    */
    static std::string getWeekDayName(
      weekDay weekday, nameFlags flags = vscpdatetime::name_Full);

    /*!
        Get English month name for a given month
    */
    static std::string getMonthName(month month,
                                    nameFlags flags = vscpdatetime::name_Full);

    /*!
        Check if a date/time is equal to another date/time
    */
    inline bool isEqualTo(vscpdatetime &dt)
    {
        return (toSysTime() == dt.toSysTime());
    }

    /*!
        Check if a date/time is earlier than another date/time
    */
    inline bool isEarlierThan(vscpdatetime &dt)
    {
        return (toSysTime() < dt.toSysTime());
    }

    /*!
        Check if a date/time is later than another date/time
    */
    inline bool isLaterThan(vscpdatetime &dt)
    {
        return (toSysTime() > dt.toSysTime());
    }

    /*!
        Check if a date/time is between two other date/time
    */
    inline bool isBetween(vscpdatetime &t1, vscpdatetime &t2)
    {
        return (isEqualTo(t1) || isEqualTo(t2) || isStrictlyBetween(t1, t2));
    }

    /*!
        Check if a date/time is strictly (not equal to one of them) between two
       other date/time
    */
    inline bool isStrictlyBetween(vscpdatetime &t1, vscpdatetime &t2)
    {
        return (isLaterThan(t1) && isEarlierThan(t2));
    }

    /*!
        Check if a date is the same as another date
    */
    inline bool isSameDate(vscpdatetime &dt) const
    {
        return ((dt.getYear() == m_year) && (dt.getMonth() == m_month) &&
                (dt.getDay() == m_day));
    }

    /*!
        Check if a time is the same as another time
    */
    inline bool isSameTime(vscpdatetime &dt) const
    {
        return ((dt.getHour() == m_hour) && (dt.getMinute() == m_minute) &&
                (dt.getSecond() == m_second));
    }

    static double diffSeconds(vscpdatetime &t1, vscpdatetime &t2)
    {
        return difftime(t1.toSysTime(), t2.toSysTime());
    }

    static int tzOffset2LocalTime(void);

    static long tz_offset_second(time_t t);

    //
    // Function      : Additive operators
    //
    // Author        : Todd Knarr
    //
    // Creation date : 30 Nov 1995
    //
    // Parameters    : Date value and integer, or
    //                 Two date values
    //
    // Return values : New Date, or difference between two Dates
    //
    // Description   :
    //
    // Adding an integral type to a Date adds the number of
    // days in the integral type to the Date. Adding two Dates
    // is not defined. Subtracting two dates yields the difference
    // between them in days.
    //

    friend vscpdatetime operator+(const vscpdatetime &Left, const long Right)
    {
        vscpdatetime temp = Left;
        long newval       = temp.getJulian() + Right;
        temp.setFromJulian(newval);
        return temp;
    }

    friend vscpdatetime operator+(const long Left, const vscpdatetime &Right)
    {
        vscpdatetime temp = Right;
        long newval       = temp.getJulian() + Left;
        temp.setFromJulian(newval);
        return temp;
    }

    vscpdatetime &operator+=(const long Right)
    {
        long newval = getJulian() + Right;
        setFromJulian(newval);
        return *this;
    }

    friend vscpdatetime operator-(const vscpdatetime &Left, const long Right)
    {
        vscpdatetime temp = Left;
        long newval       = temp.getJulian() - Right;
        temp.setFromJulian(newval);
        return temp;
    }

    friend vscpdatetime operator-(const long Left, const vscpdatetime &Right)
    {
        vscpdatetime temp = Right;
        long newval       = temp.getJulian() - Left;
        temp.setFromJulian(newval);
        return temp;
    }

    vscpdatetime &operator-=(const long Right)
    {
        long newval = getJulian() - Right;
        setFromJulian(newval);
        return *this;
    }

    long operator-(vscpdatetime &Right)
    {
        long newval = getJulian() - Right.getJulian();
        return newval;
    }

    //
    // Function      : ++ and -- operators, prefix and postfix forms
    //
    // Author        : Todd Knarr
    //
    // Creation date : 1 Dec 1995
    //
    // Parameters    : none
    //
    // Return values : New Date
    //
    vscpdatetime &operator++()
    {
        long newval = getJulian();
        newval++;
        setFromJulian(newval);
        return *this;
    }

    vscpdatetime operator++(int)
    {
        vscpdatetime Temp = *this;
        long newval       = getJulian();
        newval++;
        setFromJulian(newval);
        return Temp;
    }

    vscpdatetime &operator--()
    {
        long newval = getJulian();
        newval--;
        setFromJulian(newval);
        return *this;
    }

    vscpdatetime operator--(int)
    {
        vscpdatetime Temp = *this;
        long newval       = getJulian();
        newval--;
        setFromJulian(newval);
        return Temp;
    }

  private:
    uint16_t m_year;  // year
    uint8_t m_month;  // month 1-12
    uint8_t m_day;    // day 1-31
    uint8_t m_hour;   // hour 0-23
    uint8_t m_minute; // minute 0-59
    uint8_t m_second; // second 0-59

    uint32_t m_millisecond;
};

#endif