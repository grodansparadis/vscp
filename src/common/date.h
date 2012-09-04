#ifndef H__DATE_H
#define H__DATE_H

// $Id: date.h 236 2008-12-08 10:15:59Z akhe $
// Copyright (C) 2000, 2002
// Todd T. Knarr <tknarr@silverglass.org>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

//
// Classname      : Date
//
// Author         : Todd Knarr
//
// Creation Date  : 29 Nov 1995
//
// Superclasses   : none
//
// Subclasses     : none
//
// Required files :
//
// Description    :
//
// Provides a Date class which represents dates as
// Julian day numbers ( days since 1 Jan 4713 BC ).
// This class can handle all dates from  1 Jan 4713BC to 31 Dec 4999AD.
//
// Note: Years AD are positive, years BC are negative. There is
// no year 0AD, it goes from 1BC to 1AD. A year of 0 will be treated
// as 1BC. No attempt is made to validate ranges. Physical errors
// will not result from insane day-month combinations like the 87th
// day of the 45th month, but the results will obviously not make
// much sense.
//
// Date conversion routines  by Eric bergman-Terrell, Computer Language,
//     Dec 1990.
// Alternate method by David Burki, CUJ Feb 1993.
//
// To Do:
// * Add << and >> operators for iostreams. They should probably output
//   and input the string form.
//

#include <stdlib.h>
#include <time.h>

class Date
{

private:

    long        lJulianDay;

    //
    // Function      : YmdToJd
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : int year, month, day
    //
    // Return values : long julian day
    //
    // Description   :
    //
    // Internal routine that does the physical conversion
    // from YMD form to Julian day number.
    //
    static long YmdToJd( const int iYear, const int iMonth, const int iDay );

    //
    // Function      : JdToYmd
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : long julian day, pointers to int year, month, day
    //
    // Return values : none
    //
    // Description   :
    //
    // Internal routine that reverses the conversion, turning a Julian
    // day number into YMD values.
    //
    static void JdToYmd( const long lJD, int *piYear, int *piMonth,
                         int *piDay );

public:

    //
    // Function      : IsLeapYear
    //
    // Author        : Todd Knarr
    //
    // Creation date : 5 Dec 1995
    //
    // Parameters    : int year
    //
    // Return values : 1 if the year given is a leap year
    //                 0 if it is not
    //
    static int IsLeapYear( const int iYear )
    {
        long jd1, jd2;
        jd1 = YmdToJd( iYear, 2, 28 );
        jd2 = YmdToJd( iYear, 3, 1 );
        return ( ( jd2 - jd1 ) > 1 );
    }

    //
    // Function      : default constructor
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : none
    //
    // Return values : none
    //
    // Description   :
    //
    // Constructs a new object initialized to 1 Jan 4713BC
    //
    Date()
    {
        lJulianDay = 0L;
    }

    //
    // Function      : copy constructor
    //
    // Author        : Todd Knarr
    //
    // Creation date : 13 Mar 1996
    //
    // Parameters    : 
    //
    // Return values : 
    //
    Date( const Date& Orig ) { lJulianDay = Orig.lJulianDay; }
    Date& operator=( const Date& Orig )
    {
        lJulianDay = Orig.lJulianDay;
        return *this;
    }

    //
    // Function      : time_t constructor
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : none
    //
    // Return values : none
    //
    // Description   :
    //
    // Constructs an object initialized to the date
    // represented by a system time value.
    //
    Date( const time_t tSysTime )
    {
        struct tm *ptm;
        int y, m, d;

        ptm = localtime( &tSysTime );
        y = ptm->tm_year + 1900;
        m = ptm->tm_mon + 1;
        d = ptm->tm_mday;
        lJulianDay = YmdToJd( y, m, d );
    }

    //
    // Function      : char* ( string ) constructor
    //
    // Author        : Todd Knarr
    //
    // Creation date : 30 Nov 1995
    //
    // Parameters    :
    //
    // Return values :
    //
    // Description   :
    //
    // Constructs an object from a string.
    // The string is formatted as the ASCII representation
    // of the long Julian day number.
    //
    Date( const char *String )
    {
        lJulianDay = atol( String );
    }

    //
    // Function      : year/month/day constructor
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : int year, month, day
    //
    // Return values : none
    //
    // Description   :
    //
    // Constructs an object initialized to
    // the date given by the arguments
    //
    Date( const int iDay, const int iMonth, const int iYear )
    {
        lJulianDay = YmdToJd( iYear, iMonth, iDay );
    }

