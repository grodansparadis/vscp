// $Id: date.cpp 236 2008-12-08 10:15:59Z akhe $
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
// Required files : Date.h
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

#include "date.h"

#include <time.h>

//
//  Function      : YmdToJd
//
//  Author        : Todd Knarr
//
//  Creation date : 29 Nov 1995
//
//  Parameters    : int year, month, day
//
//  Return values : long julian day
//
//  Description   :
//
// Internal routine that does the physical conversion
// from YMD form to Julian day number.
//

long Date::YmdToJd(const int iYear, const int iMonth, const int iDay)
{
    long jul_day;

#ifndef JULDATE_USE_ALTERNATE_METHOD

    int a, b;
    int year = iYear, month = iMonth, day = iDay;
    float year_corr;

    if (year < 0)
        year++;
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
    jul_day = (long) (365.25 * year - year_corr) +
        (long) (30.6001 * (month + 1)) + day + 1720995L + b;

#else

    long lmonth = (long) iMonth, lday = (long) iDay, lyear = (long) iYear;

    // Adjust BC years
    if (lyear < 0)
        lyear++;

    jul_day = lday - 32075L +
        1461L * (lyear + 4800L + (lmonth - 14L) / 12L) / 4L +
        367L * (lmonth - 2L - (lmonth - 14L) / 12L * 12L) / 12L -
        3L * ((lyear + 4900L + (lmonth - 14L) / 12L) / 100L) / 4L;

#endif

    return jul_day;
}

//
//  Function      : JdToYmd
//
//  Author        : Todd Knarr
//
//  Creation date : 29 Nov 1995
//
//  Parameters    : long julian day, pointers to int year, month, day
//
//  Return values : none
//
//  Description   :
//
// Internal routine that reverses the conversion, turning a Julian
// day number into YMD values.
//

void Date::JdToYmd(const long lJD, int *piYear, int *piMonth, int *piDay)
{
#ifndef JULDATE_USE_ALTERNATE_METHOD

    long a, b, c, d, e, z, alpha;

    z = lJD;
    if (z < 2299161L)
        a = z;
    else {
        alpha = (long) ((z - 1867216.25) / 36524.25);
        a = z + 1 + alpha - alpha / 4;
    }
    b = a + 1524;
    c = (long) ((b - 122.1) / 365.25);
    d = (long) (365.25 * c);
    e = (long) ((b - d) / 30.6001);
    *piDay = (int) b - d - (long) (30.6001 * e);
    *piMonth = (int) (e < 13.5) ? e - 1 : e - 13;
    *piYear = (int) (*piMonth > 2.5) ? (c - 4716) : c - 4715;
    if (*piYear <= 0)
        *piYear -= 1;

#else

    long t1, t2, yr, mo;

    t1 = lJD + 68569L;
    t2 = 4L * t1 / 146097L;
    t1 = t1 - (146097L * t2 + 3L) / 4L;
    yr = 4000L * (t1 + 1L) / 1461001L;
    t1 = t1 - 1461L * yr / 4L + 31L;
    mo = 80L * t1 / 2447L;
    *piDay = (int) (t1 - 2447L * mo / 80L);
    t1 = mo / 11L;
    *piMonth = (int) (mo + 2L - 12L * t1);
    *piYear = (int) (100L * (t2 - 49L) + yr + t1);

    // Correct for BC years
    if (*piYear <= 0)
        *piYear -= 1;

#endif

    return;
}

//
//  Function     : ToString
//
//  Author       : Todd Knarr
//
//  Creation date : 30 Nov 1995
//
//  Parameters    : pointer to string buffer to fill in
//
//  Return values : none
//
//  Description    :
//
// Formats the Date into an ASCII representation.
// This is the ASCII form of the long Julian day number.
// The string is a fixed-length 12-character string, including
// the NUL terminator;
//

void Date::ToString(char *szBuffer) const
{
    int i;
    long Temp;

    Temp = lJulianDay;
    if (Temp < 0L)
        szBuffer[0] = '-';
    else
        szBuffer[0] = '+';
    szBuffer[11] = '\0';
    for (i = 10; i > 0; i--) {
        szBuffer[i] = (Temp % 10L) + '0';
        Temp /= 10;
    }

    return;
}

int Date::DayOfYear(void) const
{
    int y, m, d;
    long soy;

    JdToYmd(lJulianDay, &y, &m, &d);
    soy = YmdToJd(y, 1, 1);
    return(int) (lJulianDay - soy + 1);
}

//
//  Function      : ToSysTime
//
//  Author        : Todd Knarr
//
//  Creation date : 29 Nov 1995
//
//  Parameters    : none
//
//  Return values : converted result
//
//  Description   :
//
// Converts the date to a time_t value
// representing midnight of that date.
//

time_t Date::ToSysTime(void) const
{
    struct tm tmRep;
    int y, m, d;
    time_t t;

    JdToYmd(lJulianDay, &y, &m, &d);
    if (y < 1970) {
        y = 70;
        m = 1;
        d = 1;
    }
    tmRep.tm_year = y - 1900;
    tmRep.tm_mon = m - 1;
    tmRep.tm_mday = d;
    tmRep.tm_hour = 0;
    tmRep.tm_min = 0;
    tmRep.tm_sec = 0;
    tmRep.tm_isdst = 0;

    t = mktime(&tmRep);
    return t;
}