    //
    // Function      : Year, Month, Day, DayOfYear, DayOfWeek, DayOfWorkWeek,
    //                 IsLeapYear, YMD
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : none
    //
    // Return values : int or none
    //
    // Description   :
    //
    // Functions to return various parts of the date. The Year(),
    // Month() and Day() functions return the corresponding parts
    // of the date. The DayOfYear() returns the day of the year,
    // with 1 Jan being day 1. The DayOfWeek() and DayOfWorkWeek()
    // functions return the day of the week, from 0 through 6, with
    // the DayOfWeek() matching localtime()'s convention ( 0 = Sunday,
    // 6 = Saturday ) and DayOfWorkWeek() starting the week on
    // Monday ( 0 = Monday, 6 = Sunday ). IsLeapYear() returns 1 if
    // the date is in a leap year, 0 otherwise. YMD() puts the
    // year, month and day values directly into three integer variables,
    // for times when you need all three at the same time.
    //
    int Year( void ) const
    {
        int y, m, d;

        JdToYmd( lJulianDay, &y, &m, &d );
        return y;
    }
    int Month( void ) const
    {
        int y, m, d;

        JdToYmd( lJulianDay, &y, &m, &d );
        return m;
    }
    int Day( void ) const
    {
        int y, m, d;

        JdToYmd( lJulianDay, &y, &m, &d );
        return d;
    }
    int DayOfYear( void ) const;
    int DayOfWeek( void ) const
    {
        return ( ( ( (int) ( lJulianDay % 7L ) ) + 1 ) % 7 );
    }
    int DayOfWorkWeek( void ) const
    {
        return ( (int) ( lJulianDay % 7L ) );
    }
    int IsLeapYear( void ) const
    {
        int y, m, d;
        JdToYmd( lJulianDay, &y, &m, &d );
        return IsLeapYear( y );
    }
    void YMD( int *pY, int *pM, int *pD )
    {
        JdToYmd( lJulianDay, pY, pM, pD );
        return;
    }

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
    friend Date operator+( const Date& Left, const long Right )
    {
        Date Temp = Left;
        Temp.lJulianDay += Right;
        return Temp;
    }
    friend Date operator+( const long Left, const Date& Right )
    {
        Date Temp = Right;
        Temp.lJulianDay += Left;
        return Temp;
    }
    Date& operator+=( const long Right )
    {
        lJulianDay += Right;
        return *this;
    }
    friend Date operator-( const Date& Left, const long Right )
    {
        Date Temp = Left;
        Temp.lJulianDay -= Right;
        return Temp;
    }
    friend Date operator-( const long Left, const Date& Right )
    {
        Date Temp = Right;
        Temp.lJulianDay -= Left;
        return Temp;
    }
    Date& operator-=( const long Right )
    {
        lJulianDay -= Right;
        return *this;
    }
    long operator-( const Date& Right )
    {
        return lJulianDay - Right.lJulianDay;
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
    Date& operator++()
    {
        lJulianDay++;
        return *this;
    }
    Date operator++( int )
    {
        Date Temp = *this;
        lJulianDay++;
        return Temp;
    }
    Date& operator--()
    {
        lJulianDay--;
        return *this;
    }
    Date operator--( int )
    {
        Date Temp = *this;
        lJulianDay--;
        return Temp;
    }

    //
    // Function      : ToString
    //
    // Author        : Todd Knarr
    //
    // Creation date : 30 Nov 1995
    //
    // Parameters    : pointer to string buffer to fill in
    //
    // Return values : none
    //
    // Description   :
    //
    // Formats the Date into an ASCII representation.
    // This is the ASCII form of the long Julian day number.
    // The string is a fixed-length 12-character string, including
    // the NUL terminator;
    //
    void ToString( char *szBuffer ) const;

    //
    // Function      : comparison operators
    //
    // Author        : Todd Knarr
    //
    // Creation date : 1 Dec 1995
    //
    // Parameters    :
    //
    // Return values : non-zero if the comparison succeeds
    //                 0 if it fails
    //
    int operator==( const Date& Right ) const
    {
        return lJulianDay == Right.lJulianDay;
    }
    int operator!=( const Date& Right ) const
    {
        return lJulianDay != Right.lJulianDay;
    }
    int operator<( const Date& Right ) const
    {
        return lJulianDay < Right.lJulianDay;
    }
    int operator<=( const Date& Right ) const
    {
        return lJulianDay <= Right.lJulianDay;
    }
    int operator>( const Date& Right ) const
    {
        return lJulianDay > Right.lJulianDay;
    }
    int operator>=( const Date& Right ) const
    {
        return lJulianDay >= Right.lJulianDay;
    }

    //
    // Function      : ToSysTime
    //
    // Author        : Todd Knarr
    //
    // Creation date : 29 Nov 1995
    //
    // Parameters    : none
    //
    // Return values : converted result
    //
    // Description   :
    //
    // Converts the date to a time_t value
    // representing midnight of that date.
    //
    time_t ToSysTime( void ) const;

};

#endif
