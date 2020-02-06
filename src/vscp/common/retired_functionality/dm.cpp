// dm.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copysubstr (c) 2000-2018 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software", to deal
// in the Software without restriction, including without limitation the substrs
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copysubstr notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYsubstr HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include <dlfcn.h>
#include <fcntl.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>

#include <expat.h>

#include <actioncodes.h>
#include <controlobject.h>
#include <duk_module_node.h>
#include <duktape.h>
#include <duktape_vscp_func.h>
#include <duktape_vscp_wrkthread.h>
#include <lua_vscp_wrkthread.h>
#include <remotevariable.h>
#include <userlist.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpdb.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "dm.h"

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

#define XML_BUFF_SIZE 100000

///////////////////////////////////////////////////////////////////////////////
// Constructor dmTimer
//

dmTimer::dmTimer()
{
    m_pThread = NULL; // No working thread

    m_id            = 0;
    m_bActive       = false;
    m_delayStart    = 0;
    m_delay         = 0;
    m_bReload       = false;
    m_nameVariable  = "";
    m_reloadlimit   = -1;
    m_reloadCounter = -1;
}

dmTimer::dmTimer(std::string &nameVar,
                 uint32_t id,
                 uint32_t delay,
                 bool bStart,
                 bool bReload,
                 int reloadLimit)
{
    init(nameVar, id, delay, bStart, bReload, reloadLimit);
}

///////////////////////////////////////////////////////////////////////////////
// Destructor dmTimer
//

dmTimer::~dmTimer() {}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
dmTimer::init(std::string &nameVar,
              uint32_t id,
              uint32_t delay,
              bool bStart,
              bool bReload,
              int reloadLimit)
{
    m_bPaused = false;
    m_pThread = NULL; // No working thread

    m_id         = id;
    m_bActive    = bStart;
    m_delayStart = m_delay = delay;
    m_bReload              = bReload;
    m_nameVariable         = nameVar;
    m_reloadCounter = m_reloadlimit = reloadLimit;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

void
dmTimer::startTimer(void)
{
    reload();
    resumeTimer();
};

///////////////////////////////////////////////////////////////////////////////
// stopTimer
//

void
dmTimer::stopTimer(void)
{
    m_bActive = false;
    m_bPaused = false;
};

///////////////////////////////////////////////////////////////////////////////
// pauseTimer
//

void
dmTimer::pauseTimer(void)
{
    m_bActive = false;
    m_bPaused = true;
};

///////////////////////////////////////////////////////////////////////////////
// resumeTimer
//

void
dmTimer::resumeTimer(void)
{
    if (m_delay) {
        m_bActive       = true;
        m_bPaused       = false;
        m_reloadCounter = m_reloadlimit;
    };
};

///////////////////////////////////////////////////////////////////////////////
// isActive
//

bool
dmTimer::isActive(void)
{
    return m_bActive;
};

///////////////////////////////////////////////////////////////////////////////
// setActive
//

void
dmTimer::setActive(bool bState)
{
    m_bActive = bState;
};

///////////////////////////////////////////////////////////////////////////////
// decTimer
//

uint32_t
dmTimer::decTimer(void)
{
    if (m_delay) m_delay--;
    return m_delay;
};

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Constructor  actionTime
//

actionTime::actionTime()
{
    allowAlways();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

actionTime::~actionTime()
{
    clearTables();
}

///////////////////////////////////////////////////////////////////////////////
// clearTables
//

void
actionTime::clearTables()
{
    /*std::set<int>::iterator iter;
    for (iter = m_actionYear.begin(); iter != m_actionYear.end(); ++iter) {
        if (NULL != *iter) {
            delete *iter;
        }
    }*/

    m_actionYear.clear();

    /*for (iter = m_actionMonth.begin(); iter != m_actionMonth.end(); ++iter) {
        if (NULL != *iter) {
            delete *iter;
        }
    }*/

    m_actionMonth.clear();

    /*for (iter = m_actionDay.begin(); iter != m_actionDay.end(); ++iter) {
        if (NULL != *iter) {
            delete *iter;
        }
    }*/

    m_actionDay.clear();

    /*for (iter = m_actionHour.begin(); iter != m_actionHour.end(); ++iter) {
        if (NULL != *iter) {
            delete *iter;
        }
    }*/

    m_actionHour.clear();

    /*for (iter = m_actionMinute.begin(); iter != m_actionMinute.end(); ++iter)
    { if (NULL != *iter) { delete *iter;
        }
    }*/

    m_actionMinute.clear();

    /*for (iter = m_actionSecond.begin(); iter != m_actionSecond.end(); ++iter)
    { if (NULL != *iter) { delete *iter;
        }
    }*/

    m_actionSecond.clear();
}

///////////////////////////////////////////////////////////////////////////////
// allowAlways
//

void
actionTime::allowAlways()
{
    clearTables();

    for (int i = 0; i < 7; i++) {
        m_weekDay[i] = true; // Allow for all weekdays
    }

    // Allow from the beginning of time
    setStartTime("*");

    // to the end of time
    setEndTime("*");

    parseActionTime("*-*-* *:*:*");
}

///////////////////////////////////////////////////////////////////////////////
// setWeekDays
//

bool
actionTime::setWeekDays(const std::string &strWeek)
{
    std::string str = strWeek;

    // Trim it
    vscp_trim(str);

    if (7 != str.length()) return false;

    for (int i = 0; i < 7; i++) {
        if ('-' == str[i]) {
            m_weekDay[i] = false;
        } else {
            m_weekDay[i] = true;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getWeekDays
//

std::string
actionTime::getWeekDays(void)
{
    std::string strWeekDays;

    for (int i = 0; i < 7; i++) {
        if (m_weekDay[i]) {
            switch (i) {
                case 0:
                    strWeekDays += 'm';
                    break;
                case 1:
                    strWeekDays += 't';
                    break;
                case 2:
                    strWeekDays += 'w';
                    break;
                case 3:
                    strWeekDays += 't';
                    break;
                case 4:
                    strWeekDays += 'f';
                    break;
                case 5:
                    strWeekDays += 's';
                    break;
                case 6:
                    strWeekDays += 's';
                    break;
            }
        } else {
            strWeekDays += '-';
        }
    }

    return strWeekDays;
}

///////////////////////////////////////////////////////////////////////////////
// setWeekDay
//

bool
actionTime::setWeekDay(const std::string &strWeekday)
{
    std::string str = strWeekday;

    vscp_trim(str);

    if (0 == strcasecmp(str.c_str(), "MONDAY")) {
        m_weekDay[0] = true;
    } else if (0 == strcasecmp(str.c_str(), "TUESDAY")) {
        m_weekDay[1] = true;
    } else if (0 == strcasecmp(str.c_str(), "WEDNESDAY")) {
        m_weekDay[2] = true;
    } else if (0 == strcasecmp(str.c_str(), "THURSDAY")) {
        m_weekDay[3] = true;
    } else if (0 == strcasecmp(str.c_str(), "FRIDAY")) {
        m_weekDay[4] = true;
    } else if (0 == strcasecmp(str.c_str(), "SATURDAY")) {
        m_weekDay[5] = true;
    } else if (0 == strcasecmp(str.c_str(), "SUNDAY")) {
        m_weekDay[6] = true;
    } else {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getMultiItem
//

bool
actionTime::getMultiItem(const std::string &items, std::set<int> *pList)
{
    // Check pointer
    if (NULL == pList) return false;

    // Clear list
    pList->clear();

    std::deque<std::string> tokens;
    vscp_split(tokens, items, "/");

    if (tokens.size() > 1) {
        std::string token;
        while (tokens.size()) {
            token = tokens.front();
            tokens.pop_front();
            vscp_trim(token);

            // 'n' or 'n-m'  TODO add @2 == every second minute
            std::deque<std::string> tkzRange;
            vscp_split(tkzRange, token, "-");

            if (tkzRange.size() > 1) {
                int from = vscp_readStringValue(tkzRange.front());
                tkzRange.pop_front();
                int to = vscp_readStringValue(tkzRange.front());
                tkzRange.pop_front();
                if (from < to) {
                    for (int i = from; i < to; i++) {
                        pList->insert(i);
                    }
                }
            } else {
                int val = (int)vscp_readStringValue(items);
                pList->insert(val);
            }
        }
    } else {
        // One Token ( or none )
        // If '*' we do nothing (same for none) and
        // list will be have count == 0 => No care.
        unsigned long val;
        std::string token = tokens.front();
        tokens.pop_front();

        val = vscp_readStringValue(token);
        pList->insert(val);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// parseActionTime
//

bool
actionTime::parseActionTime(const std::string &actionTime)
{
    std::string buf = actionTime;
    std::string strDate;
    std::string strYear;
    std::string strMonth;
    std::string strDay;

    std::string strTime;
    std::string strHour;
    std::string strMinute;
    std::string strSecond;

    vscp_trim(buf);

    // '*' means "always"
    if ("*" == buf) {
        buf = "*-*-* *:*:*";
    }

    // Formats:
    // YYYY-MM-SS HH:MM:SS
    // * *
    // *-*-* *:*:*
    // YYYY-0/1/4/5-DD HH:MM:SS or variants of it
    // n-m  . n, n+1, n+2,...
    std::deque<std::string> tkzFull;
    vscp_split(tkzFull, buf, " ");

    if (tkzFull.size() < 2) {
        return false; // Wrong format
    }

    // Get date
    strDate = tkzFull.front();
    tkzFull.pop_front();
    vscp_trim(strDate);

    // Get Time
    strTime = tkzFull.front();
    tkzFull.pop_front();
    vscp_trim(strTime);

    // Date
    if (!(strDate == "*")) {
        // Work on date separator is '-'
        std::deque<std::string> tkzDate;
        vscp_split(tkzDate, strDate, "-");

        if (tkzDate.size() < 3) return false; // Wrong format

        // Get year
        strYear = tkzDate.front();
        tkzDate.pop_front();
        vscp_trim(strYear);
        getMultiItem(strYear, &m_actionYear);

        // Get month
        strMonth = tkzDate.front();
        tkzDate.pop_front();
        vscp_trim(strMonth);
        getMultiItem(strMonth, &m_actionMonth);

        // Get day
        strDay = tkzDate.front();
        tkzDate.pop_front();
        vscp_trim(strDay);
        getMultiItem(strDay, &m_actionDay);
    } else {

        m_actionYear.clear();
        m_actionMonth.clear();
        m_actionDay.clear();
    }

    // Time
    if (!(strTime == "*")) {
        // Work on date separator is ':'
        std::deque<std::string> tkzTime;
        vscp_split(tkzTime, strTime, ":");
        // if ( tkzTime.size() < 3 ) return false;	// Wrong format

        // Get hour
        strHour = tkzTime.front();
        tkzTime.pop_front();
        vscp_trim(strHour);
        getMultiItem(strHour, &m_actionHour);

        // Get minute
        strMinute = tkzTime.front();
        tkzTime.pop_front();
        vscp_trim(strMinute);
        getMultiItem(strMinute, &m_actionMinute);

        // Get second
        strSecond = tkzTime.front();
        tkzTime.pop_front();
        vscp_trim(strSecond);
        getMultiItem(strSecond, &m_actionSecond);
    } else {

        m_actionHour.clear();
        m_actionMinute.clear();
        m_actionSecond.clear();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// ShouldWeDoAction
//

bool
actionTime::ShouldWeDoAction(void)
{
    bool bMatch;
    vscpdatetime now; // Get current date/time

    // Check that times are valid
    if (!m_startTime.isValid()) {
        return false;
    }

    if (!m_endTime.isValid()) {
        return false;
    }

    // for debug
    // std::string s1 = m_startTime.getISODateTime(); std::string s2 =
    // m_endTime.getISODate() +
    // "
    // "
    // + m_endTime.getISOTime(); std::string s3 = now.getISODateTime();

    // If we are before starttime
    if (now.isEarlierThan(m_startTime)) {
        return false;
    }

    // If we are after endtime
    if (now.isLaterThan(m_endTime)) return false;

    vscpdatetime::weekDay day = now.getWeekDay();
    if ((day == vscpdatetime::Mon) && (0 == m_weekDay[0])) return false;
    if ((day == vscpdatetime::Tue) && (0 == m_weekDay[1])) return false;
    if ((day == vscpdatetime::Wed) && (0 == m_weekDay[2])) return false;
    if ((day == vscpdatetime::Thu) && (0 == m_weekDay[3])) return false;
    if ((day == vscpdatetime::Fri) && (0 == m_weekDay[4])) return false;
    if ((day == vscpdatetime::Sat) && (0 == m_weekDay[5])) return false;
    if ((day == vscpdatetime::Sun) && (0 == m_weekDay[6])) return false;

    // Check Year
    if (m_actionYear.size()) {
        bMatch = false;
        std::set<int>::iterator it;
        for (it = m_actionYear.begin(); it != m_actionYear.end(); ++it) {
            if (*it == vscpdatetime::Now().getYear()) {
                bMatch = true;
                break;
            }
        }

        // Fail if no match found
        if (!bMatch) {
            return false;
        }
    }

    // Check Month
    if (m_actionMonth.size()) {
        bMatch = false;
        std::set<int>::iterator it;
        for (it = m_actionMonth.begin(); it != m_actionMonth.end(); ++it) {
            if (*it == vscpdatetime::Now().getMonth()) {
                bMatch = true;
                break;
            }
        }

        // Fail if no match found
        if (!bMatch) {
            return false;
        }
    }

    // Check Day
    if (m_actionDay.size()) {
        bMatch = false;
        std::set<int>::iterator it;
        for (it = m_actionDay.begin(); it != m_actionDay.end(); ++it) {
            if (*it == vscpdatetime::Now().getDay()) {
                bMatch = true;
                break;
            }
        }

        // Fail if no match found
        if (!bMatch) {
            return false;
        }
    }

    // Check Hour
    if (m_actionHour.size()) {
        bMatch = false;
        std::set<int>::iterator it;
        for (it = m_actionHour.begin(); it != m_actionHour.end(); ++it) {
            if (*it == vscpdatetime::Now().getHour()) {
                bMatch = true;
                break;
            }
        }

        // Fail if no match found
        if (!bMatch) {
            return false;
        }
    }

    // Check Minute
    if (m_actionMinute.size()) {
        bMatch = false;
        std::set<int>::iterator it;
        for (it = m_actionMinute.begin(); it != m_actionMinute.end(); ++it) {
            if (*it == vscpdatetime::Now().getMinute()) {
                bMatch = true;
                break;
            }
        }

        // Fail if no match found
        if (!bMatch) {
            return false;
        }
    }

    // Check Second
    if (m_actionSecond.size()) {
        bMatch = false;
        std::set<int>::iterator it;
        for (it = m_actionSecond.begin(); it != m_actionSecond.end(); ++it) {
            if (*it == vscpdatetime::Now().getSecond()) {
                bMatch = true;
                break;
            }
        }

        // Fail if no match found
        if (!bMatch) {
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getActionTimeAsString
//

std::string
actionTime::getActionTimeAsString(void)
{
    std::string str;
    int cnt;

    // ActionYear
    if ((cnt = m_actionYear.size())) {
        std::set<int>::iterator it;
        for (it = m_actionYear.begin(); it != m_actionYear.end(); ++it) {
            str += vscp_str_format("%4d", *it);
            // Add backslash for all but last
            cnt--;
            if (cnt) {
                str += "/";
            } else {
                // Add date separator
                str += "-";
            }
        }
    } else {
        str += "*-"; // All years trigger.
    }

    // ActionMonth
    if ((cnt = m_actionMonth.size())) {
        std::set<int>::iterator it;
        for (it = m_actionMonth.begin(); it != m_actionMonth.end(); ++it) {
            str += vscp_str_format("%02d", *it);
            // Add backslash for all but last
            cnt--;
            if (cnt) {
                str += "/";
            } else {
                // Add date separator
                str += "-";
            }
        }
    } else {
        str += "*-"; // All months trigger.
    }

    // ActionDay
    if ((cnt = m_actionDay.size())) {
        std::set<int>::iterator it;
        for (it = m_actionDay.begin(); it != m_actionDay.end(); ++it) {
            str += vscp_str_format("%02d", *it);
            // Add backslash for all but last
            cnt--;
            if (cnt) {
                str += "/";
            }
        }
    } else {
        str += "*"; // All days trigger.
    }

    // Add seperator to time part
    str += " ";

    // ActionHour
    if ((cnt = m_actionHour.size())) {
        std::set<int>::iterator it;
        for (it = m_actionHour.begin(); it != m_actionHour.end(); ++it) {
            str += vscp_str_format("%02d", *it);
            // Add backslash for all but last
            cnt--;
            if (cnt) {
                str += "/";
            } else {
                // Add date separator
                str += ":";
            }
        }
    } else {
        str += "*:"; // All hours trigger.
    }

    // ActionMinute
    if ((cnt = m_actionMinute.size())) {
        std::set<int>::iterator it;
        for (it = m_actionMinute.begin(); it != m_actionMinute.end(); ++it) {
            str += vscp_str_format("%02d", *it);
            // Add backslash for all but last
            cnt--;
            if (cnt) {
                str += "/";
            } else {
                // Add date separator
                str += ":";
            }
        }
    } else {
        str += "*:"; // All minutes trigger.
    }

    // ActionSeconds
    if ((cnt = m_actionSecond.size())) {
        std::set<int>::iterator it;
        for (it = m_actionSecond.begin(); it != m_actionSecond.end(); ++it) {
            str += vscp_str_format("%02d", *it);
            // Add backslash for all but last
            cnt--;
            if (cnt) {
                str += "/";
            }
        }
    } else {
        str += "*"; // All seconds trigger.
    }

    return str;
}

///////////////////////////////////////////////////////////////////////////////

// Constructor

dmElement::dmElement()
{
    m_bEnable = false;
    m_bStatic = false;
    m_id      = 0;
    m_strGroupID.clear();
    vscp_clearVSCPFilter(&m_vscpfilter);
    m_actionCode   = 0;
    m_triggCounter = 0;
    m_errorCounter = 0;
    m_actionparam.clear();
    m_comment.clear();

    m_bCheckIndex = false;
    m_index       = 0;

    m_bCheckMeasurementIndex = false;

    m_bCheckZone = false;
    m_zone       = 0;

    m_bCheckSubZone = false;
    m_subzone       = 0;

    m_timeAllow.allowAlways();

    m_bCompareMeasurement    = false; // No measurement comparison
    m_measurementValue       = 0;
    m_measurementUnit        = 0; // Default unit
    m_measurementCompareCode = DM_MEASUREMENT_COMPARE_NOOP;

    m_pDM = NULL; // Initially no owner
};

dmElement::~dmElement()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Assignment =
//

dmElement &
dmElement::operator=(const dmElement &dm)
{
    // Check for self-assignment!
    if (this == &dm) { // Same object?
        return *this;  // Yes, so skip assignment, and just return
                       // *this.
    }

    m_id         = dm.m_id;
    m_bStatic    = dm.m_bStatic;
    m_bEnable    = dm.m_bEnable;
    m_strGroupID = dm.m_strGroupID;
    memcpy(&m_vscpfilter, &dm.m_vscpfilter, sizeof(vscpEventFilter));
    m_actionCode             = dm.m_actionCode;
    m_actionparam            = dm.m_actionparam;
    m_triggCounter           = dm.m_triggCounter;
    m_errorCounter           = dm.m_errorCounter;
    m_bCheckIndex            = dm.m_bCheckIndex;
    m_index                  = dm.m_index;
    m_bCheckMeasurementIndex = dm.m_bCheckMeasurementIndex;
    m_bCheckZone             = dm.m_bCheckZone;
    m_zone                   = dm.m_zone;
    m_bCheckSubZone          = dm.m_bCheckSubZone;
    m_subzone                = dm.m_subzone;
    m_strLastError           = dm.m_strLastError;
    m_comment                = dm.m_comment;
    m_timeAllow              = dm.m_timeAllow;
    m_bCompareMeasurement    = dm.m_bCompareMeasurement;
    m_measurementValue       = dm.m_measurementValue;
    m_measurementUnit        = dm.m_measurementUnit;
    m_measurementCompareCode = dm.m_measurementCompareCode;
    m_pDM                    = dm.m_pDM;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//

std::string
dmElement::getAsString(bool bCRLF)
{
    std::string strRow;

    // id
    strRow = vscp_str_format("%d,", m_id);

    // bEnable
    if (isEnabled()) {
        strRow += "true,";
    } else {
        strRow += "false,";
    }

    // GroupID
    strRow += m_strGroupID;
    strRow += ",";

    // Priority mask
    strRow += vscp_str_format("0x%02X,", m_vscpfilter.mask_priority);

    // Class mask
    strRow += vscp_str_format("0x%04X,", m_vscpfilter.mask_class);

    // Type mask
    strRow += vscp_str_format("0x%04X,", m_vscpfilter.mask_type);

    // GUID mask
    std::string strGUID;
    vscp_writeGuidArrayToString(m_vscpfilter.mask_GUID, strGUID);
    strRow += strGUID;
    strRow += ",";

    // Priority filter
    strRow += vscp_str_format("0x%02X,", m_vscpfilter.filter_priority);

    // Class filter
    strRow += vscp_str_format("0x%04X,", m_vscpfilter.filter_class);

    // Type filter
    strRow += vscp_str_format("0x%04X,", m_vscpfilter.filter_type);

    // GUID filter
    vscp_writeGuidArrayToString(m_vscpfilter.filter_GUID, strGUID);
    strRow += strGUID;
    strRow += ",";

    // From time
    strRow += m_timeAllow.getStartTime().getISODateTime() + ",";

    // End time
    strRow += m_timeAllow.getEndTime().getISODateTime() + ",";

    // Allowed weekdays
    strRow += m_timeAllow.getWeekDays() + ",";

    // Action time
    strRow += m_timeAllow.getActionTimeAsString() + ",";

    // bIndex
    strRow += m_bCheckIndex ? "true," : "false,";

    // index
    strRow += vscp_str_format("%d,", m_index);

    // bMeasurementIndex
    strRow += m_bCheckMeasurementIndex ? "true," : "false,";

    // bCheckZone
    strRow += m_bCheckZone ? "true," : "false,";

    // zone
    strRow += vscp_str_format("%d,", m_zone);

    // bCheckSubZone
    strRow += m_bCheckSubZone ? "true," : "false,";

    // subzone
    strRow += vscp_str_format("%d,", m_subzone);

    // m_bCompareMeasurement
    strRow += m_bCompareMeasurement ? "true," : "false,";

    // measurement value
    strRow += vscp_str_format("%lf,", m_measurementValue);

    // measurement unit
    strRow += vscp_str_format("%d,", m_measurementUnit);

    // measurement compare code
    strRow += getSymbolicMeasurementFromCompareCode(m_measurementCompareCode);
    strRow += ",";

    // Action Code
    strRow += vscp_str_format("0x%08X,", m_actionCode);

    // Action Parameters
    strRow += m_actionparam;
    strRow += ",";

    // trig-counter:
    strRow += vscp_str_format("%d,", m_triggCounter);

    // error-counter:
    strRow += vscp_str_format("%d,", m_errorCounter);

    // Last error
    strRow += m_strLastError;
    strRow += ",";

    strRow += m_comment;

    // New line
    if (bCRLF) {
        strRow += "\r";
    }

    return strRow;
}

///////////////////////////////////////////////////////////////////////////////
// getCompareCodeFromSymbolicMeasurement
//

uint8_t
dmElement::getCompareCodeFromSymbolicMeasurement(std::string &strCompare)
{
    uint8_t cc = DM_MEASUREMENT_COMPARE_NOOP;

    vscp_trim(strCompare);

    if (0 == strcasecmp(strCompare.c_str(), "EQ")) {
        cc = DM_MEASUREMENT_COMPARE_EQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "==")) {
        cc = DM_MEASUREMENT_COMPARE_EQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "NEQ")) {
        cc = DM_MEASUREMENT_COMPARE_NEQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "!=")) {
        cc = DM_MEASUREMENT_COMPARE_NEQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "LT")) {
        cc = DM_MEASUREMENT_COMPARE_LT;
    } else if (0 == strcasecmp(strCompare.c_str(), "<")) {
        cc = DM_MEASUREMENT_COMPARE_LT;
    } else if (0 == strcasecmp(strCompare.c_str(), "LTEQ")) {
        cc = DM_MEASUREMENT_COMPARE_LTEQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "<=")) {
        cc = DM_MEASUREMENT_COMPARE_LTEQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "GT")) {
        cc = DM_MEASUREMENT_COMPARE_GT;
    } else if (0 == strcasecmp(strCompare.c_str(), ">")) {
        cc = DM_MEASUREMENT_COMPARE_GT;
    } else if (0 == strcasecmp(strCompare.c_str(), "GTEQ")) {
        cc = DM_MEASUREMENT_COMPARE_GTEQ;
    } else if (0 == strcasecmp(strCompare.c_str(), ">=")) {
        cc = DM_MEASUREMENT_COMPARE_GTEQ;
    } else if (0 == strcasecmp(strCompare.c_str(), "NOOP")) {
        cc = DM_MEASUREMENT_COMPARE_NOOP;
    }

    return cc;
}

///////////////////////////////////////////////////////////////////////////////
// getSymbolicMeasurementCompareCode
//

std::string
dmElement::getSymbolicMeasurementFromCompareCode(uint8_t cc, uint8_t type)
{
    std::string scc = "NOOP";

    if (DM_MEASUREMENT_COMPARE_EQ == cc) {
        if (0 == type) {
            scc = "==";
        } else {
            scc = "EQ";
        }
    } else if (DM_MEASUREMENT_COMPARE_NEQ == cc) {
        if (0 == type) {
            scc = "!=";
        } else {
            scc = "NEQ";
        }
    } else if (DM_MEASUREMENT_COMPARE_LT == cc) {
        if (0 == type) {
            scc = "<";
        } else {
            scc = "LT";
        }
    } else if (DM_MEASUREMENT_COMPARE_LTEQ == cc) {
        if (0 == type) {
            scc = "<=";
        } else {
            scc = "LTEQ";
        }
    } else if (DM_MEASUREMENT_COMPARE_GT == cc) {
        if (0 == type) {
            scc = ">";
        } else {
            scc = "GT";
        }
    } else if (DM_MEASUREMENT_COMPARE_GTEQ == cc) {
        if (0 == type) {
            scc = ">=";
        } else {
            scc = "GTEQ";
        }
    } else {
        scc = "NOOP";
    }

    return scc;
}

///////////////////////////////////////////////////////////////////////////////
// getMeasurementGroup
//

std::string
dmElement::getMeasurementGroup(void)
{
    std::string str;

    str = m_bCompareMeasurement ? "true," : "false,";
    str += vscp_str_format("%f,%d,%d,",m_measurementValue, (int)m_measurementUnit );
    str += getSymbolicMeasurementFromCompareCode(m_measurementCompareCode);
    return str;
}


///////////////////////////////////////////////////////////////////////////////
// setMeasurementGroup
//

bool
dmElement::setMeasurementGroup(std::string& group)
{
    std::string strToken;
    std::deque<std::string> tokens;
    vscp_split( tokens, group, "," );

    // enable
    if ( tokens.empty() ) return false;
    strToken = tokens.front();
    tokens.pop_front();
    vscp_trim(strToken);
    vscp_makeLower(strToken);

    if ( "true" == strToken ) {
        m_bCompareMeasurement = true;
    }
    else {
        m_bCompareMeasurement = false;
    }

    // value
    if ( tokens.empty() ) return false;
    strToken = tokens.front();
    tokens.pop_front();
    try {
        m_measurementValue = std::stod(strToken);
    }
    catch (...) {
        return false;
    }

    // unit
    if ( tokens.empty() ) return false;
    strToken = tokens.front();
    tokens.pop_front();
    m_measurementUnit = vscp_readStringValue(strToken);

    // compare code/string
    if ( tokens.empty() ) return false;
    strToken = tokens.front();
    tokens.pop_front();
    vscp_trim(strToken);
    vscp_makeLower(strToken);

    if ( vscp_isNumber( strToken )) {
        m_measurementCompareCode = vscp_readStringValue( strToken );
    }
    else {
        if ( !setSymbolicMeasurementCompareCode(strToken) ) {
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setFromString
//

bool
dmElement::setFromString(std::string &strDM)
{
    std::string str;
    long unsigned int lval;

    std::deque<std::string> tokens;
    vscp_split(tokens, strDM, ",");

    // bEnable
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (0 == strcasecmp(str.c_str(), "true")) {
            m_bEnable = true;
        } else {
            m_bEnable = false;
        }
    }

    // GroupID
    if (!tokens.empty()) {
        m_strGroupID = tokens.front();
        tokens.pop_front();
    }

    // Mask priority
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_vscpfilter.mask_priority = vscp_readStringValue(str);
    }

    // Mask class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_vscpfilter.mask_class = vscp_readStringValue(str);
    }

    // Mask type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_vscpfilter.mask_type = vscp_readStringValue(str);
    }

    // mask GUID
    if (!tokens.empty()) {
        cguid guid;
        str = tokens.front();
        tokens.pop_front();
        guid.getFromString(str);
        memcpy(m_vscpfilter.mask_GUID, guid.getGUID(), 16);
    }

    // Filter priority
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_vscpfilter.filter_priority = vscp_readStringValue(str);
    }

    // Filter class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_vscpfilter.filter_class = vscp_readStringValue(str);
    }

    // Filter type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_vscpfilter.filter_type = vscp_readStringValue(str);
    }

    // Filter GUID
    if (!tokens.empty()) {
        cguid guid;
        str = tokens.front();
        tokens.pop_front();
        guid.getFromString(str);
        memcpy(m_vscpfilter.filter_GUID, guid.getGUID(), 16);
    }

    // Allowed start
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
    }

    // Allowed end
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
    }

    // Allowed weekdays
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_timeAllow.setWeekDay(str);
    }

    // Allowed time
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_timeAllow.parseActionTime(str);
    }

    // bCheckIndex
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_makeUpper(str);

        if (0 == strcasecmp(str.c_str(), "TRUE")) {
            m_bCheckIndex = true;
        } else {
            m_bCheckIndex = false;
        }
    }

    // index
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_index = vscp_readStringValue(str);
    }

    // bCheckZone
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_makeUpper(str);
        if (0 == strcasecmp(str.c_str(), "TRUE")) {
            m_bCheckZone = true;
        } else {
            m_bCheckZone = false;
        }
    }

    // zone
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_zone = vscp_readStringValue(str);
    }

    // bCheckSubZone
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        if (0 == strcasecmp(str.c_str(), "true")) {
            m_bCheckSubZone = true;
        } else {
            m_bCheckSubZone = false;
        }
    }

    // subzone
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_subzone = vscp_readStringValue(str);
    }

    // bCheckMeasurementIndex
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_makeUpper(str);
        if (0 == strcasecmp(str.c_str(), "true")) {
            m_bCheckMeasurementIndex = true;
        } else {
            m_bCheckMeasurementIndex = false;
        }
    }

    // actioncode
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_actionCode = vscp_readStringValue(str);
    }

    // Action parameter
    if (!tokens.empty()) {
        m_actionparam = tokens.front();
        tokens.pop_front();
    }

    // Measurement compare flag
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_makeUpper(str);
        if (0 == strcasecmp(str.c_str(), "true")) {
            m_bCompareMeasurement = true;
        } else {
            m_bCompareMeasurement = false;
        }
    }

    // Measurement value
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_measurementValue = stod(str);
    }

    // measurement unit
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_measurementUnit = vscp_readStringValue(str);
    }

    // measurement compare code
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_measurementCompareCode = vscp_readStringValue(str);
    }

    // comment
    if (!tokens.empty()) {
        m_comment = tokens.front();
        tokens.pop_front();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// handleEscapes
//

bool
dmElement::handleEscapes(vscpEvent *pEvent, std::string &str)
{
    int pos;
    std::string strResult;

    while (str.npos != (pos = str.find('%'))) {
        strResult += str.substr(pos);
        str = str.substr(str.length() - pos);

        if (str.length() > 1) {
            // Check for percent i.e. %%
            if (vscp_startsWith(str, "%%", &str)) {
                strResult += "%"; // add percent
            }
            // Check for ';' i.e. %;
            else if (vscp_startsWith(str, "%;", &str)) {
                strResult += ";"; // add semicolon
            }
            // Check for %cr
            else if (vscp_startsWith(str, "%cr", &str)) {
                strResult += "\r"; // add carrige return
            }
            // Check for %lf
            else if (vscp_startsWith(str, "%lf", &str)) {
                strResult += ""; // add new line
            }
            // Check for %crlf
            else if (vscp_startsWith(str, "%crlf", &str)) {
                strResult += "\r"; // add carrige return line feed
            }
            // Check for %tab
            else if (vscp_startsWith(str, "%tab", &str)) {
                strResult += "\t"; // add tab
            }
            // Check for %bell
            else if (vscp_startsWith(str, "%bell", &str)) {
                strResult += "\a"; // add bell
            }
            // Check for %amp.html
            else if (vscp_startsWith(str, "%amp.html", &str)) {
                strResult += "&amp;"; // add bell
            }
            // Check for %amp
            else if (vscp_startsWith(str, "%amp", &str)) {
                strResult += "&"; // add bell
            }
            // Check for %lt.html
            else if (vscp_startsWith(str, "%lt.html", &str)) {
                strResult += "&lt;"; // add bell
            }
            // Check for %lt
            else if (vscp_startsWith(str, "%lt", &str)) {
                strResult += "<"; // add bell
            }
            // Check for %gt.html
            else if (vscp_startsWith(str, "%gt.html", &str)) {
                strResult += "&gt;"; // add bell
            }
            // Check for %gt
            else if (vscp_startsWith(str, "%gt", &str)) {
                strResult += ">"; // add bell
            }
            // Check for head escape
            else if (vscp_startsWith(str, "%event.head", &str)) {
                strResult += vscp_str_format("%d", pEvent->head);
            }
            // Check for priority escape
            else if (vscp_startsWith(str, "%event.priority", &str)) {
                strResult += vscp_str_format(
                  "%d", ((pEvent->head & VSCP_HEADER_PRIORITY_MASK) >> 5));
            }
            // Check for hardcoded escape
            else if (vscp_startsWith(str, "%event.hardcoded", &str)) {
                if (pEvent->head & VSCP_HEADER_HARD_CODED) {
                    strResult += "1";
                } else {
                    strResult += "0";
                }
            }
            // Check for class escape
            else if (vscp_startsWith(str, "%event.class", &str)) {
                strResult += vscp_str_format("%d", pEvent->vscp_class);
            }
            // Check for class string  escape
            else if (vscp_startsWith(str, "%event.class.str", &str)) {
                // VSCPInformation info;  TODO
                // strResult +=  info.getClassDescription(
                // pEvent->vscp_class );
            }
            // Check for type escape
            else if (vscp_startsWith(str, "%event.type", &str)) {
                strResult += vscp_str_format("%d", pEvent->vscp_type);
            }
            // Check for type string escape
            else if (vscp_startsWith(str, "%event.type.str", &str)) {
                // VSCPInformation info;  TODO
                // strResult +=  info.getTypeDescription(
                // pEvent->vscp_class,
                //                                    pEvent->vscp_type
                //                                    );
            }
            // Check for data[n] escape
            else if (vscp_startsWith(str, "%event.data[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if (idx < pEvent->sizeData) {
                        strResult +=
                          vscp_str_format("%d", pEvent->pdata[idx]);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data[n] escape
            else if (vscp_startsWith(str, "%event.hexdata[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if (idx < pEvent->sizeData) {
                        strResult +=
                          vscp_str_format("%02X", pEvent->pdata[idx]);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.bit[n] escape
            else if (vscp_startsWith(str, "%event.data.bit[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx     = 0;
                    idx          = vscp_readStringValue(str);
                    uint8_t byte = idx / 8;
                    uint8_t bit  = idx % 8;
                    if (byte < pEvent->sizeData) {
                        strResult += vscp_str_format(
                          "%d",
                          (pEvent->pdata[byte] & (1 << (7 - bit))) ? 1 : 0);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.bool[n] escape
            else if (vscp_startsWith(str, "%event.data.bool[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if (idx < pEvent->sizeData) {
                        strResult += vscp_str_format(
                          "%s", pEvent->pdata[idx] ? "true" : "false");
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.int8[n] escape
            else if (vscp_startsWith(str, "%event.data.int8[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if (idx < pEvent->sizeData) {
                        strResult +=
                          vscp_str_format("%d", (int8_t)pEvent->pdata[idx]);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.uint8[n] escape
            else if (vscp_startsWith(str, "%event.data.uint8[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if (idx < pEvent->sizeData) {
                        strResult +=
                          vscp_str_format("%d", (uint8_t)pEvent->pdata[idx]);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.uint8[n] escape
            else if (vscp_startsWith(str, "%event.hexdata.uint8[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if (idx < pEvent->sizeData) {
                        strResult += vscp_str_format(
                          "%02X", (uint8_t)pEvent->pdata[idx]);
                    } else {
                        strResult += "? "; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += " "; // No data
                }
            }
            // Check for data.int16[n] escape
            else if (vscp_startsWith(str, "%event.data.int16[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    long idx = 0;
                    vscp_readStringValue(str);
                    if ((idx + 1) < pEvent->sizeData) {
                        int16_t val = ((int16_t)pEvent->pdata[idx] << 8) +
                                      ((int16_t)pEvent->pdata[idx + 1]);
                        strResult += vscp_str_format("%d", (int)val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.uint16[n] escape
            else if (vscp_startsWith(str, "%event.data.uint16[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 1) < pEvent->sizeData) {
                        uint16_t val = ((uint16_t)pEvent->pdata[idx] << 8) +
                                       ((uint16_t)pEvent->pdata[idx + 1]);
                        strResult += vscp_str_format("%d", (int)val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            } // Check for data.uint16[n] escape
            else if (vscp_startsWith(str, "%event.hexdata.uint16[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 1) < pEvent->sizeData) {
                        uint16_t val = ((uint16_t)pEvent->pdata[idx] << 8) +
                                       ((uint16_t)pEvent->pdata[idx + 1]);
                        strResult += vscp_str_format("%04X", val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.int32[n] escape
            else if (vscp_startsWith(str, "%event.data.int32[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 3) < pEvent->sizeData) {
                        long val = ((long)pEvent->pdata[idx] << 24) +
                                   ((long)pEvent->pdata[idx + 1] << 16) +
                                   ((long)pEvent->pdata[idx + 2] << 8) +
                                   ((long)pEvent->pdata[idx + 3]);
                        strResult += vscp_str_format("%ld", val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.uint32[n] escape
            else if (vscp_startsWith(str, "%event.data.uint32[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 3) < pEvent->sizeData) {
                        uint32_t val =
                          ((uint32_t)pEvent->pdata[idx] << 24) +
                          ((uint32_t)pEvent->pdata[idx + 1] << 16) +
                          ((uint32_t)pEvent->pdata[idx + 2] << 8) +
                          ((uint32_t)pEvent->pdata[idx + 3]);
                        strResult +=
                          vscp_str_format("%lu", (unsigned long)val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for hexdata.uint32[n] escape
            else if (vscp_startsWith(str, "%event.hexdata.uint32[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 3) < pEvent->sizeData) {
                        uint32_t val =
                          ((uint32_t)pEvent->pdata[idx] << 24) +
                          ((uint32_t)pEvent->pdata[idx + 1] << 16) +
                          ((uint32_t)pEvent->pdata[idx + 2] << 8) +
                          ((uint32_t)pEvent->pdata[idx + 3]);
                        strResult += vscp_str_format("%08X", val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.float[n] escape
            else if (vscp_startsWith(str, "%event.data.float[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 3) < pEvent->sizeData) {
                        float val = *((float *)(pEvent->pdata + idx));
                        strResult += vscp_str_format("%f", val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for data.double[n] escape
            else if (vscp_startsWith(str, "%event.data.double[", &str)) {
                // Must be data
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str = str;
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }

                    long idx = 0;
                    idx      = vscp_readStringValue(str);
                    if ((idx + 7) < pEvent->sizeData) {
                        double val = *((double *)(pEvent->pdata + idx));
                        strResult += vscp_str_format("%lf", val);
                    } else {
                        strResult += "?"; // invalid index
                    }
                } else {
                    // Just remove ending ]
                    if (str.npos != (pos = str.find(']'))) {
                        str = str.substr(str.length() - pos - 1);
                    }
                    strResult += ""; // No data
                }
            }
            // Check for sizedata escape
            else if (vscp_startsWith(str, "%event.sizedata", &str)) {
                strResult += vscp_str_format("%d", pEvent->sizeData);
            }
            // Check for data escape
            else if (vscp_startsWith(str, "%event.data", &str)) {
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    std::string str;
                    vscp_writeDataToString(pEvent, str, false);
                    strResult += str;
                } else {
                    strResult += "empty"; // No data
                }
            }
            // Check for guid escape
            else if (vscp_startsWith(str, "%event.guid", &str)) {
                std::string strGUID;
                vscp_writeGuidToString(pEvent, strGUID);
                strResult += strGUID;
            }
            // Check for nickname escape
            else if (vscp_startsWith(str, "%event.nickname", &str)) {
                strResult +=
                  vscp_str_format(("%d"), pEvent->GUID[VSCP_GUID_LSB]);
            }
            // Check for obid escape
            else if (vscp_startsWith(str, "%event.obid", &str)) {
                strResult += vscp_str_format("%d", pEvent->obid);
            }
            // Check for timestamp escape
            else if (vscp_startsWith(str, "%event.timestamp", &str)) {
                strResult += vscp_str_format("%d", pEvent->timestamp);
            }
            // Check for event index escape
            else if (vscp_startsWith(str, "%event.index", &str)) {
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    strResult += vscp_str_format(("%d"), pEvent->pdata[0]);
                } else {
                    strResult += ("empty"); // No data
                }
            }
            // Check for event zone escape
            else if (vscp_startsWith(str, "%event.zone", &str)) {
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    strResult += vscp_str_format(("%d"), pEvent->pdata[1]);
                } else {
                    strResult += ("empty"); // No data
                }
            }
            // Check for event subzone escape
            else if (vscp_startsWith(str, "%event.subzone", &str)) {
                if (pEvent->sizeData && (NULL != pEvent->pdata)) {
                    strResult += vscp_str_format(("%d"), pEvent->pdata[2]);
                } else {
                    strResult += ("empty"); // No data
                }
            }
            // Check for event escape
            else if (vscp_startsWith(str, "%event", &str)) {
                std::string strEvent;
                vscp_convertEventExToEvent(pEvent, strEvent);
                strResult += strEvent;
            }
            // Check for isodate escape
            else if (vscp_startsWith(str, "%isodate", &str)) {
                strResult += vscpdatetime::Now().getISODate();
            }
            // Check for isotime escape
            else if (vscp_startsWith(str, "%isotime", &str)) {
                strResult += vscpdatetime::Now().getISOTime();
            }
            // Check for isobothms escape
            else if (vscp_startsWith(str, "%isobothms", &str)) {
                // Milliseconds
                long ms; // Milliseconds
#ifdef WIN32
                SYSTEMTIME st;
                getSystemTime(&st);
                ms = st.wMilliseconds;
#else
                time_t s; // Seconds
                struct timespec spec;
                clock_gettime(CLOCK_REALTIME, &spec);
                s  = spec.tv_sec;
                ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to
                                                  // milliseconds
#endif
                // int ms =
                // vscpdatetime::Now().getMillisecond();
                // !!!!! Does not work  !!!!!
                std::string msstr = vscp_str_format(".%ld", ms);
                strResult += vscpdatetime::Now().getISODate();
                strResult += "T";
                strResult += vscpdatetime::Now().getISOTime();
                strResult += msstr;
            }
            // Check for isoboth escape
            else if (vscp_startsWith(str, "%isoboth", &str)) {
                strResult += vscpdatetime::Now().getISODate();
                strResult += "T";
                strResult += vscpdatetime::Now().getISOTime();
            }
            // Check for mstime escape
            else if (vscp_startsWith(str, "%mstime", &str)) {
                uint64_t now = time(NULL); // get current time
                strResult += vscp_str_format("%llu", now);
            }
            // Check for unixtime escape
            else if (vscp_startsWith(str, "%unixtime", &str)) {
                uint64_t now = time(NULL); // get current time
                strResult += vscp_str_format("%llu", now);
            }
            // Check for hour escape
            else if (vscp_startsWith(str, "%hour", &str)) {
                strResult +=
                  vscp_str_format("%d", vscpdatetime::Now().getHour());
            }
            // Check for minute escape
            else if (vscp_startsWith(str, "%minute", &str)) {
                strResult +=
                  vscp_str_format("%d", vscpdatetime::Now().getMinute());
            }
            // Check for second escape
            else if (vscp_startsWith(str, "%second", &str)) {
                strResult +=
                  vscp_str_format("%d", vscpdatetime::Now().getSecond());
            }
            // Check for week0 escape
            else if (vscp_startsWith(str, "%week0", &str)) {
                strResult += vscp_str_format(
                  "%d", vscpdatetime::Now().getWeekNumber());
            }
            // Check for week1 escape
            else if (vscp_startsWith(str, "%week1", &str)) {
                strResult +=
                  vscp_str_format("%d", // TODO FIX!
                                     vscpdatetime::Now().getWeekNumber());
            }
            // Check for weekdaytxtfull escape
            else if (vscp_startsWith(str, "%weekdaytxtfull", &str)) {
                strResult += vscpdatetime::getWeekDayName(
                  vscpdatetime::Now().getWeekDay(), vscpdatetime::name_Full);
            }
            // Check for weekdaytxt escape
            else if (vscp_startsWith(str, "%weekdaytxt", &str)) {
                strResult += vscpdatetime::getWeekDayName(
                  vscpdatetime::Now().getWeekDay(), vscpdatetime::name_Abbr);
            }
            // Check for monthtxtfull escape
            else if (vscp_startsWith(str, "%monthtxtfull", &str)) {
                strResult += vscpdatetime::getMonthName(
                  vscpdatetime::Now().getMonth(), vscpdatetime::name_Full);
            }
            // Check for monthtxt escape
            else if (vscp_startsWith(str, "%monthtxt", &str)) {
                strResult += vscpdatetime::getMonthName(
                  vscpdatetime::Now().getMonth(), vscpdatetime::name_Abbr);
            }
            // Check for month escape
            else if (vscp_startsWith(str, "%month", &str)) {
                strResult += vscp_str_format(
                  "%d", vscpdatetime::Now().getMonth() + 1);
            }
            // Check for year escape
            else if (vscp_startsWith(str, "%year", &str)) {
                strResult +=
                  vscp_str_format("%d", vscpdatetime::Now().getYear());
            }
            // Check for quarter escape
            else if (vscp_startsWith(str, "%quarter", &str)) {
                if (vscpdatetime::Now().getMonth() < 4) {
                    strResult += "1";
                } else if (vscpdatetime::Now().getMonth() < 7) {
                    strResult += "2";
                } else if (vscpdatetime::Now().getMonth() < 10) {
                    strResult += "3";
                } else {
                    strResult += "1";
                }
            }
            // Check for path_config escape
            else if (vscp_startsWith(str, "%path.config", &str)) {
                strResult += "/etc";
            }
            // Check for path_datadir escape
            else if (vscp_startsWith(str, "%path.datadir", &str)) {
                strResult += "/usr/share/vscp/"; // TODO prefix....
            }
            // Check for path_documentsdir escape
            else if (vscp_startsWith(str, "%path.documentsdir", &str)) {
                strResult += "~";
            }
            // Check for path_executable escape
            else if (vscp_startsWith(str, "%path.executable", &str)) {
                strResult += "/usr/bin/vscpd"; // TODO
            }
            // Check for path_localdatadir escape
            else if (vscp_startsWith(str, "%path.localdatadir", &str)) {
                strResult += "/etc/vscp"; // TODO
            }
            // Check for path_pluginsdir escape
            else if (vscp_startsWith(str, "%path.pluginsdir", &str)) {
                strResult += "/usr/lib/vscp";
            }
            // Check for path_resourcedir escape
            else if (vscp_startsWith(str, "%path.resourcedir", &str)) {
                strResult += "/usr/share/vscp";
            }
            // Check for path_tempdir escape
            else if (vscp_startsWith(str, "%path.tempdir", &str)) {
                strResult += "/tmp";
            }
            // Check for path_userconfigdir escape
            else if (vscp_startsWith(str, "%path.userconfigdir", &str)) {
                strResult += "~";
            }
            // Check for path_userdatadir escape
            else if (vscp_startsWith(str, "%path.userdatadir", &str)) {
                strResult += "~";
            }
            // Check for path_localdatadir escape
            else if (vscp_startsWith(str, "%path.localdatadir", &str)) {
                strResult += "~";
            }
            // Check for toliveafter
            else if (vscp_startsWith(str, "%toliveafter1", &str)) {
                strResult += "Carpe diem quam minimum credula postero.";
            }
            // Check for toliveafter
            else if (vscp_startsWith(str, "%toliveafter2", &str)) {
                strResult += "Be Hungry - Stay Foolish.";
            }
            // Check for toliveafter
            else if (vscp_startsWith(str, "%toliveafter3", &str)) {
                strResult += "Be Foolish - Stay Hungry.";
            }
            // Check for measurement.index escape
            else if (vscp_startsWith(str, "%measurement.index", &str)) {
                uint8_t data_coding_byte =
                  vscp_getMeasurementDataCoding(pEvent);
                if (-1 != data_coding_byte) {
                    strResult += vscp_str_format(
                      "%d", VSCP_DATACODING_INDEX(data_coding_byte));
                }
            }
            // Check for measurement.unit escape
            else if (vscp_startsWith(str, "%measurement.unit", &str)) {
                uint8_t data_coding_byte =
                  vscp_getMeasurementDataCoding(pEvent);
                if (-1 != data_coding_byte) {
                    strResult += vscp_str_format(
                      "%d", VSCP_DATACODING_UNIT(data_coding_byte));
                }
            }
            // Check for measurement.coding escape
            else if (vscp_startsWith(str, "%measurement.coding", &str)) {
                uint8_t data_coding_byte =
                  vscp_getMeasurementDataCoding(pEvent);
                if (-1 != data_coding_byte) {
                    strResult += vscp_str_format(
                      "%d", VSCP_DATACODING_TYPE(data_coding_byte));
                }
            }
            // Check for measurement.float escape
            else if (vscp_startsWith(str, "%measurement.float", &str)) {
                std::string str;
                vscp_getMeasurementAsString(pEvent, str);
                strResult += str;
            }
            // Check for measurement.string escape
            else if (vscp_startsWith(str, "%measurement.string", &str)) {
                std::string str;
                vscp_getMeasurementAsString(pEvent, str);
                strResult += str;
            }
            // Check for measurement.convert.data escape
            else if (vscp_startsWith(str, "%measurement.convert.data", &str)) {
                std::string str;
                if (vscp_getMeasurementAsString(pEvent, str)) {
                    for (unsigned int i = 0; i < str.length(); i++) {
                        if (0 != i) {
                            strResult += ','; // Not at find
                                              // location
                        }
                        strResult += str.at(i);
                    }
                }
            }

            // Check for eventdata.realtext escape
            //else if (vscp_startsWith(str, "%eventdata.realtext", &str)) {
            //    strResult += vscp_getRealTextData(pEvent);
            //}

            // Check for %variable:[name] (name is name of variable)
            else if (vscp_startsWith(str, "%variable:[", &str)) {
                vscp_trim(str); // Trim of leading white space
                if (str.npos != (pos = str.find(']'))) {
                    CVariable variable;

                    std::string variableName = str.substr(pos);
                    str = str.substr(str.length() - pos - 1);

                    // Assign value if variable exist
                    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
                    if (gpobj->m_variables.find(variableName, pAdminUser, variable)) {
                        std::string strwrk;
                        variable.writeValueToString(strwrk, true);
                        strResult += strwrk;
                    }
                }
            }

            // Check for %variable:[name] (name is name of variable)
            else if (vscp_startsWith(str, "%vardecode:[", &str)) {
                vscp_trim(str); // Trim of leading white space
                if (str.npos != (pos = str.find(']'))) {
                    CVariable variable;

                    std::string variableName = str.substr(pos);
                    str = str.substr(str.length() - pos - 1);

                    // Assign value if variable exist
                    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
                    if (gpobj->m_variables.find(variableName, pAdminUser, variable)) {
                        std::string strwrk;
                        variable.writeValueToString(strwrk);
                        strResult += strwrk;
                    }
                }
            }

            // Check for %file:[path] (path is path to file to
            // include)
            else if (vscp_startsWith(str, "%file:[", &str)) {
                vscp_trim(str); // Trim of leading white space
                if (str.npos != (pos = str.find(']'))) {
                    std::string path = str.substr(pos);
                    str              = str.substr(str.length() - pos - 1);

                    if (!vscp_fileExists(path.c_str())) {
                        syslog(LOG_ERR,
                               "[DM ]File does not "
                               "exist. Path=%s",
                               path.c_str());
                    } else {
                        FILE *fp;
                        fp = fopen(path.c_str(), "r");
                        if (NULL == fp) {
                            syslog(LOG_ERR,
                                   "Failed to open "
                                   "configuration "
                                   "file [%s]",
                                   path.c_str());
                        } else {
                            char buf[32000];
                            memset(buf, 0, sizeof(buf));

                            size_t size = 0;
                            size =
                              fread(buf, sizeof(char), sizeof(buf) - 1, fp);
                            strResult += std::string(buf, size);

                            fclose(fp);
                        }
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////
            //                  VSCP Stock variables
            ////////////////////////////////////////////////////////////////////

            else if (vscp_startsWith(str, "%vscp.version.major", &str)) {
                strResult += vscp_str_format("%d", VSCPD_MAJOR_VERSION);
            } else if (vscp_startsWith(str, "%vscp.version.minor", &str)) {
                strResult += vscp_str_format("%d", VSCPD_MINOR_VERSION);
            } else if (vscp_startsWith(str, "%vscp.version.sub", &str)) {
                strResult += vscp_str_format("%d", VSCPD_RELEASE_VERSION);
            } else if (vscp_startsWith(str, "%vscp.version.build", &str)) {
                strResult += vscp_str_format("%d", VSCPD_BUILD_VERSION);
            } else if (vscp_startsWith(str, "%vscp.version.str", &str)) {
                strResult += vscp_str_format("%s", VSCPD_DISPLAY_VERSION);
            } else if (vscp_startsWith(str, "%vscp.copysubstr", &str)) {
                strResult += vscp_str_format("%s", VSCPD_COPYRIGHT);
            } else if (vscp_startsWith(str, "%vscp.copysubstr.vscp", &str)) {
                strResult += vscp_str_format("%s", VSCPD_COPYRIGHT);
            } else if (vscp_startsWith(str, "%vscp.os.str", &str)) {
                // strResult += xxGetOsDescription(); TODO
                strResult += "TODO";
            } else if (vscp_startsWith(str, "%vscp.os.width", &str)) {
                if (1) { // TODO
                    strResult += "64-bit ";
                } else {
                    strResult += "32-bit ";
                }
                strResult += "TODO";
            } else if (vscp_startsWith(str, "%vscp.os.endian", &str)) {
                if (vscp_isLittleEndian()) {
                    strResult += "Little endian ";
                } else {
                    strResult += "Big endian ";
                }
            } else if (vscp_startsWith(str, "%vscp.memory.free", &str)) {
                // xxMemorySize memsize;  TODO
                // strResult += memsize.ToString();
                strResult += "TODO";
            } else if (vscp_startsWith(str, "%vscp.host.fullname", &str)) {
                // strResult += xxGetFullHostName(); TODO
                strResult += "TODO";
            }
            // ****************  ESCAPE WAS NOT FOUND
            // ****************
            else {
                // Move beyond the '%'
                strResult += "%";
                str = str.substr(str.length() - 1);
            }
        }
    }

    // Add last part of working string if any.
    strResult += str;
    str = strResult;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// DoAction
//

bool
dmElement::doAction(vscpEvent *pEvent)
{
    std::string logStr;
    std::string actionTokenStr;

    // Leave here for test of escapes
    // handleEscapes( pEvent, std::string( "This is a test %class %type
    // [%sizedata] %data  %data[0] %data[90]" ) );

    m_triggCounter++;

    // Must be a valid event
    if (NULL == pEvent) return false;

    switch (m_actionCode) {

        case VSCP_DAEMON_ACTION_CODE_EXECUTE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE";

            doActionExecute(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionExecuteConditional(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENT:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionSendEvent(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_TO_REMOTE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionSendEventRemote(pEvent, false);
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENT_CONDITIONAL:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionSendEventConditional(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_SEND_EVENTS_FROM_FILE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionSendEventsFromFile(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_STORE_VARIABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionStoreVariable(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_ADD_VARIABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionAddVariable(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_SUBTRACT_VARIABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionSubtractVariable(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_MULTIPLY_VARIABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionMultiplyVariable(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_DIVIDE_VARIABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionDivideVariable(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_TRUE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionCheckVariable(pEvent, VARIABLE_CHECK_SET_TRUE);
            break;

        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE_FALSE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionCheckVariable(pEvent, VARIABLE_CHECK_SET_FALSE);
            break;

        case VSCP_DAEMON_ACTION_CODE_CHECK_VARIABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionCheckVariable(pEvent, VARIABLE_CHECK_SET_OUTCOME);
            break;

        case VSCP_DAEMON_ACTION_CODE_CHECK_MEASUREMENT:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionCheckMeasurement(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_STORE_MIN:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionStoreMin(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_STORE_MAX:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";

            doActionStoreMax(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_START_TIMER:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionStartTimer(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_PAUSE_TIMER:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionPauseTimer(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_STOP_TIMER:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionStopTimer(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_RESUME_TIMER:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionResumeTimer(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_WRITE_FILE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionWriteFile(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_GET_PUT_POST_URL:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            doActionGetURL(pEvent);
            break;

        case VSCP_DAEMON_ACTION_CODE_WRITE_TABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            {
                // Write in possible escapes
                std::string strParam = m_actionparam;
                handleEscapes(pEvent, strParam);

                actionThread_Table *pThread =
                  new actionThread_Table(strParam, pEvent);
                if (NULL == pThread) return false;

                /*  TODO
                xxThreadError err;
                if (xxTHREAD_NO_ERROR == (err =
                pThread->Create()))
                {
                    pThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                    if (xxTHREAD_NO_ERROR != (err =
                pThread->Run()))
                    {
                        gpobj->logMsg("[DM] " +
                                          "Unable to run
                actionThread_Table client thread." );
                    }
                }
                else
                {
                    gpobj->logMsg("[DM] " +
                                      "Unable to create
                actionThread_Table client thread." );
                }*/
            }
            break;

        case VSCP_DAEMON_ACTION_CODE_CLEAR_TABLE:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            {
                CVSCPTable *pTable; // Table object
                std::string strErr;

                // Write in possible escapes
                std::string strParam = m_actionparam;
                handleEscapes(pEvent, strParam);

                std::deque<std::string> tokens;
                vscp_split(tokens, strParam, ";");

                if (tokens.empty()) {
                    // Strange action parameter
                    syslog(LOG_ERR,
                           "[DM] [Action] Delete Table: Action parameter "
                           "is not correct. Parameter=%s ",
                           m_actionparam.c_str());
                    break;
                }

                // Get table name
                std::string name = tokens.front();
                tokens.pop_front();

                // Get table object
                if (NULL ==
                    (pTable = gpobj->m_userTableObjects.getTable(name))) {
                    syslog(LOG_ERR,
                           "[DM] [Action] Delete Table: A table with that "
                           "name was not found. Parameter=%s ",
                           m_actionparam.c_str());
                    return NULL;
                }

                if (tokens.empty()) {
                    std::string sql = pTable->getSQLDelete();

                    // Escapes
                    dmElement::handleEscapes(pEvent, sql);

                    // Do the delete
                    if (!pTable->executeSQL(sql)) {
                        syslog(LOG_ERR,
                               "[DM] [Action] Delete Table (internal SQL): "
                               "Failed SQL=%s ",
                               sql.c_str());
                    }

                } else {
                    std::string sql;
                    std::string str = tokens.front();
                    tokens.pop_front();

                    if (!vscp_std_decodeBase64IfNeeded(sql, str)) {

                        syslog(LOG_ERR,
                               "[DM] [Action] Write Table: "
                               "Failed to decode sql string. "
                               "Will continue anyway. sql=%s",
                               str.c_str());
                        sql = str;
                    }

                    // Escapes
                    dmElement::handleEscapes(pEvent, sql);

                    // Do the delete
                    if (!pTable->executeSQL(sql)) {

                        syslog(LOG_ERR,
                               "[DM] [Action] Delete Table (Action parameter "
                               "SQL): Failed SQL=%s ",
                               sql.c_str());
                    }
                }
            }
            break;

        case VSCP_DAEMON_ACTION_CODE_RUN_JAVASCRIPT:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            {
                // If BASE64 encoded then decode
                std::string strParam = m_actionparam;
                vscp_trim(strParam);
                if (vscp_startsWith(strParam, "BASE64:", &strParam)) {
                    // Yes should be decoded
                    vscp_base64_std_decode(strParam);
                    if (0 == strParam.length()) {
                        syslog(LOG_ERR,
                               "[DM] Failed to decode "
                               "BASE64 "
                               "parameter (len=0)");
                        break;
                    }
                }

                // Write in possible escapes
                handleEscapes(pEvent, strParam);

                // TODO
                // actionThread_JavaScript *pThread =
                //  new actionThread_JavaScript(strParam);
                // if (NULL == pThread) return false;

                // vscp_convertEventToEventEx(&pThread->m_feedEvent,
                //                     pEvent); // Save feed event

                /* TODO
                xxThreadError err;
                if (xxTHREAD_NO_ERROR == (err =
                pThread->Create()))
                {
                    pThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                    if (xxTHREAD_NO_ERROR != (err =
                pThread->Run()))
                    {
                        syslog(LOG_ERR, "[DM] Unable to run
                " actionThread_JavaScript client thread." );
                    }
                }
                else
                {
                    syslog(LOG_ERR, "[DM] Unable to create
                actionThread_JavaScript client thread." );
                }
                */
            }
            break;

        case VSCP_DAEMON_ACTION_CODE_RUN_LUASCRIPT:
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            {
                // If BASE64 encoded then decode
                std::string strParam = m_actionparam;
                vscp_trim(strParam);
                if (vscp_startsWith(strParam, "BASE64:", &strParam)) {
                    // Yes should be decoded
                    vscp_base64_std_decode(strParam);
                    if (0 == strParam.length()) {
                        syslog(LOG_ERR,
                               "[DM] Failed to decode "
                               "BASE64 parameter (len=0)");
                        break;
                    }
                }

                // Write in possible escapes
                handleEscapes(pEvent, strParam);

                vscp_trim(strParam);

                // TODO
                // actionThread_Lua *pThread = new actionThread_Lua(strParam);
                // if (NULL == pThread) return false;

                // vscp_convertEventToEventEx(&pThread->m_feedEvent,
                //                     pEvent); // Save feed event

                /* TODO
                xxThreadError err;
                if (xxTHREAD_NO_ERROR == (err =
                pThread->Create()))
                {
                    pThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
                    if (xxTHREAD_NO_ERROR != (err =
                pThread->Run()))
                    {
                        syslog(LOG_ERR, "[DM] Unable to run "
                " actionThread_Lua client thread." );
                    }
                }
                else
                {
                    syslog(LOG_ERR, "[DM] Unable to create ""
                " actionThread_Lua client thread."
                );
                }
                */
            }
            break;

        default:
        case VSCP_DAEMON_ACTION_CODE_NOOP:
            // We do nothing
            actionTokenStr = "VSCP_DAEMON_ACTION_CODE_EXECUTE_CONDITIONAL";
            break;
    }

    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM) {
        std::string strEvent;
        vscp_convertEventExToEvent(pEvent, strEvent);
        syslog(LOG_ERR,
               "[DM] %s - %s - "
               "%s - Event = %s",
               actionTokenStr.c_str(),
               getAsString(false).c_str(),
               logStr.c_str(),
               strEvent.c_str());
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionExecute
//

bool
dmElement::doActionExecute(vscpEvent *pDMEvent, bool bCheckExecutable)
{
    // Write in possible escapes
    std::string str = m_actionparam;
    vscp_trim(str);
    handleEscapes(pDMEvent, str);

    // Check for bCheckExecutable flag
    if (str[0] == '!') {
        str              = str.substr(str.length() - 1);
        bCheckExecutable = false;
    }

    std::string strfn = m_actionparam;
    bool bOK          = true;

    if (bCheckExecutable) {
        int pos = m_actionparam.find(' ');
        if (m_actionparam.npos != pos) {
            strfn = m_actionparam.substr(pos);
        }
    }
    /* TODO
    // xxExecute breaks if the path does not exist so we have to
    // check that it does.
    if ( bCheckExecutable &&
            ( !xxFileName::FileExists( strfn ) ||
              !xxFileName::IsFileExecutable( strfn ) ) ) {
        syslog( LOG_ERR, "[DM] Target does not exist or is not executable" );
        bOK = false;
    }

    //std::string cdir = xxGetCwd();
    //bool rv = xxSetWorkingDirectory("c:\\programdata\\vscp\\actions");
  #ifdef WIN32
    if ( bOK && ( ::xxExecute(str, xxEXEC_ASYNC | xxEXEC_HIDE_CONSOLE ) ) )
  { #else
    //if ( bOK && ( ::xxExecute(str, xxEXEC_ASYNC  ) ) ) {
    if ( unixVSCPExecute( str ) ) {
  #endif
        if ( gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM ) {
            syslog( LOG_ERR, "[DM] [Action] Executed:", m_actionparam.c_str() );
        }
    }
    else {
        // Failed to execute
        m_errorCounter++;
        if ( bOK ) {
            m_strLastError = "[Action] Failed to execute :";
            syslog(  LOG_ERR, "[DM] "
                           "[Action] Failed to execute " );
        }
        else {
            m_strLastError = "File does not exists or is not an executable
  :";
   syslog(  LOG_ERR, "[DM] File does not exists or is not an
  executable %s", m_actionparam.c_str()  );
        }

        return false;
    }
  */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionExecuteConditional
//

bool
dmElement::doActionExecuteConditional(vscpEvent *pDMEvent,
                                      bool bCheckExecutable)
{
    bool bTrigger;
    std::string varName;

    // Write in possible escapes
    std::string escaped_actionparam = m_actionparam;
    handleEscapes(pDMEvent, escaped_actionparam);

    std::deque<std::string> tokens;
    vscp_split(tokens, escaped_actionparam, ";");

    // Handle variable
    if (!tokens.empty()) {
        std::string varname = tokens.front();
        tokens.pop_front();

        CVariable variable;
        CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
        if (!gpobj->m_variables.find(varname, pAdminUser, variable)) {
            // Variable was not found - create it
            if (!gpobj->m_variables.add(
                  varname, "false", VSCP_DAEMON_VARIABLE_CODE_BOOLEAN)) {

                syslog(LOG_ERR,
                       "[DM] [Action] Conditional send event: Variable "
                       "[%s] "
                       "not defined. Failed to create it.",
                       (const char *)varName.c_str());
                return false;
            }

            // Get the variable
            if (!gpobj->m_variables.find(varName, pAdminUser, variable)) {

                // Well should not happen - but in case report it...

                syslog(LOG_ERR,
                       "[DM] [Action] Conditional execute: Variable "
                       "[%s] "
                       "was not found (but was created).",
                       varName.c_str());

                return false;
            }
        }

        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != variable.getType()) {
            // must be a variable
            syslog(LOG_ERR,
                   "[DM] [Action] Conditional execute: "
                   "Condition variable must be boolean Param = %s",
                   escaped_actionparam.c_str());
            return false;
        }

        // Get the value
        variable.getValue(&bTrigger);

        // if the variable is false we should do nothing
        if (!bTrigger) return false;
    } else {
        // must be a condition variable
        syslog(
          LOG_ERR,
          "[DM] [Action] Conditional execute: No variable defined. Param = %s",
          escaped_actionparam.c_str());
        return false;
    }

    std::string str;

    // Handle variable
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
    }

    // Check for bCheckExecutable flag
    if (str[0] == '!') {
        str              = str.substr(str.length() - 1);
        bCheckExecutable = false;
    }

    std::string strfn = str;
    bool bOK          = true;

    if (bCheckExecutable) {
        int pos = str.find(' ');
        if (str.npos != pos) {
            strfn = str.substr(pos);
        }
    }
    /* TODO
    // xxExecute breaks if the path does not exist so we have to
    // check that it does.
    if ( bCheckExecutable &&
            ( !xxFileName::FileExists( strfn ) ||
              !xxFileName::IsFileExecutable( strfn ) ) ) {
        syslog(  LOG_ERR, "[DM] Conditional execute: Target does not exist
  or is not executable" );
        bOK = false;
    }

    //std::string cdir = xxGetCwd();
    //bool rv = xxSetWorkingDirectory("c:\\programdata\\vscp\\actions");
  #ifdef WIN32
    if ( bOK && ( ::xxExecute(str, xxEXEC_ASYNC | xxEXEC_HIDE_CONSOLE ) ) )
  { #else
    //if ( bOK && ( ::xxExecute(str, xxEXEC_ASYNC  ) ) ) {
    if ( unixVSCPExecute( str ) ) {
  #endif
        if ( gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM ) {
            syslog(  LOG_ERR, "[DM] [Action] Conditional Executed: ",
  m_actionparam.c_str()  );
        }
    }
    else {
        // Failed to execute
        m_errorCounter++;
        if ( bOK ) {
            m_strLastError = "";
            syslog(  LOG_ERR, "[DM] "
                           "[Action] Failed to conditional execute : " );
        }
        else {
            m_strLastError = "File does not exists or is not an executable
  :"; syslog(  LOG_ERR, "[DM] Conditional execute: " + "File does not exists
  or is not an executable " );
        }

        m_strLastError += m_actionparam;
        m_strLastError += "";
        syslog(  LOG_ERR, "[DM] " + str  );
        return false;
    }
  */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// unixVSCPExecute
//
// http://stackoverflow.com/questions/21558937/i-do-not-understand-how-execlp-work-in-linux
// http://stackoverflow.com/questions/6718272/c-exec-fork-defunct-processes
// http://beej.us/guide/bgipc/output/html/multipage/fork.html
// https://www.gidforums.com/t-3369.html
//

bool
dmElement::unixVSCPExecute(std::string &argExec)
{
    int pos;
    int cntArgs;
    std::string strPath;
    std::string strExe;
    std::deque<std::string> listargs;
    char **args;

    /*
    xxCmdLineParser cmdParser;
    listargs = cmdParser.ConvertStringToArgs( argExec, xxCMD_LINE_SPLIT_UNIX
    );
    */
    cntArgs = listargs.size();

    if (!cntArgs) return false; // Must at least have an exec file

    // find parameter is executable
    strPath = listargs[0];

    // If it contains a path we need to pick out the executable name
    if (strPath.npos != (pos = strPath.find('/'))) {
        strExe      = strPath.substr(strPath.length() - pos - 1);
        listargs[0] = strExe;
    } else {
        strExe = strPath;
    }

    pid_t pid = fork();

    if (pid == 0) {
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
        open("/dev/null", O_RDONLY); // stdin
        open("/dev/null", O_WRONLY); // stdout
        open("/dev/null", O_WRONLY); // stderr

        // Create the args
        args = (char **)malloc((cntArgs + 1) * sizeof(char *));
        for (int i = 0; i < cntArgs; i++) {
            args[i] = (char *)malloc(listargs[i].length() + 1);
            strcpy(args[i], (const char *)listargs[i].c_str());
        }

        // Last is NULL pointer
        args[cntArgs] = NULL;

        int rc = execvp((const char *)strPath.c_str(), args);

        // Here only if execvp fails

        // free allocated resources before terminating
        for (int i = 0; i < cntArgs; i++) {
            free(args[i]);
            args[i] = NULL;
        }

        free(args);

        // Terminate
        exit(rc);
    } else if (-1 == pid) {
        // TODO args.clear();

        // fork error
        return false;
    } else {
        // TODO args.clear();

        // OK
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////
// doActionSendEvent
//

bool
dmElement::doActionSendEvent(vscpEvent *pDMEvent)
{
    // int idx;
    std::string str;
    std::string strEvent;
    std::string varName;

    // Write in possible escapes
    std::string escaped_actionparam = m_actionparam;
    handleEscapes(pDMEvent, escaped_actionparam);

    std::deque<std::string> tokens;
    vscp_split(tokens, escaped_actionparam, ";");

    // Get event
    if (!tokens.empty()) {
        strEvent = tokens.front();
        tokens.pop_front();
    } else {
        // must be an event
        syslog(LOG_ERR,
               "[DM] [Action] Send event: No event defined. Param = %s",
               str.c_str());
        return false;
    }

    // Get confirmation variable (if any)
    if (!tokens.empty()) {
        varName = tokens.front();
        tokens.pop_front();
    }

    // There must be room in the send queue
    if (gpobj->m_maxItemsInClientReceiveQueue >
        gpobj->m_clientOutputQueue.size()) {
        vscpEvent *pEvent = new vscpEvent;

        if (NULL != pEvent) {
            pEvent->pdata = NULL;

            if (!vscp_convertStringToEvent(pEvent, strEvent)) {
                
                vscp_deleteEvent_v2(&pEvent);

                // Event has wrong format
                syslog(LOG_ERR,
                       "[DM] [Action] Send event: Invalid format for "
                       "event. Param = %s",
                       str.c_str());
                return false;
            }

            pthread_mutex_lock(&gpobj->m_mutex_ClientOutputQueue);
            gpobj->m_clientOutputQueue.push_back(pEvent);
            sem_post(&gpobj->m_semClientOutputQueue);
            pthread_mutex_unlock(&gpobj->m_mutex_ClientOutputQueue);

            // TX Statistics
            m_pDM->m_pClientItem->m_statistics.cntTransmitData +=
              pEvent->sizeData;
            m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

            // Set the condition variable to false if it is defined
            if (varName.length()) {
                CVariable variable;
                CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
                if (!gpobj->m_variables.find(varName, pAdminUser, variable)) {
                    // Variable was not found - create it
                    if (!gpobj->m_variables.add(
                          varName,
                          "false",
                          VSCP_DAEMON_VARIABLE_CODE_BOOLEAN)) {

                        syslog(LOG_ERR,
                               "[DM] [Action] Send event: "
                               "Variable [%s] "
                               "not defined. Failed "
                               "to create it.",
                               (const char *)varName.c_str());
                        return false;
                    }

                    // Get the variable
                    if (!gpobj->m_variables.find(varName, pAdminUser, variable)) {
                        // Well should not happen - but
                        // in case...
                        syslog(LOG_ERR,
                               "[DM] [Action] Check "
                               "measurement: Variable "
                               "[%s] "
                               "was not found (but "
                               "was created "
                               "alsubstr).",
                               (const char *)varName.c_str());
                        return false;
                    }
                }

                // Set it to true
                variable.setValue(true);

                // Update the variable
                if (!gpobj->m_variables.update(variable,pAdminUser)) {
                    syslog(LOG_ERR,
                           "[DM] [Action] Send event: Failed to "
                           "update variable. [%s]",
                           str.c_str());
                    return false;
                }
            }
        }
    } else {
        m_pDM->m_pClientItem->m_statistics.cntOverruns++;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionSendEventConditional
//

bool
dmElement::doActionSendEventConditional(vscpEvent *pDMEvent)
{
    bool bTrigger = false;
    std::string varName;

    // Write in possible escapes
    std::string escaped_actionparam = m_actionparam;
    handleEscapes(pDMEvent, escaped_actionparam);

    std::deque<std::string> tokens;
    vscp_split(tokens, escaped_actionparam, ";");

    // Handle variable
    if (!tokens.empty()) {
        std::string varname = tokens.front();
        tokens.pop_front();

        CVariable variable;
        CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
        if (!gpobj->m_variables.find(varname, pAdminUser, variable)) {

            // Variable was not found - create it

            if (!gpobj->m_variables.add(
                  varname, "false", VSCP_DAEMON_VARIABLE_CODE_BOOLEAN)) {
                syslog(LOG_ERR,
                       "[DM] [Action] Conditional send event: Variable "
                       "[%s] "
                       "not defined. Failed to create it.",
                       (const char *)varName.c_str());
                return false;
            }

            // Get the variable
            if (!gpobj->m_variables.find(varName, pAdminUser, variable)) {

                // Well should not happen - but in case...

                syslog(LOG_ERR,
                       "[DM] [Action] Check measurement: Variable [%s] "
                       "was not found (but was created alsubstr).",
                       (const char *)varName.c_str());
                return false;
            }
        }

        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != variable.getType()) {

            // must be a variable

            syslog(LOG_ERR,
                   "[DM] [Action] Conditional send event: "
                   "Condition variable must be boolean [%s]",
                   escaped_actionparam.c_str());
            return false;
        }

        // Get the value
        variable.getValue(&bTrigger);

        // if the variable is false we should do nothing
        if (!bTrigger) return false;
    } else {
        // must be a condition variable

        syslog(LOG_ERR,
               "[DM] [Action] Conditional send event: No variable defined %s",
               escaped_actionparam.c_str());
        return false;
    }

    vscpEvent *pEvent = new vscpEvent;
    pEvent->pdata     = NULL;

    // We must have an event to send
    if (!tokens.empty()) {
        std::string strEvent = tokens.front();
        tokens.pop_front();

        if (!vscp_convertStringToEvent(pEvent, strEvent)) {
            vscp_deleteEvent_v2(&pEvent);

            // Could not parse event string
            syslog(LOG_ERR,
                   "[DM] "
                   "[Action] Conditional send event: Unable to parse "
                   "event %s",
                   escaped_actionparam.c_str());
            return false;
        }
    } else {
        // must be an event
        syslog(LOG_ERR,
               "[DM] "
               "[Action] Conditional send event: No event defined %s",
               escaped_actionparam.c_str());
        return false;
    }

    // Get confirmation variable (if any)
    if (!tokens.empty()) {
        varName = tokens.front();
        tokens.pop_front();
    }

    // There must be room in the send queue
    if (gpobj->m_maxItemsInClientReceiveQueue >
        gpobj->m_clientOutputQueue.size()) {
        pthread_mutex_lock(&gpobj->m_mutex_ClientOutputQueue);
        gpobj->m_clientOutputQueue.push_back(pEvent);
        sem_post(&gpobj->m_semClientOutputQueue);
        pthread_mutex_unlock(&gpobj->m_mutex_ClientOutputQueue);

        // TX Statistics
        m_pDM->m_pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
        m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;

        // Set the condition variable to false if it is defined
        if (varName.length()) {
            CVariable variable;
            CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
            if (gpobj->m_variables.find(varName, pAdminUser, variable)) {
                // Set it to true
                variable.setValue(true);

                // Update the variable
                if (!gpobj->m_variables.update(variable,pAdminUser)) {

                    syslog(LOG_ERR,
                           "[DM] "
                           "[Action] Send event: Failed to "
                           "update "
                           "variable. param = %s",
                           escaped_actionparam.c_str());
                    return false;
                }
            } else {

                syslog(LOG_ERR,
                       "[DM] "
                       "[Action] Send event: Confirmation "
                       "variable "
                       "was not found. Param = %s",
                       escaped_actionparam.c_str());
                return false;
            }
        }
    } else {
        vscp_deleteEvent_v2(&pEvent);
        m_pDM->m_pClientItem->m_statistics.cntOverruns++;
    }

    return true;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// doActionSendEventsFromFile
//

bool
dmElement::doActionSendEventsFromFile(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);
    /* TODO
    // File must exist
    if (!xxFile::Exists(str))
    {

        syslog(LOG_ERR, "[DM] " "[Action] Send event from file: Non existent
    file  Oaram=%s", str.c_str() ); return false;
    }

    xxXmlDocument doc;
    if (!doc.Load(str))
    {

        syslog(LOG_ERR, "[DM] "[Action] Send event from file: Failed to load "
            "event XML file  Param = %s", str.c_str());
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != ("events"))
    {

        syslog(LOG_ERR, "[DM] " "[Action] Send event from file: "
                             "<events> tag is missing. Param = %s", str.c_str()
    ); return false;
    }

    xxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child)
    {

        if (child->GetName() == ("event"))
        {

            // Set defaults for event
            vscpEvent *pEvent = new vscpEvent;

            if (NULL != pEvent)
            {

                pEvent->head = 0;
                pEvent->obid = 0;
                pEvent->timestamp = 0;
                vscp_setEventDateTimeBlockToNow(pEvent);
                pEvent->sizeData = 0;
                pEvent->pdata = NULL;
                pEvent->vscp_class = 0;
                pEvent->vscp_type = 0;
                memset(pEvent->GUID, 0, 16);

                xxXmlNode *subchild = child->GetChildren();
                while (subchild)
                {

                    if (subchild->GetName() == ("head"))
                    {
                        pEvent->head =
                            vscp_readStringValue(subchild->GetNodeContent());
                    }
                    else if (subchild->GetName() == ("class"))
                    {
                        pEvent->vscp_class =
                            vscp_readStringValue(subchild->GetNodeContent());
                    }
                    else if (subchild->GetName() == ("type"))
                    {
                        pEvent->vscp_type =
                            vscp_readStringValue(subchild->GetNodeContent());
                    }
                    else if (subchild->GetName() == ("guid"))
                    {
                        vscp_getGuidFromString(pEvent,
                                               subchild->GetNodeContent());
                    }
                    if (subchild->GetName() == ("data"))
                    {
                        vscp_setEventDataFromString(pEvent,
                                                        subchild->GetNodeContent());
                    }

                    subchild = subchild->GetNext();
                }

                // ==============
                // Send the event
                // ==============

                // There must be room in the send queue
                if (gpobj->m_maxItemsInClientReceiveQueue >
                    gpobj->m_clientOutputQueue.size())
                {

                    pthread_mutex_lock( &gpobj->m_mutex_ClientOutputQueue ;
                    gpobj->m_clientOutputQueue.push_back(pEvent);
                    sem_post( gpobj->m_semClientOutputQueue ;
                    pthread_mutex_unlock( &gpobj->m_mutex_ClientOutputQueue );

                    // TX Statistics
                    m_pDM->m_pClientItem->m_statistics.cntTransmitData +=
                        pEvent->sizeData;
                    m_pDM->m_pClientItem->m_statistics.cntTransmitFrames++;
                }
                else
                {

                    // Remove the event
                    vscp_deleteEvent_v2(&pEvent);

                    m_pDM->m_pClientItem->m_statistics.cntOverruns++;
                }
            }
        }

        child = child->GetNext();
    }
  */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionSendEventRemote
//

bool
dmElement::doActionSendEventRemote(vscpEvent *pDMEvent, bool bSecure)
{
    std::string strHostname;
    short port;
    std::string strUsername;
    std::string strPassword;
    std::string strEvent;

    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ";");

    // Get servername
    if (!tokens.empty()) {
        strHostname = tokens.front();
        tokens.pop_front();
    } else {
        return false;
    }

    // Get port
    if (!tokens.empty()) {
        port = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        return false;
    }

    // Get username
    if (!tokens.empty()) {
        strUsername = tokens.front();
        tokens.pop_front();
    } else {
        return false;
    }

    // Get password
    if (!tokens.empty()) {
        strPassword = tokens.front();
        tokens.pop_front();
    } else {
        return false;
    }

    // Get event
    if (!tokens.empty()) {
        strEvent = tokens.front();
        tokens.pop_front();
    } else {
        return false;
    }

    // Go do your work mate
    actionThread_VSCPSrv *thread = new actionThread_VSCPSrv(
      gpobj, strHostname, port, strUsername, strPassword, strEvent);
    if (NULL == thread) return false;

    // Go Go Go
    // thread->Run();  TODO

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionWriteFile
//

bool
dmElement::doActionWriteFile(vscpEvent *pDMEvent)
{
    /* TODO
    xxFile f;
    std::string path;
    std::string strOut;
    bool bAppend = true;

    // Write in possible escapes
    std::string escaped_param = m_actionparam;
    handleEscapes(pDMEvent, escaped_param);

      std::deque<std::string> tokens;
      vscp_split(tokens, escaped_actionparam, ";");

    // Handle path
    if (!tokens.empty())
    {
        path = tokens.front();
        tokens.pop_front();
    }
    else
    {
        // Must have a path
        syslog(LOG_ERR, "[DM] " "[Action] Write to file: No path to file given
    " );
    return false;
    }

    // Handle append/overwrite flag
    if (!tokens.empty())
    {
        if (!vscp_readStringValue(tokens.front()))
        {
            bAppend = false;
        }
        tokens.pop_front();
    }

    // Get string to write
    if (!tokens.empty())
    {
        strOut = tokens.front();
        tokens.pop_front();
    }

    if (f.Open(path, (bAppend ? xxFile::write_append : xxFile::write)))
    {

        f.Write(strOut);
        f.Flush();
        f.Close();
    }
    else
    {
        // Failed to open file
        syslog(LOG_ERR, "[DM] " "[Action] Write to file: Failed to open file
    Path = %s", path.c_str() ); return false;
    }
  */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionGetURL
//

bool
dmElement::doActionGetURL(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ";");

    if (tokens.empty()) {
        // Action parameter is wrong
        syslog(LOG_ERR,
               "[DM] "
               "[Action] Get URL: Wrong action parameter (method;URL "
               "required Param = %s",
               str.c_str());
        return false;
    }

    // Access method
    // actionThread_URL::accessmethod_t nAccessMethod = actionThread_URL::GET;
    std::string access = tokens.front();
    tokens.pop_front();
    vscp_trim(access);
    if (0 == vscp_strcasecmp(access.c_str(), "PUT")) {
        // TODO nAccessMethod = actionThread_URL::PUT;
    } else if (0 == vscp_strcasecmp(access.c_str(), "POST")) {
        // TODO nAccessMethod = actionThread_URL::POST;
    }

    // Get URL
    std::string url;
    if (!tokens.empty()) {
        url = tokens.front();
        tokens.pop_front();
    } else {
        // URL is required
        syslog(LOG_ERR,
               "[DM] [Action] Get URL: Wrong action parameter (URL required) "
               "Param = %s",
               str.c_str());
        return false;
    }

    // Get POST/PUT data
    std::string putdata;
    if (!tokens.empty()) {
        putdata = tokens.front();
        tokens.pop_front();
    }

    // Get extra headers
    std::string extraheaders;
    if (!tokens.empty()) {
        extraheaders = tokens.front();
        tokens.pop_front();
    }

    // Get proxy
    std::string proxy;
    if (!tokens.empty()) {
        proxy = tokens.front();
        tokens.pop_front();
    }

    // Go do your work mate
    // TODO
    // actionThread_URL *thread = new actionThread_URL(
    //  gpobj, url, nAccessMethod, putdata, extraheaders, proxy);
    // if (NULL == thread) {
    //    return false;
    //}

    // Go Go Go
    // thread->Run();  TODO

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStoreVariable
//

bool
dmElement::doActionStoreVariable(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    std::string params = m_actionparam;
    handleEscapes(pDMEvent, params);

    std::deque<std::string> tokens;
    vscp_split(tokens, params, ";");

    CVariable var;

    if (tokens.size() >= 6) {
        // The form is variable-name; variable-type; persistence; value

        if (!(var.setFromString(params))) {
            // must be a variable
            syslog(LOG_ERR,
                   "[DM] "
                   "[Action] Store Variable: Could not set new "
                   "variable Param = %s",
                   params.c_str());
            return false;
        }

        if (!gpobj->m_variables.add(var)) {
            // must be a variable

            syslog(
              LOG_ERR,
              "[DM] "
              "[Action] Store Variable: Could not add variable Params = %s",
              params.c_str());
            return false;
        }
    } else {
        // variablename; type; persistence
        uint16_t varType = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent = false;

        // We must have the name
        if (tokens.empty()) return false;

        std::string varName = tokens.front();
        tokens.pop_front();

        // Variable type
        if (!tokens.empty()) {
            varType = vscp_readStringValue(tokens.front());
            tokens.pop_front();
        }

        // Persistence
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);
            if (0 == vscp_strcasecmp(str.c_str(), "true")) {
                bPersistent = true;
            }
        }

        CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
        if (!gpobj->m_variables.find(varName, pAdminUser, var)) {
            // The variable was not found - it should be added
            var.setName(varName);
            var.setType(varType);
            var.setPersistent(bPersistent);
        }

        // Add/create the variable
        if (!gpobj->m_variables.add(var)) {
            // must be a variable

            syslog(LOG_ERR,
                   "[DM] "
                   "[Action] Store Variable: Could not add variable Param = %s",
                   params.c_str());
            return false;
        }

        switch (var.getType()) {
            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                if (vscp_isMeasurement(pDMEvent)) {
                    std::string strValue;
                    if (vscp_getMeasurementAsString(pDMEvent, strValue)) {
                        var.setValue(strValue);
                    } else {

                        syslog(LOG_ERR,
                               "[DM] "
                               "[Action] Store Variable: "
                               "Failed to convert value "
                               "to string Params = %s",
                               params.c_str());
                        return false;
                    }
                }
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                if (vscp_isMeasurement(pDMEvent)) {
                    double value;
                    if (vscp_getMeasurementAsDouble(pDMEvent, &value)) {
                        var.setValue((int)value);
                    } else {

                        syslog(LOG_ERR,
                               "[DM] "
                               "[Action] Store Variable: "
                               "Failed to convert value "
                               "to double Param = %s",
                               params.c_str());
                        return false;
                    }
                }
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                if (vscp_isMeasurement(pDMEvent)) {
                    double value;
                    if (vscp_getMeasurementAsDouble(pDMEvent, &value)) {
                        var.setValue((long)value);
                    } else {

                        syslog(LOG_ERR,
                               "[DM] "
                               "[Action] Store Variable: "
                               "Failed to convert value "
                               "to double Param = %s",
                               params.c_str());
                        return false;
                    }
                }
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                if (vscp_isMeasurement(pDMEvent)) {
                    double value;
                    if (vscp_getMeasurementAsDouble(pDMEvent, &value)) {
                        var.setValue(value);
                    } else {

                        syslog(LOG_ERR,
                               "[DM] "
                               "[Action] Store Variable: "
                               "Failed to convert value "
                               "to double Params = %s",
                               params.c_str());
                        return false;
                    }
                }
                break;

            case VSCP_DAEMON_VARIABLE_CODE_MEASUREMENT:
                if (vscp_isMeasurement(pDMEvent)) {
                    double value         = 0;
                    uint8_t unit         = 0;
                    uint8_t sensor_index = 0;
                    uint8_t zone         = 0;
                    uint8_t subzone      = 0;

                    if (-1 == (unit = vscp_getMeasurementUnit(pDMEvent))) {
                        unit = 0;
                    }

                    if (-1 ==
                        (sensor_index =
                           vscp_getMeasurementSensorIndex(pDMEvent))) {
                        sensor_index = 0;
                    }

                    if (-1 == (zone = vscp_getMeasurementZone(pDMEvent))) {
                        zone = 0;
                    }

                    if (-1 ==
                        (subzone = vscp_getMeasurementSubZone(pDMEvent))) {
                        subzone = 0;
                    }

                    if (vscp_getMeasurementAsDouble(pDMEvent, &value)) {
                        // (MEASUREMENT|6;true|false;)value;unit;sensor-index;zone;subzone
                        std::string strValue =
                          vscp_str_format("%lf;%d;%d;%d;%d",
                                             value,
                                             unit,
                                             sensor_index,
                                             zone,
                                             subzone);
                        var.setValue(strValue);
                    } else {

                        syslog(LOG_ERR,
                               "[DM] "
                               "[Action] Store Variable: "
                               "Failed to convert value "
                               "to double Param = %s",
                               params.c_str());
                        return false;
                    }
                }
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT: {
                std::string strEvent;
                if (vscp_convertEventExToEvent(pDMEvent, strEvent)) {
                    var.setValue(strEvent);
                }
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_GUID: {
                std::string str;
                cguid guid;
                guid.getFromArray(pDMEvent->GUID);
                guid.toString(str);
                var.setValue(std::string(str));
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_DATA: {
                std::string strData;
                if (vscp_writeDataToString(pDMEvent, strData)) {
                    var.setValue(strData);
                }
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_CLASS:
                var.setValue(pDMEvent->vscp_class);
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_TYPE:
                var.setValue(pDMEvent->vscp_type);
                break;

            case VSCP_DAEMON_VARIABLE_CODE_EVENT_TIMESTAMP:
                var.setValue((double)pDMEvent->timestamp);
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATETIME: {
                vscpdatetime dt = vscpdatetime::Now();
                var.setValue(dt.getISODateTime());
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_DATE: {
                vscpdatetime dt = vscpdatetime::Now();
                var.setValue(dt.getISODate());
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_TIME: {
                vscpdatetime dt = vscpdatetime::Now();
                var.setValue(dt.getISODateTime());
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_HTML: {
                std::string strData;
                vscp_writeDataToString(pDMEvent, strData);

                std::string strGUID;
                vscp_writeGuidToString(pDMEvent, strGUID);

                std::string str =
                  vscp_str_format(VSCP_HTML_EVENT_TEMPLATE,
                                     vscpdatetime::Now().getISODateTime(),
                                     pDMEvent->vscp_class,
                                     pDMEvent->vscp_type,
                                     pDMEvent->sizeData,
                                     (const char *)strData.c_str(),
                                     (const char *)strGUID.c_str(),
                                     pDMEvent->head,
                                     pDMEvent->timestamp,
                                     pDMEvent->obid,
                                     "");
                var.setValue(str);
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_JSON: {
                std::string strData;
                vscp_writeDataToString(pDMEvent, strData);

                std::string strGUID;
                vscp_writeGuidToString(pDMEvent, strGUID);

                std::string str =
                  vscp_str_format(VSCP_JSON_EVENT_TEMPLATE,
                                     vscpdatetime::Now().getISODateTime(),
                                     pDMEvent->head,
                                     pDMEvent->timestamp,
                                     pDMEvent->obid,
                                     pDMEvent->vscp_class,
                                     pDMEvent->vscp_type,
                                     (const char *)strGUID.c_str(),
                                     (const char *)strData.c_str(),
                                     "");
                var.setValue(str);
            } break;

            case VSCP_DAEMON_VARIABLE_CODE_XML: {
                std::string strData;
                vscp_writeDataToString(pDMEvent, strData);

                std::string strGUID;
                vscp_writeGuidToString(pDMEvent, strGUID);

                std::string str =
                  vscp_str_format(VSCP_XML_EVENT_TEMPLATE,
                                     vscpdatetime::Now().getISODateTime(),
                                     pDMEvent->head,
                                     pDMEvent->timestamp,
                                     pDMEvent->obid,
                                     pDMEvent->vscp_class,
                                     pDMEvent->vscp_type,
                                     (const char *)strGUID.c_str(),
                                     pDMEvent->sizeData,
                                     (const char *)strData.c_str());
                var.setValue(str);
            } break;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionAddVariable
//

bool
dmElement::doActionAddVariable(vscpEvent *pDMEvent)
{
    CVariable variable;
    std::string strName;
    long val        = 0;
    double floatval = 0.0;
    size_t pos;

    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    // Get the variable name
    if (str.npos == (pos = str.find(';'))) {
        syslog(LOG_ERR, "[DM] [Action] Add to variable: Format error");
    }

    strName = str.substr(0, pos);

    // Get the value
    std::string strval = vscp_str_after(str, ';'); // TODO check

    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (0 == gpobj->m_variables.find(strName, pAdminUser, variable)) {

        syslog(LOG_ERR,
               "[DM] "
               "[Action] Add to Variable: Variable was not found Param = %s",
               m_actionparam.c_str());
        return false;
    }

    // Must be a numerical variable
    if (!variable.isNumerical()) {

        syslog(LOG_ERR,
               "[DM] "
               "[Action] Add to Variable: Variable is not numerical Path = %s",
               m_actionparam.c_str());
        return false;
    }

    if ((VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType())) {
        long lval;
        val = stol(strval);
        variable.getValue(&lval);
        lval += val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType())) {
        long lval;
        val = stol(strval);
        variable.getValue(&lval);
        lval += val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType())) {
        double dval;
        floatval = stod(strval);
        variable.getValue(&dval);
        dval += floatval;
        variable.setValue(dval);
    }

    if (!gpobj->m_variables.update(variable,pAdminUser)) {
        syslog(LOG_ERR,
               "[DM] "
               "[Action] Add to Variable: Failed to update variable param = %s",
               m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionSubtractVariable
//

bool
dmElement::doActionSubtractVariable(vscpEvent *pDMEvent)
{
    CVariable variable;
    std::string strName;
    long val        = 0;
    double floatval = 0.0;

    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    // Get the variable name
    strName = vscp_str_before(str, ';'); // TODO check

    // Get the value
    std::string strval = vscp_str_after(str, ';'); // TODO check

    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (0 == gpobj->m_variables.find(strName, pAdminUser, variable)) {

        syslog(
          LOG_ERR,
          "[DM] "
          "[Action] Subtract from Variable: Variable was not found Param = %s",
          m_actionparam.c_str());
        return false;
    }

    // Must be a numerical variable
    if (!variable.isNumerical()) {

        syslog(LOG_ERR,
               "[DM] "
               "[Action] Subtract from Variable: Variable is not "
               "numerical Param = %s",
               m_actionparam.c_str());
        return false;
    }

    if ((VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType())) {
        long lval;
        val = stol(strval);
        variable.getValue(&lval);
        lval -= val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType())) {
        long lval;
        val = stol(strval);
        variable.getValue(&lval);
        lval -= val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType())) {
        double dval;
        floatval = stod(strval);
        variable.getValue(&dval);
        dval -= floatval;
        variable.setValue(dval);
    }

    if (!gpobj->m_variables.update(variable,pAdminUser)) {

        syslog(LOG_ERR,
               "[DM] "
               "[Action] Subtract from Variable: Failed to update variable. "
               "Path = %s"
               " ",
               m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionMultiplyVariable
//

bool
dmElement::doActionMultiplyVariable(vscpEvent *pDMEvent)
{
    CVariable variable;
    std::string strName;
    long val        = 0;
    double floatval = 0.0;

    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    // Get the variable name
    strName = vscp_str_before(str, ';'); // TODO check

    // Get the value
    std::string strval = vscp_str_after(str, ';');

    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (0 == gpobj->m_variables.find(strName, pAdminUser, variable)) {
        syslog(LOG_ERR,
               "[DM] "
               "[Action] Multiply Variable: Variable was not found. Param = %s",
               m_actionparam.c_str());
        return false;
    }

    // Must be a numerical variable
    if (!variable.isNumerical()) {
        syslog(
          LOG_ERR,
          "[DM] "
          "[Action] Multiply Variable: Variable is not numerical. Param = %s ",
          m_actionparam.c_str());
        return false;
    }

    if ((VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType())) {
        long lval;
        val = stol(strval);
        variable.getValue(&lval);
        lval *= val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType())) {
        long lval;
        val = stol(strval);
        variable.getValue(&lval);
        lval *= val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType())) {
        double dval;
        floatval = stod(strval);
        variable.getValue(&dval);
        dval *= floatval;
        variable.setValue(dval);
    }

    if (!gpobj->m_variables.update(variable,pAdminUser)) {

        syslog(
          LOG_ERR,
          "[DM] "
          "[Action] Multiply Variable: Failed to update variable. Param = %s ",
          m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionDivideVariable
//

bool
dmElement::doActionDivideVariable(vscpEvent *pDMEvent)
{
    CVariable variable;
    std::string strName;
    long val        = 0;
    double floatval = 0.0;

    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    // Get the variable name
    strName = vscp_str_before(str, ';');

    // Get the value
    std::string strval = vscp_str_after(str, ';');

    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (0 == gpobj->m_variables.find(strName, pAdminUser, variable)) {
        syslog(LOG_ERR,
               "[DM] "
               "[Action] Divide Variable: Variable was not found. Param = %s",
               m_actionparam.c_str());
        return false;
    }

    // Must be a numerical variable
    if (!variable.isNumerical()) {
        syslog(
          LOG_ERR,
          "[DM] "
          "[Action] Divide Variable: Variable is not numerical. Param = %s",
          m_actionparam.c_str());
        return false;
    }

    if ((VSCP_DAEMON_VARIABLE_CODE_LONG == variable.getType())) {
        long lval;
        val = stol(strval);
        if (0 == val) {
            syslog(LOG_ERR,
                   "[DM] "
                   "[Action] Can't divide with zero. Param = %s",
                   m_actionparam.c_str());
            val = 1;
        }
        variable.getValue(&lval);
        lval /= val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_INTEGER == variable.getType())) {
        long lval;
        val = stol(strval);
        if (0 == val) {
            syslog(LOG_ERR,
                   "[DM] "
                   "[Action] Can't divide with zero. Param = %s",
                   m_actionparam.c_str());
            val = 1;
        }
        variable.getValue(&lval);
        lval /= val;
        variable.setValue(lval);
    } else if ((VSCP_DAEMON_VARIABLE_CODE_DOUBLE == variable.getType())) {
        double dval;
        floatval = stod(strval);
        if (0 == floatval) {
            syslog(LOG_ERR,
                   "[DM] "
                   "[Action] Can't divide with zero. Param = %s",
                   m_actionparam.c_str());
            floatval = 1;
        }
        variable.getValue(&dval);
        dval /= floatval;
        variable.setValue(dval);
    }

    if (!gpobj->m_variables.update(variable,pAdminUser)) {
        syslog(
          LOG_ERR,
          "[DM] "
          "[Action] Divide Variable: Failed to update variable. Param = %s",
          m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionCheckVariable
//

bool
dmElement::doActionCheckVariable(vscpEvent *pDMEvent, VariableCheckType type)
{
    std::string str;
    double value      = 0;
    uint8_t operation = DM_MEASUREMENT_COMPARE_NOOP;
    CVariable varCompare; // Variable to compare
    CVariable varFlag;    // Variable with flag

    // Write in possible escapes
    std::string params = m_actionparam;
    handleEscapes(pDMEvent, params);

    // value;unit;operation;variable;flag-variable
    std::deque<std::string> tokens;
    vscp_split(tokens, params, ";");

    // Value
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        value = stod(str);
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter (value). param = %s",
               params.c_str());
        return false;
    }

    // Operation
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        operation = CDM::getCompareCodeFromToken(str);
        if (-1 == operation) {
            syslog(LOG_ERR,
                   "[DM] [Action] Invalid compare operation. param = %s ",
                   params.c_str());
            operation = DM_MEASUREMENT_COMPARE_NOOP;
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter (operation). param = %s",
               params.c_str());
        return false;
    }

    // Variable to compare
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
        if (!gpobj->m_variables.find(str, pAdminUser, varCompare)) {
            // Variable not found
            syslog(LOG_ERR,
                   "[DM] [Action] Compare variable was not found. param = %s",
                   params.c_str());
            return false;
        }

        // The variable must be numerical
        if (!varCompare.isNumerical()) {
            syslog(LOG_ERR,
                   "[DM] [Action] Compare variable need to be numerical. %s"
                   "param = ",
                   params.c_str());
            return false;
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter (variable). param = %s",
               params.c_str());
        return false;
    }

    // Variable with flag
    if (!tokens.empty()) {
        std::string varName = tokens.front();
        tokens.pop_front();
        vscp_trim(varName);
        CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
        if (!gpobj->m_variables.find(varName, pAdminUser, varFlag)) {
            // Get the variable
            if (!gpobj->m_variables.find(varName, pAdminUser, varFlag)) {
                // Well should not happen - but in case...
                syslog(LOG_ERR,
                       "[DM] [Action] Check measurement: Variable [%s] "
                       "was not found (but was created alsubstr).",
                       (const char *)varName.c_str());
                return false;
            }
        }

        // The variable must be boolean
        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != varFlag.getType()) {
            syslog(LOG_ERR,
                   "[DM] [Action] Flag variable must be boolean. param = %s",
                   params.c_str());
            return false;
        }
    } else {
        syslog(
          LOG_ERR,
          "[DM] [Action] Missing needed parameter (flag variable). param = %s",
          params.c_str());
        return false;
    }

    double valCompare;
    varCompare.getValue(&valCompare);

    switch (operation) {
        case DM_MEASUREMENT_COMPARE_NOOP:
            if (VARIABLE_CHECK_SET_TRUE == type) {
                varFlag.setValue(true);
            } else if (VARIABLE_CHECK_SET_FALSE == type) {
                varFlag.setValue(false);
            }
            break;

        case DM_MEASUREMENT_COMPARE_EQ:
            if (vscp_almostEqualRelativeDouble(value, valCompare)) {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(true);
                }
            } else {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(false);
                }
            }
            break;

        case DM_MEASUREMENT_COMPARE_NEQ:
            if (!vscp_almostEqualRelativeDouble(value, valCompare)) {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(true);
                }
            } else {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(false);
                }
            }
            break;

        case DM_MEASUREMENT_COMPARE_LT:
            if (value < valCompare) {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(true);
                }
            } else {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(false);
                }
            }
            break;

        case DM_MEASUREMENT_COMPARE_LTEQ:
            if ((value < valCompare) ||
                vscp_almostEqualRelativeDouble(value, valCompare)) {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(true);
                }
            } else {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(false);
                }
            }
            break;

        case DM_MEASUREMENT_COMPARE_GT:
            if (value > valCompare) {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(true);
                }
            } else {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(false);
                }
            }
            break;

        case DM_MEASUREMENT_COMPARE_GTEQ:
            if ((value >= valCompare) ||
                vscp_almostEqualRelativeDouble(value, valCompare)) {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(true);
                }
            } else {
                if (VARIABLE_CHECK_SET_TRUE == type) {
                    varFlag.setValue(false);
                } else if (VARIABLE_CHECK_SET_FALSE == type) {
                    varFlag.setValue(true);
                } else if (VARIABLE_CHECK_SET_OUTCOME == type) {
                    varFlag.setValue(false);
                }
            }
            break;
    }

    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (!gpobj->m_variables.update(varFlag, pAdminUser)) {
        syslog(LOG_ERR,
               "[DM] [Action] Compare Variable: "
               "Failed to update variable. Param = %s ",
               m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionCheckMeasurement
//

bool
dmElement::doActionCheckMeasurement(vscpEvent *pDMEvent)
{
    std::string str;
    int unit                = 0;
    int sensorIndex         = 0;
    double value            = 0;
    double valueMeasurement = 0;
    uint8_t operation       = DM_MEASUREMENT_COMPARE_NOOP;
    CVariable varFlag; // Variable for result flag

    // Write in possible escapes
    std::string escaped_param = m_actionparam;
    handleEscapes(pDMEvent, escaped_param);

    // Make sure it is a measurement event
    if (!vscp_isMeasurement(pDMEvent)) {
        syslog(LOG_ERR,
               "[DM] [Action] Must be measurement event. param = %s",
               escaped_param.c_str());
        return false;
    }

    // Get value
    if (!vscp_getMeasurementAsDouble(pDMEvent, &valueMeasurement)) {
        syslog(LOG_ERR,
               "[DM] [Action] Failed to get measurement value. param = %s",
               escaped_param.c_str());
        return false;
    }

    // unit;sendorindex;value;operation;flag-variable
    std::deque<std::string> tokens;
    vscp_split(tokens, escaped_param, ";");

    // unit
    if (!tokens.empty()) {
        unit = vscp_readStringValue(tokens.front());
        tokens.pop_front();

        if (unit != vscp_getMeasurementUnit(pDMEvent)) {
            // It's another unit
            syslog(LOG_ERR,
                   "[DM] [Action] Different unit. param = %s",
                   escaped_param.c_str());
            return false;
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter (unit). param = %s",
               escaped_param.c_str());
        return false;
    }

    // sensor index
    if (!tokens.empty()) {
        sensorIndex = vscp_readStringValue(tokens.front());
        tokens.pop_front();

        if (sensorIndex != vscp_getMeasurementSensorIndex(pDMEvent)) {
            // It's another unit
            syslog(LOG_ERR,
                   "[DM] [Action] Different sensor index. param = %s",
                   escaped_param.c_str());
            return false;
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter "
               "(sensor index). param = %s",
               escaped_param.c_str());
        return false;
    }

    // valueCompare
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        value = stod(str);
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter (value). param = %s",
               escaped_param.c_str());
        return false;
    }

    // Operation
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        operation = CDM::getCompareCodeFromToken(str);
        if (-1 == operation) {
            syslog(LOG_ERR,
                   "[DM] [Action] Invalid compare operation. param = %s",
                   escaped_param.c_str());
            operation = DM_MEASUREMENT_COMPARE_NOOP;
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter (operation). param = %s",
               escaped_param.c_str());
        return false;
    }

    // Variable (flag) for result (Boolean)
    if (!tokens.empty()) {
        std::string varName = tokens.front();
        tokens.pop_front();
        vscp_trim(varName);
        CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
        if (!gpobj->m_variables.find(varName, pAdminUser, varFlag)) {
            // Variable was not found - create it
            if (!gpobj->m_variables.add(
                  varName, "false", VSCP_DAEMON_VARIABLE_CODE_BOOLEAN)) {
                syslog(LOG_ERR,
                       "[DM] [Action] Check measurement: Variable [%s] "
                       "not defined. Failed to create it.",
                       (const char *)varName.c_str());
                return false;
            }

            // Get the variable
            CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
            if (!gpobj->m_variables.find(varName, pAdminUser, varFlag)) {
                syslog(LOG_ERR,
                       "[DM] [Action] Check measurement: Variable [%s] "
                       "was not found.",
                       varName.c_str());
                return false;
            }
        }

        // The variable must be boolean
        if (VSCP_DAEMON_VARIABLE_CODE_BOOLEAN != varFlag.getType()) {
            syslog(LOG_ERR,
                   "[DM] [Action] Result (flag) variable must be boolean. "
                   "param = %s",
                   escaped_param.c_str());
            return false;
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Missing needed parameter "
               "(flag variable). param = %s",
               escaped_param.c_str());
        return false;
    }

    switch (operation) {
        case DM_MEASUREMENT_COMPARE_NOOP:
            break;

        case DM_MEASUREMENT_COMPARE_EQ:
            if (vscp_almostEqualRelativeDouble(value, valueMeasurement)) {
                varFlag.setValue(true);
            } else {
                varFlag.setValue(false);
            }
            break;

        case DM_MEASUREMENT_COMPARE_NEQ:
            if (!vscp_almostEqualRelativeDouble(value, valueMeasurement)) {
                varFlag.setValue(true);
            } else {
                varFlag.setValue(false);
            }
            break;

        case DM_MEASUREMENT_COMPARE_LT:
            if (value < valueMeasurement) {
                varFlag.setValue(true);
            } else {
                varFlag.setValue(false);
            }
            break;

        case DM_MEASUREMENT_COMPARE_LTEQ:
            if ((value < valueMeasurement) ||
                vscp_almostEqualRelativeDouble(value, valueMeasurement)) {
                varFlag.setValue(true);
            } else {
                varFlag.setValue(false);
            }
            break;

        case DM_MEASUREMENT_COMPARE_GT:
            if (value > valueMeasurement) {
                varFlag.setValue(true);
            } else {
                varFlag.setValue(false);
            }
            break;

        case DM_MEASUREMENT_COMPARE_GTEQ:
            if ((value >= valueMeasurement) ||
                vscp_almostEqualRelativeDouble(value, valueMeasurement)) {
                varFlag.setValue(true);
            } else {
                varFlag.setValue(false);
            }
            break;
    }

    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (!gpobj->m_variables.update(varFlag,pAdminUser)) {
        syslog(LOG_ERR,
               "[DM] [Action] Compare Variable: "
               "Failed to update variable %s",
               m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStoreMin
//

bool
dmElement::doActionStoreMin(vscpEvent *pDMEvent)
{
    std::string varname;
    uint8_t unit;
    uint8_t sensorindex;
    uint8_t zone;
    uint8_t subzone;
    CVariable variable; // Variable to store possible new min in
    double currentValue;    // Current value of variable
    double value;           // Value of event

    // Write in possible escapes
    std::string params = m_actionparam;
    handleEscapes(pDMEvent, params);

    // Event must be measurement
    if (!vscp_isMeasurement(pDMEvent)) {
        syslog(LOG_ERR,
               "[DM] [Action] Event must be a "
               "measurement event. param = %s",
               params.c_str());
        return false;
    }

    // Get value from measurement event
    vscp_getMeasurementAsDouble(pDMEvent, &value);

    // variable;unit;index;zone;subzone
    std::deque<std::string> tokens;
    vscp_split(tokens, params, ";");

    if (tokens.empty()) {
        syslog(LOG_ERR,
               "[DM] [Action] Need variable name. param =%s",
               params.c_str());
        return false;
    }

    // Get variable name
    varname = tokens.front();
    tokens.pop_front();
    vscp_trim(varname);

    // Find the variable
    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (!gpobj->m_variables.find(varname, pAdminUser, variable)) {
        // Variable not found
        syslog(LOG_ERR,
               "[DM] [Action] Variable was not found. param = %s",
               params.c_str());
        return false;
    }

    // Variable must be numerical to be comparable
    if (!variable.isNumerical()) {
        syslog(LOG_ERR,
               "[DM] [Action] Can't compare with non "
               "numeric variable. param = %s",
               params.c_str());
        return false;
    }

    // Get variable value
    variable.getValue(&currentValue);

    // Unit
    if (!tokens.empty()) {
        unit = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        unit = 0; // Default unit
    }

    // Sensor index
    if (!tokens.empty()) {
        sensorindex = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        sensorindex = 0; // Default unit
    }

    // zone
    if (!tokens.empty()) {
        zone = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        zone = 0; // Default unit
    }

    // subzone
    if (!tokens.empty()) {
        subzone = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        subzone = 0; // Default unit
    }

    if ((currentValue > value) &&
        (unit == vscp_getMeasurementUnit(pDMEvent)) &&
        (sensorindex == vscp_getMeasurementSensorIndex(pDMEvent)) &&
        (zone == vscp_getMeasurementZone(pDMEvent)) &&
        (subzone == vscp_getMeasurementSubZone(pDMEvent))) {
        // Store new lowest value
        variable.setValue(value);
    }

    if (!gpobj->m_variables.update(variable,pAdminUser)) {
        syslog(LOG_ERR,
               "[DM] [Action] Minimum: "
               "Failed to update variable param = %s",
               m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStoreMax
//

bool
dmElement::doActionStoreMax(vscpEvent *pDMEvent)
{
    std::string varname;
    uint8_t unit;
    uint8_t sensorindex;
    uint8_t zone;
    uint8_t subzone;
    CVariable variable; // Variable to store possible new max in
    double currentValue;    // Current value of variable
    double value;           // Value of event

    // Write in possible escapes
    std::string params = m_actionparam;
    handleEscapes(pDMEvent, params);

    // Event must be measurement
    if (!vscp_isMeasurement(pDMEvent)) {
        syslog(LOG_ERR,
               "[DM] [Action] Event must be a measurement event. param =%s",
               +params.c_str());
        return false;
    }

    // Get value from measurement event
    vscp_getMeasurementAsDouble(pDMEvent, &value);

    // variable;unit;index;zone;subzone
    std::deque<std::string> tokens;
    vscp_split(tokens, params, ";");

    if (tokens.empty()) {
        syslog(LOG_ERR,
               "[DM] [Action] Need variable name. param = %s",
               params.c_str());
        return false;
    }

    // Get variable name
    varname = tokens.front();
    tokens.pop_front();
    vscp_trim(varname);

    // Find the variable
    CUserItem *pAdminUser = gpobj->m_userList.getUser(USER_ID_ADMIN);
    if (!gpobj->m_variables.find(varname, pAdminUser, variable)) {
        // Variable not found
        syslog(LOG_ERR,
               "[DM] [Action] Variable was not found. param = %s",
               params.c_str());
        return false;
    }

    // Variable must be numerical to be comparable
    if (!variable.isNumerical()) {
        syslog(LOG_ERR,
               "[DM] [Action] Can't compare with non numeric variable. "
               "param = %s",
               params.c_str());
        return false;
    }

    // Get variable value
    variable.getValue(&currentValue);

    // Unit
    if (!tokens.empty()) {
        unit = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        unit = 0; // Default unit
    }

    // Sensor index
    if (!tokens.empty()) {
        sensorindex = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        sensorindex = 0; // Default unit
    }

    // zone
    if (!tokens.empty()) {
        zone = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        zone = 0; // Default unit
    }

    // subzone
    if (!tokens.empty()) {
        subzone = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        subzone = 0; // Default unit
    }

    if ((currentValue < value) &&
        (unit == vscp_getMeasurementUnit(pDMEvent)) &&
        (sensorindex == vscp_getMeasurementSensorIndex(pDMEvent)) &&
        (zone == vscp_getMeasurementZone(pDMEvent)) &&
        (subzone == vscp_getMeasurementSubZone(pDMEvent))) {
        // Store new highest value
        variable.setValue(value);
    }

    if (!gpobj->m_variables.update(variable,pAdminUser)) {
        syslog(LOG_ERR,
               "[DM] [Action] Maximum: Failed to update variable %s ",
               m_actionparam.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStartTimer
//

bool
dmElement::doActionStartTimer(vscpEvent *pDMEvent)
{
    bool bAutoReload = false;
    std::string strVarName;
    int reloadLimit = -1;

    // Write in possible escapes
    std::string escaped_action = m_actionparam;
    handleEscapes(pDMEvent, escaped_action);

    std::deque<std::string> tokens;
    vscp_split(tokens, escaped_action, ";");

    // Timer id
    if (tokens.empty()) {
        // Strange action parameter
        syslog(LOG_ERR,
               "[DM] [Action] Start timer: Wrong action parameter %s",
               escaped_action.c_str());
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // Initial count
    if (tokens.empty()) {
        // Strange action parameter
        syslog(LOG_ERR,
               "[DM] [Action] Start timer: Delay is missing %s",
               escaped_action.c_str());
        return false;
    }

    // Get initial time
    uint32_t initialTime = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // Flag variable   // Can be blank
    if (!tokens.empty()) {
        strVarName = tokens.front();
        tokens.pop_front();
        vscp_trim(strVarName);
    }

    // Auto reload
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        if (0 == vscp_strcasecmp(str.c_str(), "true")) {
            bAutoReload = true;
        }
    }

    //  Reload Limit
    if (!tokens.empty()) {
        reloadLimit = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    }

    if (0 == m_pDM->startTimer(
               id, strVarName, initialTime, bAutoReload, reloadLimit)) {
        // Strange action parameter
        syslog(LOG_ERR,
               "DM [Action] Start timer: Failed to start timer due to "
               "strange "
               "action parameter %s  ",
               escaped_action.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionPauseTimer
//

bool
dmElement::doActionPauseTimer(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ";");

    if (tokens.empty()) {
        // Strange action parameter
        syslog(LOG_ERR,
               "[DM] [Action] Stop timer: Error in action parameter = %s",
               str.c_str());
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    m_pDM->pauseTimer(id);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionResumeTimer
//

bool
dmElement::doActionResumeTimer(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ";");

    if (tokens.empty()) {
        // Strange action parameter
        syslog(LOG_ERR,
               "[DM] [Action] Stop timer: Error in action parameter %s",
               str.c_str());
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    m_pDM->startTimer(id);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doActionStopTimer
//

bool
dmElement::doActionStopTimer(vscpEvent *pDMEvent)
{
    // Write in possible escapes
    std::string str = m_actionparam;
    handleEscapes(pDMEvent, str);

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ";");

    if (tokens.empty()) {
        // Strange action parameter
        syslog(LOG_ERR,
               "[DM] [Action] Stop timer: Wrong action parameter %s",
               str.c_str());
        return false;
    }

    // Get timer id
    uint32_t id = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    m_pDM->stopTimer(id);

    return true;
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CDM::CDM(CControlObject *ctrlObj)
{
    m_bEnable = true;

    // Set the default dm configuration path
    m_staticXMLPath = "/srv/vscp/dm.xml";

    // Not allowed to edit XML content
    bAllowXMLsave = false;

    // Default is to feed all events through the matrix
    vscp_clearVSCPFilter(&m_DM_Table_filter);

    srand(vscpdatetime::Now().getYear() +
          vscpdatetime::Now().getSecond());
    m_rndMinute =
      (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 60;
    m_rndHour =
      (uint8_t)(((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 60);
    m_rndDay =
      (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 24;
    m_rndWeek =
      (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 7;
    m_rndMonth =
      (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 30;
    m_rndYear =
      (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 365;

    m_lastTime = vscpdatetime::Now();

    m_staticXMLPath   = "/srv/vscp/dm.xml";
    m_path_db_vscp_dm = "/srv/vscp/dm.sqlite3";

    m_db_vscp_dm = NULL;
    // m_db_vscp_dm_memory = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CDM::~CDM()
{
    // Must remove any timers
    std::map<int, dmTimer *>::iterator it;
    for (it = m_timerHash.begin(); it != m_timerHash.end(); ++it) {
        dmTimer *pTimer = it->second;

        // Check if variable name is already there
        if (NULL != pTimer) {
            delete pTimer;
            pTimer = NULL;
        }
    }

    // Remove all elements
    m_timerHash.clear();
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
CDM::init(void)
{
    std::string str;

    // * * * VSCP Daemon DM database * * *

    // Check filename
    if (vscp_fileExists(m_path_db_vscp_dm)) {
        if (SQLITE_OK !=
            sqlite3_open(m_path_db_vscp_dm.c_str(), &m_db_vscp_dm)) {
            // Failed to open/create the database file
            syslog(LOG_ERR,
                   "VSCP Daemon decision matrix database could not "
                   "be opened."
                   " - Will not be used. Path=%s error=%s",
                   (const char *)m_path_db_vscp_dm.c_str(),
                   sqlite3_errmsg(m_db_vscp_dm));

            if (NULL != m_db_vscp_dm) sqlite3_close(m_db_vscp_dm);
            m_db_vscp_dm = NULL;
        }
    } else {

        // We need to create the database from scratch. This may
        // not work if the database is in a read only location.
        syslog(LOG_ERR,
               "VSCP Daemon DM database does not exist - will "
               "be created. Path=%s",
               m_path_db_vscp_dm.c_str());

        if (SQLITE_OK == sqlite3_open((const char *)m_path_db_vscp_dm.c_str(),
                                      &m_db_vscp_dm)) {
            // create the table.
            doCreateDMTable();
        } else {
            syslog(LOG_ERR,
                   "Failed to create VSCP DM database - "
                   "will not be used. Path=%s error=%s",
                   m_path_db_vscp_dm.c_str(),
                   sqlite3_errmsg(m_db_vscp_dm));
        }
    }

    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM) {
        syslog(LOG_ERR,
               "[DM] DM engine started. DM from [%s]",
               m_staticXMLPath.c_str());
    }
}

///////////////////////////////////////////////////////////////////////////////
// cleanup
//

void
CDM::cleanup(void)
{
    if (NULL != m_db_vscp_dm) sqlite3_close(m_db_vscp_dm);
    m_db_vscp_dm = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// getCompareCodeFromToken
//

int
CDM::getCompareCodeFromToken(std::string &token)
{
    vscp_makeLower(token);
    vscp_trim(token);

    if (vscp_startsWith(token, "noop")) {
        return DM_MEASUREMENT_COMPARE_NOOP;
    } else if (vscp_startsWith(token, "eqgt")) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    } else if (vscp_startsWith(token, "eqlt")) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
    } else if (vscp_startsWith(token, "eq")) {
        return DM_MEASUREMENT_COMPARE_EQ;
    } else if (vscp_startsWith(token, "==")) {
        return DM_MEASUREMENT_COMPARE_EQ;
    } else if (vscp_startsWith(token, "neq")) {
        return DM_MEASUREMENT_COMPARE_NEQ;
    } else if (vscp_startsWith(token, "!=")) {
        return DM_MEASUREMENT_COMPARE_NEQ;
    } else if (vscp_startsWith(token, "<")) {
        return DM_MEASUREMENT_COMPARE_LT;
    } else if (vscp_startsWith(token, ">")) {
        return DM_MEASUREMENT_COMPARE_GT;
    } else if (vscp_startsWith(token, "gteq")) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    } else if (vscp_startsWith(token, "gt")) {
        return DM_MEASUREMENT_COMPARE_GT;
    } else if (vscp_startsWith(token, ">=")) {
        return DM_MEASUREMENT_COMPARE_GTEQ;
    } else if (vscp_startsWith(token, "lteq")) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
    } else if (vscp_startsWith(token, "lt")) {
        return DM_MEASUREMENT_COMPARE_LT;
    } else if (vscp_startsWith(token, "<=")) {
        return DM_MEASUREMENT_COMPARE_LTEQ;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////////
// getTokenFromCompareCode
//

std::string
CDM::getTokenFromCompareCode(uint8_t code)
{
    switch (code) {
        case DM_MEASUREMENT_COMPARE_NOOP:
            return "noop";
            break;

        case DM_MEASUREMENT_COMPARE_EQ:
            return "eq";
            break;

        case DM_MEASUREMENT_COMPARE_NEQ:
            return "neq";
            break;

        case DM_MEASUREMENT_COMPARE_LT:
            return "lt";
            break;

        case DM_MEASUREMENT_COMPARE_LTEQ:
            return "lteq";
            break;

        case DM_MEASUREMENT_COMPARE_GT:
            return "eq";
            break;

        case DM_MEASUREMENT_COMPARE_GTEQ:
            return "lteq";
            break;

        default:
            break;
    }

    return "error";
}

///////////////////////////////////////////////////////////////////////////////
// addMemoryElement
//
// Read decision matrix from file
//

bool
CDM::addMemoryElement(dmElement *pItem)
{
    // Check pointer
    if (NULL == pItem) return false;

    // Let it know we own it
    pItem->m_pDM = this;

    pthread_mutex_lock(&m_mutexDM);
    m_DMList.push_back(pItem);
    pthread_mutex_unlock(&m_mutexDM);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeMemoryElement
//

bool
CDM::removeMemoryElement(unsigned short row)
{
    if (row >= m_DMList.size()) return false;

    std::deque<dmElement *>::iterator it = m_DMList.begin();

    pthread_mutex_lock(&m_mutexDM);
    m_DMList.erase(it + row);
    pthread_mutex_unlock(&m_mutexDM);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getMemoryElementFromRow
//

dmElement *
CDM::getMemoryElementFromRow(const short row)
{
    if (row < 0) return NULL;
    if ((unsigned short)row >= m_DMList.size()) return NULL;

    return m_DMList[row];
}

///////////////////////////////////////////////////////////////////////////////
// getDbIndexFromRow
//

uint32_t
CDM::getDbIndexFromRow(const short row)
{
    uint32_t idx = 0;

    dmElement *pElement = m_DMList[row];

    if (idx) {
        idx = pElement->m_id;
    }

    return idx;
}

///////////////////////////////////////////////////////////////////////////////
// getMemoryElementFromId
//

dmElement *
CDM::getMemoryElementFromId(const uint32_t idx, short *prow)
{
    short row = 0;
    std::deque<dmElement *>::iterator it;
    for (it = m_DMList.begin(); it != m_DMList.end(); ++it) {
        dmElement *pe = *it;
        if (idx == pe->m_id) {
            if (NULL != prow) *prow = row;
            return pe;
        }
        row++;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// getAllRows
//

bool
CDM::getAllRows(std::string &strAllRows)
{
    short row = 0;

    strAllRows.clear();

    std::deque<dmElement *>::iterator it;
    for (it = m_DMList.begin(); it != m_DMList.end(); ++it) {
        dmElement *pe = *it;
        if (NULL != pe) strAllRows += pe->getAsString();
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// feedPeriodicEvent
//

bool
CDM::feedPeriodicEvent(void)
{
    if (m_lastTime.getSecond() != vscpdatetime::Now().getSecond()) {
        vscpEvent EventSecond;
        EventSecond.vscp_class = VSCP_CLASS2_VSCPD;
        EventSecond.vscp_type =
          VSCP2_TYPE_VSCPD_SECOND;                     // Internal Second Event
        EventSecond.head      = VSCP_PRIORITY_NORMAL;  // Set priority
        EventSecond.timestamp = vscp_makeTimeStamp();  // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventSecond); // Set time
        EventSecond.sizeData = 0;                      // No data
        EventSecond.pdata    = NULL;
        // memcpy( EventSecond.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventSecond.GUID);
        feed(&EventSecond);

        // Update timers
        serviceTimers();

        if (m_rndMinute == vscpdatetime::Now().getSecond()) {
            vscpEvent EventRandomMinute;
            EventRandomMinute.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomMinute.vscp_type =
              VSCP2_TYPE_VSCPD_RANDOM_MINUTE;                   // Internal
                                                                // Random-Minute
                                                                // Event
            EventRandomMinute.head      = VSCP_PRIORITY_NORMAL; // Set priority
            EventRandomMinute.sizeData  = 0;                    // No data
            EventRandomMinute.timestamp = vscp_makeTimeStamp(); // Set timestamp
            vscp_setEventDateTimeBlockToNow(&EventRandomMinute); // Set time
            EventRandomMinute.pdata = NULL;
            // memcpy( EventRandomMinute.GUID,
            // m_pCtrlObject->m_GUID, 16 ); // Server GUID
            gpobj->m_guid.writeGUID(EventRandomMinute.GUID);
            feed(&EventRandomMinute);
        }
    }

    if (m_lastTime.getMinute() != vscpdatetime::Now().getMinute()) {
        m_rndMinute = (rand() / RAND_MAX) * 60;
        vscpEvent EventMinute;
        EventMinute.vscp_class = VSCP_CLASS2_VSCPD;
        EventMinute.vscp_type =
          VSCP2_TYPE_VSCPD_MINUTE;                     // Internal Minute Event
        EventMinute.head      = VSCP_PRIORITY_NORMAL;  // Set priority
        EventMinute.timestamp = vscp_makeTimeStamp();  // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventMinute); // Set time
        EventMinute.sizeData = 0;                      // No data
        EventMinute.pdata    = NULL;
        // memcpy( EventMinute.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventMinute.GUID);
        m_rndMinute =
          (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 60;
        feed(&EventMinute);

        if (m_rndHour == vscpdatetime::Now().getMinute()) {
            vscpEvent EventRandomHour;
            EventRandomHour.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomHour.vscp_type =
              VSCP2_TYPE_VSCPD_RANDOM_HOUR; // Internal
                                            // Random-Hour Event
            EventRandomHour.head      = VSCP_PRIORITY_NORMAL;  // Set priority
            EventRandomHour.timestamp = vscp_makeTimeStamp();  // Set timestamp
            vscp_setEventDateTimeBlockToNow(&EventRandomHour); // Set time
            EventRandomHour.sizeData = 0;                      // No data
            EventRandomHour.pdata    = NULL;
            // memcpy( EventRandomHour.GUID, m_pCtrlObject->m_GUID,
            // 16 );    // Server GUID
            gpobj->m_guid.writeGUID(EventRandomHour.GUID);

            feed(&EventRandomHour);
        }
    }

    if (m_lastTime.getHour() != vscpdatetime::Now().getHour()) {
        m_rndHour = (rand() / RAND_MAX) * 60;
        vscpEvent EventHour;
        EventHour.vscp_class = VSCP_CLASS2_VSCPD;
        EventHour.vscp_type  = VSCP2_TYPE_VSCPD_HOUR; // Internal Hour Event
        EventHour.head       = VSCP_PRIORITY_NORMAL;  // Set priority
        EventHour.timestamp  = vscp_makeTimeStamp();  // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventHour);  // Set time
        EventHour.sizeData = 0;                       // No data
        EventHour.pdata    = NULL;
        // memcpy( EventtHour.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventHour.GUID);
        m_rndHour =
          (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 24;
        feed(&EventHour);

        // Check if it's time to send radom day event
        if (m_rndDay == vscpdatetime::Now().getHour()) {
            vscpEvent EventRandomDay;
            EventRandomDay.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomDay.vscp_type =
              VSCP2_TYPE_VSCPD_RANDOM_DAY; // Internal Random-Day
                                           // Event
            EventRandomDay.head      = VSCP_PRIORITY_NORMAL;  // Set priority
            EventRandomDay.timestamp = vscp_makeTimeStamp();  // Set timestamp
            vscp_setEventDateTimeBlockToNow(&EventRandomDay); // Set time
            EventRandomDay.sizeData = 0;                      // No data
            EventRandomDay.pdata    = NULL;
            // memcpy( EventRandomDay.GUID, m_pCtrlObject->m_GUID,
            // 16 );     // Server GUID
            gpobj->m_guid.writeGUID(EventRandomDay.GUID);
            feed(&EventRandomDay);
        }
    }

    if (m_lastTime.getDay() != vscpdatetime::Now().getDay()) {
        m_rndDay = (rand() / RAND_MAX) * 24;
        vscpEvent EventDay;
        EventDay.vscp_class = VSCP_CLASS2_VSCPD;
        EventDay.vscp_type =
          VSCP2_TYPE_VSCPD_MIDNIGHT;                // Internal Midnight Event
        EventDay.head      = VSCP_PRIORITY_NORMAL;  // Set priority
        EventDay.timestamp = vscp_makeTimeStamp();  // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventDay); // Set time
        EventDay.sizeData = 0;                      // No data
        EventDay.pdata    = NULL;
        // memcpy( EventDay.GUID, m_pCtrlObject->m_GUID, 16 );   //
        // Server GUID
        gpobj->m_guid.writeGUID(EventDay.GUID);
        m_rndDay =
          (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 7;
        feed(&EventDay);

        // Check if it's time to send random week event
        if (m_rndWeek == vscpdatetime::Now().getDay()) {
            vscpEvent EventRandomWeek;
            EventRandomWeek.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomWeek.vscp_type =
              VSCP2_TYPE_VSCPD_RANDOM_WEEK; // Internal
                                            // Random-Week Event
            EventRandomWeek.head      = VSCP_PRIORITY_NORMAL;  // Set priority
            EventRandomWeek.timestamp = vscp_makeTimeStamp();  // Set timestamp
            vscp_setEventDateTimeBlockToNow(&EventRandomWeek); // Set time
            EventRandomWeek.sizeData = 0;                      // No data
            EventRandomWeek.pdata    = NULL;
            // memcpy( EventRandomWeek.GUID, m_pCtrlObject->m_GUID,
            // 16 );// Server GUID
            gpobj->m_guid.writeGUID(EventRandomWeek.GUID);
            feed(&EventRandomWeek);
        }
    }

    if ((1 == vscpdatetime::Now().getWeekDay()) &&
        (m_lastTime.getWeekDay() != vscpdatetime::Now().getWeekDay())) {
        m_rndWeek = (rand() / RAND_MAX) * 7;
        vscpEvent EventWeek;
        EventWeek.vscp_class = VSCP_CLASS2_VSCPD;
        EventWeek.vscp_type  = VSCP2_TYPE_VSCPD_WEEK; // Internal Week Event
        EventWeek.head       = VSCP_PRIORITY_NORMAL;  // Set priority
        EventWeek.timestamp  = vscp_makeTimeStamp();  // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventWeek);  // Set time
        EventWeek.sizeData = 0;                       // No data
        EventWeek.pdata    = NULL;
        // memcpy( EventWeek.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventWeek.GUID);
        m_rndWeek =
          (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 52;
        feed(&EventWeek);

        if (m_rndMonth == vscpdatetime::Now().getWeekDay()) {
            vscpEvent EventRandomMonth;
            EventRandomMonth.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomMonth.vscp_type =
              VSCP2_TYPE_VSCPD_RANDOM_MONTH;                    // Internal
                                                                // Random-Month
                                                                // Event
            EventRandomMonth.head      = VSCP_PRIORITY_NORMAL;  // Set priority
            EventRandomMonth.timestamp = vscp_makeTimeStamp();  // Set timestamp
            vscp_setEventDateTimeBlockToNow(&EventRandomMonth); // Set time
            EventRandomMonth.sizeData = 0;                      // No data
            EventRandomMonth.pdata    = NULL;
            // memcpy( EventRandomMonth.GUID, m_pCtrlObject->m_GUID,
            // 16 );   // Server GUID
            gpobj->m_guid.writeGUID(EventRandomMonth.GUID);
            feed(&EventRandomMonth);
        }
    }

    if (m_lastTime.getMonth() != vscpdatetime::Now().getMonth()) {
        m_rndMonth = (rand() / RAND_MAX) * 30;
        vscpEvent EventMonth;
        EventMonth.vscp_class = VSCP_CLASS2_VSCPD;
        EventMonth.vscp_type  = VSCP2_TYPE_VSCPD_MONTH; // Internal Month Event
        EventMonth.head       = VSCP_PRIORITY_NORMAL;   // Set priority
        EventMonth.timestamp  = vscp_makeTimeStamp();   // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventMonth);   // Set time
        EventMonth.sizeData = 0;                        // No data
        EventMonth.pdata    = NULL;
        // memcpy( EventMonth.GUID, m_pCtrlObject->m_GUID, 16 );  //
        // Server GUID
        gpobj->m_guid.writeGUID(EventMonth.GUID);
        m_rndMonth =
          (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 12;
        feed(&EventMonth);

        if (m_rndYear == vscpdatetime::Now().getMonth()) {
            vscpEvent EventRandomYear;
            EventRandomYear.vscp_class = VSCP_CLASS2_VSCPD;
            EventRandomYear.vscp_type =
              VSCP2_TYPE_VSCPD_RANDOM_YEAR;                    // Internal
                                                               // Random-Minute
                                                               // Event
            EventRandomYear.head      = VSCP_PRIORITY_NORMAL;  // Set priority
            EventRandomYear.timestamp = vscp_makeTimeStamp();  // Set timestamp
            vscp_setEventDateTimeBlockToNow(&EventRandomYear); // Set time
            EventRandomYear.sizeData = 0;                      // No data
            EventRandomYear.pdata    = NULL;
            // memcpy( EventRandomYear.GUID, m_pCtrlObject->m_GUID,
            // 16 ); // Server GUID
            gpobj->m_guid.writeGUID(EventRandomYear.GUID);
            feed(&EventRandomYear);
        }
    }

    if (m_lastTime.getYear() != vscpdatetime::Now().getYear()) {
        m_rndYear = (rand() / RAND_MAX) * 365;
        vscpEvent EventYear;
        EventYear.vscp_class = VSCP_CLASS2_VSCPD;
        EventYear.vscp_type  = VSCP2_TYPE_VSCPD_YEAR; // Internal Year Event
        EventYear.head       = VSCP_PRIORITY_NORMAL;  // Set priority
        EventYear.timestamp  = vscp_makeTimeStamp();  // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventYear);  // Set time
        EventYear.sizeData = 0;                       // No data
        EventYear.pdata    = NULL;
        // memcpy( EventYear.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventYear.GUID);
        m_rndYear =
          (uint8_t)((double)rand() / ((double)(RAND_MAX) + (double)(1))) * 365;
        feed(&EventYear);
    }

    if ((1 == vscpdatetime::Now().getMonth()) &&
        (m_lastTime.getMonth() != vscpdatetime::Now().getMonth())) {
        vscpEvent EventQuarter;
        EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
        EventQuarter.vscp_type =
          VSCP2_TYPE_VSCPD_QUARTER;                    // Internal Quarter Event
        EventQuarter.head      = VSCP_PRIORITY_NORMAL; // Set priority
        EventQuarter.timestamp = vscp_makeTimeStamp(); // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventQuarter); // Set time
        EventQuarter.sizeData = 0;                      // No data
        EventQuarter.pdata    = NULL;
        // memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventQuarter.GUID);
        feed(&EventQuarter);
    }

    if ((4 == vscpdatetime::Now().getMonth()) &&
        (m_lastTime.getMonth() != vscpdatetime::Now().getMonth())) {
        vscpEvent EventQuarter;
        EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
        EventQuarter.vscp_type =
          VSCP2_TYPE_VSCPD_QUARTER;                    // Internal Quarter Event
        EventQuarter.head      = VSCP_PRIORITY_NORMAL; // Set priority
        EventQuarter.timestamp = vscp_makeTimeStamp(); // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventQuarter); // Set time
        EventQuarter.sizeData = 0;                      // No data
        EventQuarter.pdata    = NULL;
        // memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 );   //
        // Server GUID
        gpobj->m_guid.writeGUID(EventQuarter.GUID);
        feed(&EventQuarter);
    }

    if ((7 == vscpdatetime::Now().getMonth()) &&
        (m_lastTime.getMonth() != vscpdatetime::Now().getMonth())) {
        vscpEvent EventQuarter;
        EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
        EventQuarter.vscp_type =
          VSCP2_TYPE_VSCPD_QUARTER;                    // Internal Quarter Event
        EventQuarter.head      = VSCP_PRIORITY_NORMAL; // Set priority
        EventQuarter.timestamp = vscp_makeTimeStamp(); // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventQuarter); // Set time
        EventQuarter.sizeData = 0;                      // No data
        EventQuarter.pdata    = NULL;
        // memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventQuarter.GUID);
        feed(&EventQuarter);
    }

    if ((10 == vscpdatetime::Now().getMonth()) &&
        (m_lastTime.getMonth() != vscpdatetime::Now().getMonth())) {
        vscpEvent EventQuarter;
        EventQuarter.vscp_class = VSCP_CLASS2_VSCPD;
        EventQuarter.vscp_type =
          VSCP2_TYPE_VSCPD_QUARTER;                    // Internal Quarter Event
        EventQuarter.head      = VSCP_PRIORITY_NORMAL; // Set priority
        EventQuarter.timestamp = vscp_makeTimeStamp(); // Set timestamp
        vscp_setEventDateTimeBlockToNow(&EventQuarter); // Set time
        EventQuarter.sizeData = 0;                      // No data
        EventQuarter.pdata    = NULL;
        // memcpy( EventQuarter.GUID, m_pCtrlObject->m_GUID, 16 ); //
        // Server GUID
        gpobj->m_guid.writeGUID(EventQuarter.GUID);
        feed(&EventQuarter);
    }

    m_lastTime = vscpdatetime::Now();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerStarted
//

bool
CDM::feedTimerStarted(uint32_t id, uint32_t time)
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type =
      VSCP2_TYPE_VSCPD_TIMER_STARTED; // Internal Random-Minute Event
    EventTimerStart.head      = VSCP_PRIORITY_NORMAL;  // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();  // Set timestamp
    vscp_setEventDateTimeBlockToNow(&EventTimerStart); // Set time
    EventTimerStart.sizeData = 8;                      // No data
    uint8_t data[8];
    data[0]               = (id >> 24) & 0xff;
    data[1]               = (id >> 16) & 0xff;
    data[2]               = (id >> 8) & 0xff;
    data[3]               = id & 0xff;
    data[4]               = (time >> 24) & 0xff;
    data[5]               = (time >> 16) & 0xff;
    data[6]               = (time >> 8) & 0xff;
    data[7]               = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID(EventTimerStart.GUID);

    return feed(&EventTimerStart);
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerPaused
//

bool
CDM::feedTimerPaused(uint32_t id, uint32_t time)
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type =
      VSCP2_TYPE_VSCPD_TIMER_PAUSED; // Internal Random-Minute Event
    EventTimerStart.head      = VSCP_PRIORITY_NORMAL;  // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();  // Set timestamp
    vscp_setEventDateTimeBlockToNow(&EventTimerStart); // Set time
    EventTimerStart.sizeData = 8;                      // No data
    uint8_t data[8];
    data[0]               = (id >> 24) & 0xff;
    data[1]               = (id >> 16) & 0xff;
    data[2]               = (id >> 8) & 0xff;
    data[3]               = id & 0xff;
    data[4]               = (time >> 24) & 0xff;
    data[5]               = (time >> 16) & 0xff;
    data[6]               = (time >> 8) & 0xff;
    data[7]               = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID(EventTimerStart.GUID);

    return feed(&EventTimerStart);
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerResumed
//

bool
CDM::feedTimerResumed(uint32_t id, uint32_t time)
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type =
      VSCP2_TYPE_VSCPD_TIMER_RESUMED; // Internal Random-Minute Event
    EventTimerStart.head      = VSCP_PRIORITY_NORMAL;  // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();  // Set timestamp
    vscp_setEventDateTimeBlockToNow(&EventTimerStart); // Set time
    EventTimerStart.sizeData = 8;                      // No data
    uint8_t data[8];
    data[0]               = (id >> 24) & 0xff;
    data[1]               = (id >> 16) & 0xff;
    data[2]               = (id >> 8) & 0xff;
    data[3]               = id & 0xff;
    data[4]               = (time >> 24) & 0xff;
    data[5]               = (time >> 16) & 0xff;
    data[6]               = (time >> 8) & 0xff;
    data[7]               = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID(EventTimerStart.GUID);

    return feed(&EventTimerStart);
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerStopped
//

bool
CDM::feedTimerStopped(uint32_t id, uint32_t time)
{
    vscpEvent EventTimerStart;
    EventTimerStart.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerStart.vscp_type =
      VSCP2_TYPE_VSCPD_TIMER_STOPPED; // Internal Random-Minute Event
    EventTimerStart.head      = VSCP_PRIORITY_NORMAL;  // Set priority
    EventTimerStart.timestamp = vscp_makeTimeStamp();  // Set timestamp
    vscp_setEventDateTimeBlockToNow(&EventTimerStart); // Set time
    EventTimerStart.sizeData = 8;                      // No data
    uint8_t data[8];
    data[0]               = (id >> 24) & 0xff;
    data[1]               = (id >> 16) & 0xff;
    data[2]               = (id >> 8) & 0xff;
    data[3]               = id & 0xff;
    data[4]               = (time >> 24) & 0xff;
    data[5]               = (time >> 16) & 0xff;
    data[6]               = (time >> 8) & 0xff;
    data[7]               = time & 0xff;
    EventTimerStart.pdata = data;
    gpobj->m_guid.writeGUID(EventTimerStart.GUID);

    return feed(&EventTimerStart);
}

///////////////////////////////////////////////////////////////////////////////
// feedTimerElapsed
//

bool
CDM::feedTimerElapsed(uint32_t id)
{
    vscpEvent EventTimerElapsed;
    EventTimerElapsed.vscp_class = VSCP_CLASS2_VSCPD;
    EventTimerElapsed.vscp_type =
      VSCP2_TYPE_VSCPD_TIMER_ELLAPSED; // Internal Random-Minute Event
    EventTimerElapsed.head      = VSCP_PRIORITY_NORMAL;  // Set priority
    EventTimerElapsed.timestamp = vscp_makeTimeStamp();  // Set timestamp
    vscp_setEventDateTimeBlockToNow(&EventTimerElapsed); // Set time
    EventTimerElapsed.sizeData = 4;                      // No data
    uint8_t data[4];
    data[0]                 = (id >> 24) & 0xff;
    data[1]                 = (id >> 16) & 0xff;
    data[2]                 = (id >> 8) & 0xff;
    data[3]                 = id & 0xff;
    EventTimerElapsed.pdata = data;
    gpobj->m_guid.writeGUID(EventTimerElapsed.GUID);

    return feed(&EventTimerElapsed);
}

///////////////////////////////////////////////////////////////////////////////
// serviceTimers
//

void
CDM::serviceTimers(void)
{
    CVariable variable;

    std::map<int, dmTimer *>::iterator it;
    for (it = m_timerHash.begin(); it != m_timerHash.end(); ++it) {
        dmTimer *pTimer = it->second;

        if (pTimer->isActive()) {
            if (!pTimer->decTimer()) {
                // We have reached zero

                bool bVariableFound = false;
                CUserItem *pAdminUser =
                  gpobj->m_userList.getUser(USER_ID_ADMIN);
                if (gpobj->m_variables.find(
                      pTimer->getVariableName(), pAdminUser, variable)) {
                    bVariableFound = true;
                }

                // Set variable to true if one was defined
                if (bVariableFound) {
                    variable.setTrue();
                    if (!gpobj->m_variables.update(variable,pAdminUser)) {
                        syslog(LOG_ERR,
                               "[DM] [serviceTimers] Maximum: "
                               "Failed to update "
                               "variable ");
                    }
                }

                // Feed timer elapsed event
                feedTimerElapsed(pTimer->getId());

                if (pTimer->isReloading()) {
                    if (!pTimer->isReloadLimit()) {
                        pTimer->reload();
                        feedTimerStarted(pTimer->getId(),
                                         pTimer->getDelay() * 1000);
                    } else {
                        pTimer->reload(); // Reload even
                                          // if we
                                          // should stop
                        if (pTimer->getReloadCounter() > 0) {
                            feedTimerStarted(pTimer->getId(),
                                             pTimer->getDelay() * 1000);
                        } else {
                            pTimer->setActive(false); // We go
                                                      // inactive
                            // Feed timer stopped
                            // event
                            feedTimerStopped(pTimer->getId(),
                                             pTimer->getDelay() + 1000);
                        }
                    }
                } else {
                    pTimer->setActive(false); // We go inactive
                    // Feed timer stopped event
                    feedTimerStopped(pTimer->getId(),
                                     pTimer->getDelay() + 1000);
                }
            }

        } // Active and variable exists
    }
}

///////////////////////////////////////////////////////////////////////////////
// addTimer
//

int
CDM::addTimer(uint32_t id,
              std::string &nameVar,
              uint32_t delay,
              bool bStart,
              bool bReload,
              int reloadLimit)
{
    int rv = 0; // Default return value is failure
    dmTimer *pTimer;

    // Log
    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
        syslog(LOG_ERR,
               "[DM] [Add timer] Add Timer %s.",
               (const char *)nameVar.c_str());
    }

    // Check if the timer already exist - set new values
    if (NULL != (pTimer = m_timerHash[(int)id])) {
        pTimer->setVariableName(nameVar);
        pTimer->setId(id);
        pTimer->setReload(bReload);
        pTimer->setDelay(delay);
        pTimer->setActive(bStart);
        pTimer->setReloadLimit(reloadLimit);

        if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
            syslog(LOG_ERR,
                   "[DM] [Add timer] Timer %d "
                   "already exist.",
                   id);
        }
    } else {
        // Create the timer
        dmTimer *pTimer =
          new dmTimer(nameVar, id, delay, bStart, bReload, reloadLimit);

        if (NULL == pTimer) {
            syslog(LOG_ERR,
                   "[DM] [Add timer] Timer %d "
                   "Could not create.",
                   id);
            return 0;
        }

        // add the timer to the hash table
        rv              = id;
        m_timerHash[id] = pTimer;
    }

    vscp_trim(nameVar);

    // Create variable if it does not exist
    if (!gpobj->m_variables.exist(nameVar)) {
        if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
            syslog(LOG_ERR,
                   "[DM] [Add timer] Variable [%s] not defined. "
                   "Creating it.",
                   (const char *)nameVar.c_str());
        }

        if (!gpobj->m_variables.add(
              nameVar, "false", VSCP_DAEMON_VARIABLE_CODE_BOOLEAN)) {
            syslog(LOG_ERR,
                   "[DM] [Add timer] Variable [%s] not defined. "
                   "Failed to create it.",
                   (const char *)nameVar.c_str());
            return false;
        }
    }

    // Start timer if requested to do so
    if (bStart) startTimer(id);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

bool
CDM::startTimer(uint32_t idTimer)
{
    dmTimer *pTimer = m_timerHash[idTimer];
    if (NULL == pTimer) return false;

    pTimer->startTimer();

    feedTimerStarted(idTimer, pTimer->getDelay() * 1000);

    // Log
    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
        syslog(LOG_ERR, "[DM] Timer %d started.", idTimer);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// startTimer
//

int
CDM::startTimer(uint32_t idTimer,
                std::string &nameVariable,
                uint32_t delay,
                bool bSetValue,
                int reloadLimit)
{
    uint16_t rv = 0;

    dmTimer *pTimer = m_timerHash[idTimer];
    if (NULL == pTimer) {
        // Log
        if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
            syslog(LOG_ERR, "[DM] Timer %d created.", idTimer);
        }

        // Nonexistent timer - create it
        rv =
          addTimer(idTimer, nameVariable, delay, true, bSetValue, reloadLimit);
    } else {
        pTimer->startTimer();
        rv = 1;
    }

    // Log
    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
        syslog(LOG_ERR, "[DM] Timer %d started.", idTimer);
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// stopTimer
//

bool
CDM::stopTimer(uint32_t idTimer)
{
    dmTimer *pTimer = m_timerHash[idTimer];
    if (NULL == pTimer) return false;

    // Stop it
    pTimer->stopTimer();

    // Tell the world that it is stopped
    feedTimerStopped(idTimer, pTimer->getDelay());

    // Log
    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
        syslog(LOG_ERR, "[DM] Timer %d stopped.", idTimer);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// pauseTimer
//

bool
CDM::pauseTimer(uint32_t idTimer)
{
    dmTimer *pTimer = m_timerHash[idTimer];
    if (NULL == pTimer) return false;

    // Pause the timer
    pTimer->pauseTimer();

    // Tell the world it is paused
    feedTimerPaused(idTimer, pTimer->getDelay());

    // Log
    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
        syslog(LOG_ERR, "[DM] Timer %d pused.", idTimer);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// resumeTimer
//

bool
CDM::resumeTimer(uint32_t idTimer)
{
    dmTimer *pTimer = m_timerHash[idTimer];
    if (NULL == pTimer) return false;

    // Resume timer
    pTimer->resumeTimer();

    // Tell the world
    feedTimerResumed(idTimer, pTimer->getDelay());

    // Log
    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM_TIMERS) {
        syslog(LOG_ERR, "[DM] Timer %d resumed.", idTimer);
    }

    return true;
}

//*****************************************************************************
//                                Database
//*****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// doCreateDMTable
//

bool
CDM::doCreateDMTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_DM_CREATE;

    // Check if database is open
    if (NULL == m_db_vscp_dm) return false;

    pthread_mutex_lock(&m_mutexDM);

    if (SQLITE_OK != sqlite3_exec(m_db_vscp_dm, psql, NULL, NULL, &pErrMsg)) {
        pthread_mutex_unlock(&m_mutexDM);
        return false;
    }

    pthread_mutex_unlock(&m_mutexDM);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getDatabaseRecordCount
//
//

uint32_t
CDM::getDatabaseRecordCount(void)
{
    std::string str;
    sqlite3_stmt *ppStmt;
    uint32_t count = 0;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Get record count. Database file is not open.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare_v2(m_db_vscp_dm,
                           "SELECT count(*) FROM dm WHERE bEnable!=0;",
                           -1,
                           &ppStmt,
                           NULL)) {
        return false;
    }
    while (SQLITE_ROW == sqlite3_step(ppStmt)) {
        count = sqlite3_column_int(ppStmt, 0);
    }
    sqlite3_finalize(ppStmt);

    return count;
}

///////////////////////////////////////////////////////////////////////////////
// deleteDatabaseRecord
//
//

bool
CDM::deleteDatabaseRecord(uint32_t idx)
{
    char *pErrMsg;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Delete record. Database file is not open.");
        return false;
    }

    char *sql = sqlite3_mprintf("DELETE FROM \"dm\" "
                                "WHERE id='%d';",
                                idx);
    if (SQLITE_OK != sqlite3_exec(m_db_vscp_dm, sql, NULL, NULL, &pErrMsg)) {
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addDatabaseRecord
//
//

bool
CDM::addDatabaseRecord(dmElement *pdm)
{
    char *pErrMsg;
    cguid guid_mask;
    cguid guid_filter;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Add record. Database file is not open.");
        return false;
    }

    guid_mask.getFromArray(pdm->m_vscpfilter.mask_GUID);
    guid_filter.getFromArray(pdm->m_vscpfilter.filter_GUID);

    char *sql = sqlite3_mprintf(
      VSCPDB_DM_INSERT,
      (const char *)pdm->m_strGroupID.c_str(),
      pdm->m_bEnable ? 1 : 0,
      pdm->m_vscpfilter.mask_priority,
      pdm->m_vscpfilter.mask_class,
      pdm->m_vscpfilter.mask_type,
      (const char *)guid_mask.getAsString().c_str(),
      pdm->m_vscpfilter.filter_priority,
      pdm->m_vscpfilter.filter_class,
      pdm->m_vscpfilter.filter_type,
      (const char *)guid_filter.getAsString().c_str(),
      //--------------------------------------------------------------
      (const char *)pdm->m_timeAllow.getStartTime().getISODateTime().c_str(),
      (const char *)pdm->m_timeAllow.getEndTime().getISODateTime().c_str(),

      pdm->m_timeAllow.getWeekday(0) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(1) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(2) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(3) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(4) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(5) ? 1 : 0,
      //--------------------------------------------------------------
      pdm->m_timeAllow.getWeekday(6) ? 1 : 0,
      (const char *)pdm->m_timeAllow.getActionTimeAsString().c_str(),
      pdm->m_bCheckIndex ? 1 : 0,
      (int)pdm->m_index,
      pdm->m_bCheckZone ? 1 : 0,
      (int)pdm->m_zone,
      pdm->m_bCheckSubZone ? 1 : 0,
      (int)pdm->m_subzone,
      pdm->m_bCheckMeasurementIndex ? 1 : 0,
      //--------------------------------------------------------------
      (int)pdm->m_actionCode,
      (const char *)pdm->m_actionparam.c_str(),
      pdm->m_bCompareMeasurement ? 1 : 0,
      pdm->m_measurementValue,
      (int)pdm->m_measurementUnit,
      (int)pdm->m_measurementCompareCode,
      (const char *)pdm->m_comment.c_str());

    if (SQLITE_OK != sqlite3_exec(m_db_vscp_dm, sql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "[DM] addDatabaseRecord failed. SQL = %s, err = %s",
               sql,
               pErrMsg);
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateDatabaseRecord
//

bool
CDM::updateDatabaseRecord(dmElement *pdm)
{
    char *pErrMsg;
    cguid guid_mask;
    cguid guid_filter;

    // Can't save to db if loaded from XML
    if (pdm->m_bStatic) return false;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Add record. Database file is not open.");
        return false;
    }

    guid_mask.getFromArray(pdm->m_vscpfilter.mask_GUID);
    guid_filter.getFromArray(pdm->m_vscpfilter.filter_GUID);

    std::string dtFrom = pdm->m_timeAllow.getStartTime().getISODateTime();
    std::string dtEnd  = pdm->m_timeAllow.getEndTime().getISODateTime();

    char *sql = sqlite3_mprintf(
      VSCPDB_DM_UPDATE,
      (const char *)pdm->m_strGroupID.c_str(),
      pdm->m_bEnable ? 1 : 0,
      pdm->m_vscpfilter.mask_priority,
      pdm->m_vscpfilter.mask_class,
      pdm->m_vscpfilter.mask_type,
      (const char *)guid_mask.getAsString().c_str(),
      pdm->m_vscpfilter.filter_priority,
      pdm->m_vscpfilter.filter_class,
      pdm->m_vscpfilter.filter_type,
      (const char *)guid_filter.getAsString().c_str(),
      //--------------------------------------------------------------
      (const char *)dtFrom.c_str(),
      (const char *)dtFrom.c_str(),

      pdm->m_timeAllow.getWeekday(0) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(1) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(2) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(3) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(4) ? 1 : 0,
      pdm->m_timeAllow.getWeekday(5) ? 1 : 0,
      //--------------------------------------------------------------
      pdm->m_timeAllow.getWeekday(6) ? 1 : 0,
      (const char *)pdm->m_timeAllow.getActionTimeAsString().c_str(),
      pdm->m_bCheckIndex ? 1 : 0,
      pdm->m_index,
      pdm->m_bCheckZone,
      pdm->m_zone,
      pdm->m_bCheckSubZone,
      pdm->m_subzone,
      pdm->m_bCheckMeasurementIndex,
      //--------------------------------------------------------------
      pdm->m_actionCode,
      (const char *)pdm->m_actionparam.c_str(),
      pdm->m_bCompareMeasurement,
      pdm->m_measurementValue,
      pdm->m_measurementUnit,
      pdm->m_measurementCompareCode,
      (const char *)pdm->m_comment.c_str(),
      pdm->m_id // Where clause
    );

    if (SQLITE_OK != sqlite3_exec(m_db_vscp_dm, sql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "[DM] updateDatabaseRecord failed. SQL = %s error=%s",
               sql,
               pErrMsg);
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateDatabaseRecordItem
//

bool
CDM::updateDatabaseRecordItem(unsigned long id,
                              const std::string &strUpdateField,
                              const std::string &strUpdateValue)
{
    char *pErrMsg;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Update record item. Database file is not open.");
        return false;
    }

    char *sql = sqlite3_mprintf(VSCPDB_DM_UPDATE_ITEM,
                                (const char *)strUpdateField.c_str(),
                                (const char *)strUpdateValue.c_str(),
                                id);
    if (SQLITE_OK != sqlite3_exec(m_db_vscp_dm, sql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "[DM] updateDatabaseRecordItem failed. SQL = %s error=%s",
               sql,
               pErrMsg);
        sqlite3_free(sql);
        return false;
    }

    sqlite3_free(sql);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getDatabaseRecord
//
//

bool
CDM::getDatabaseRecord(uint32_t idx, dmElement *pDMitem)
{
    std::string str;
    sqlite3_stmt *ppStmt;

    // Check pointer
    if (NULL == pDMitem) return false;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Get record. Database file is not open.");
        return false;
    }

    char *sql = sqlite3_mprintf("SELECT * FROM dm WHERE id='%d';", idx);

    if (SQLITE_OK != sqlite3_prepare_v2(m_db_vscp_dm, sql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR, "[DM] getDatabaseRecord failed. SQL = %s", sql);
        sqlite3_free(sql);
        return false;
    }

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        const char *p;

        pDMitem->m_pDM = this; // Record owner

        // id in database
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ID))) {
            pDMitem->m_id = atol(p);
        } // bEnable
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ENABLE))) {
            pDMitem->m_bEnable = atoi(p) ? true : false;
        } // groupid
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_GROUPID))) {
            pDMitem->m_strGroupID = std::string(p);
        } // Mask priority
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_MASK_PRIORITY))) {
            pDMitem->m_vscpfilter.mask_priority = atoi(p);
        } // Filter priority
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_FILTER_PRIORITY))) {
            pDMitem->m_vscpfilter.filter_priority = atoi(p);
        } // Mask class
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_MASK_CLASS))) {
            pDMitem->m_vscpfilter.mask_class = atoi(p);
        } // Filter class
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_FILTER_CLASS))) {
            pDMitem->m_vscpfilter.filter_class = atoi(p);
        } // Mask type
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_MASK_TYPE))) {
            pDMitem->m_vscpfilter.mask_type = atoi(p);
        } // Filter type
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_FILTER_TYPE))) {
            pDMitem->m_vscpfilter.filter_type = atoi(p);
        } // Mask GUID
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_MASK_GUID))) {
            cguid guid;
            guid.getFromString(p);
            memcpy(pDMitem->m_vscpfilter.mask_GUID, guid.getGUID(), 16);
        }

        // Filter GUID
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_FILTER_GUID))) {
            cguid guid;
            guid.getFromString(p);
            memcpy(pDMitem->m_vscpfilter.filter_GUID, guid.getGUID(), 16);
        }

        // Allowed start
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_START))) {
            pDMitem->m_timeAllow.setStartTime(std::string(p));
        }

        // Allowed end
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_END))) {
            pDMitem->m_timeAllow.setEndTime(std::string(p));
        }

        // Allowed time
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TIME))) {
            pDMitem->m_timeAllow.parseActionTime(std::string(p));
        }

        // Allow Monday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_MONDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowMonday()
                    : pDMitem->m_timeAllow.allowMonday(false);
        }

        // Allow Tuesday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TUESDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowTuesday()
                    : pDMitem->m_timeAllow.allowTuesday(false);
        }

        // Allow Wednesday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_WEDNESDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowWednesday()
                    : pDMitem->m_timeAllow.allowWednesday(false);
        }

        // Allow Thursday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_THURSDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowThursday()
                    : pDMitem->m_timeAllow.allowThursday(false);
        }

        // Allow Friday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_FRIDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowFriday()
                    : pDMitem->m_timeAllow.allowFriday(false);
        }

        // Allow Saturday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SATURDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowSaturday()
                    : pDMitem->m_timeAllow.allowSaturday(false);
        }

        // Allow Sunday
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SUNDAY))) {
            atoi(p) ? pDMitem->m_timeAllow.allowSunday()
                    : pDMitem->m_timeAllow.allowSunday(false);
        }

        // bCheckIndex
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_CHECK_INDEX))) {
            pDMitem->m_bCheckIndex = atoi(p) ? true : false;
        } // Index
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_INDEX))) {
            pDMitem->m_index = atoi(p);
        } // bCheckZone
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_CHECK_ZONE))) {
            pDMitem->m_bCheckZone = atoi(p) ? true : false;
        } // Zone
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ZONE))) {
            pDMitem->m_zone = atoi(p);
        } // bCheckSubZone
        pDMitem->m_bCheckZone =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_CHECK_SUBZONE) ? true
                                                                      : false;

        // SubZone
        pDMitem->m_zone = sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_SUBZONE);

        // bMeasurementIndex
        pDMitem->m_bCheckMeasurementIndex =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_CHECK_MEASUREMENT_INDEX)
            ? true
            : false;

        // ActionCode
        pDMitem->m_actionCode =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_ACTIONCODE);

        // ActionParamter
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_DM_ACTIONPARAMETER))) {
            pDMitem->m_actionparam = std::string(p);
        }

        // Measurement check value flag
        pDMitem->m_bCompareMeasurement =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_CHECK_VALUE)
            ? true
            : false;

        // Measurement value
        pDMitem->m_measurementValue =
          sqlite3_column_double(ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE);

        // Measurement unit
        pDMitem->m_measurementUnit =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT);

        // Measurement compare code
        pDMitem->m_measurementCompareCode =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE);
    }

    sqlite3_finalize(ppStmt);
    sqlite3_free(sql);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadFromDatabase
//
// Read decision matrix from the database
//

bool
CDM::loadFromDatabase(void)
{
    std::string str;
    sqlite3_stmt *ppStmt;

    // Database file must be open
    if (NULL == m_db_vscp_dm) {
        syslog(LOG_ERR, "[DM] Loading. Database file is not open.");
        return false;
    }

    syslog(LOG_ERR,
           "[DM] Loading decision matrix from database : %s",
           m_path_db_vscp_dm.c_str());

    pthread_mutex_lock(&m_mutexDM);

    if (SQLITE_OK != sqlite3_prepare_v2(
                       m_db_vscp_dm, "SELECT * FROM dm;", -1, &ppStmt, NULL)) {
        pthread_mutex_unlock(&m_mutexDM);
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {
        // Set row default values for row
        dmElement *pDMitem = new dmElement;
        if (NULL != pDMitem) {
            const char *p;

            pDMitem->m_pDM = this; // Record owner

            // id in database
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ID))) {
                pDMitem->m_id = atol(p);
            } // bEnable
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ENABLE))) {
                pDMitem->m_bEnable = atoi(p) ? true : false;
            } // groupid
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_GROUPID))) {
                pDMitem->m_strGroupID = std::string(p);
            }

            // Mask priority
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_MASK_PRIORITY))) {
                pDMitem->m_vscpfilter.mask_priority = atoi(p);
            }

            // Filter priority
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_FILTER_PRIORITY))) {
                pDMitem->m_vscpfilter.filter_priority = atoi(p);
            } // Mask class
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_MASK_CLASS))) {
                pDMitem->m_vscpfilter.mask_class = atoi(p);
            } // Filter class
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_FILTER_CLASS))) {
                pDMitem->m_vscpfilter.filter_class = atoi(p);
            } // Mask type
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_MASK_TYPE))) {
                pDMitem->m_vscpfilter.mask_type = atoi(p);
            } // Filter type
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_FILTER_TYPE))) {
                pDMitem->m_vscpfilter.filter_type = atoi(p);
            } // Mask GUID
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_MASK_GUID))) {
                if ((NULL != p) && ('*' != *p)) {
                    cguid guid;
                    guid.getFromString(p);
                    memcpy(pDMitem->m_vscpfilter.mask_GUID, guid.getGUID(), 16);
                }
            }

            // Filter GUID
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_FILTER_GUID))) {
                if ((NULL != p) && ('*' != *p)) {
                    cguid guid;
                    guid.getFromString(p);
                    memcpy(
                      pDMitem->m_vscpfilter.filter_GUID, guid.getGUID(), 16);
                }
            }

            // Allowed from
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_START))) {
                if ((NULL != p) && ('*' != *p)) {
                    pDMitem->m_timeAllow.setStartTime(std::string(p));
                }
            }

            // Allowed to
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_END))) {
                if ((NULL != p) && ('*' != *p)) {
                    pDMitem->m_timeAllow.setEndTime(std::string(p));
                }
            }

            // Allowed time
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TIME))) {
                if ((NULL != p) && ('*' != *p)) {
                    pDMitem->m_timeAllow.parseActionTime(std::string(p));
                }
            }

            // Allow Monday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_MONDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowMonday()
                        : pDMitem->m_timeAllow.allowMonday(false);
            }

            // Allow Tuesday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_TUESDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowTuesday()
                        : pDMitem->m_timeAllow.allowTuesday(false);
            }

            // Allow Wednesday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_WEDNESDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowWednesday()
                        : pDMitem->m_timeAllow.allowWednesday(false);
            }

            // Allow Thursday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_THURSDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowThursday()
                        : pDMitem->m_timeAllow.allowThursday(false);
            }

            // Allow Friday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_FRIDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowFriday()
                        : pDMitem->m_timeAllow.allowFriday(false);
            }

            // Allow Saturday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SATURDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowSaturday()
                        : pDMitem->m_timeAllow.allowSaturday(false);
            }

            // Allow Sunday
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ALLOWED_SUNDAY))) {
                atoi(p) ? pDMitem->m_timeAllow.allowSunday()
                        : pDMitem->m_timeAllow.allowSunday(false);
            }

            // bCheckIndex
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_CHECK_INDEX))) {
                pDMitem->m_bCheckIndex = atoi(p) ? true : false;
            } // Index
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_INDEX))) {
                pDMitem->m_index = atoi(p);
            } // bCheckZone
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_CHECK_ZONE))) {
                pDMitem->m_bCheckZone = atoi(p) ? true : false;
            } // Zone
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ZONE))) {
                pDMitem->m_zone = atoi(p);
            } // bCheckSubZone
            pDMitem->m_bCheckZone =
              sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_CHECK_SUBZONE)
                ? true
                : false;

            // SubZone
            pDMitem->m_zone =
              sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_SUBZONE);

            // bMeasurementIndex
            pDMitem->m_bCheckMeasurementIndex =
              sqlite3_column_int(ppStmt,
                                 VSCPDB_ORDINAL_DM_CHECK_MEASUREMENT_INDEX)
                ? true
                : false;

            // ActionCode
            pDMitem->m_actionCode =
              sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_ACTIONCODE);

            // ActionParamter
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_ACTIONPARAMETER))) {
                pDMitem->m_actionparam = std::string(p);
            }

            // Measurement check value flag
            pDMitem->m_bCompareMeasurement =
              sqlite3_column_int(ppStmt,
                                 VSCPDB_ORDINAL_DM_MEASUREMENT_CHECK_VALUE)
                ? true
                : false;

            // Measurement value
            pDMitem->m_measurementValue = sqlite3_column_double(
              ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE);

            // Measurement unit
            pDMitem->m_measurementUnit =
              sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT);

            // Measurement compare code
            pDMitem->m_measurementCompareCode =
              sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE);

            // Comment
            if (NULL != (p = (const char *)sqlite3_column_text(
                           ppStmt, VSCPDB_ORDINAL_DM_COMMENT))) {
                pDMitem->m_comment = std::string(p);
            }

            // Add the DM row to the matrix
            addMemoryElement(pDMitem);
        }
    }

    sqlite3_finalize(ppStmt);

    pthread_mutex_unlock(&m_mutexDM);

    return true;
}

// ----------------------------------------------------------------------------

static int depth_dm_parser     = 0;
static char *last_dm_content   = NULL;
static dmElement *pnew_dm_item = NULL;

static void
startDMParser(void *data, const char *name, const char **attr)
{
    CDM *pdm = (CDM *)data;
    if (NULL == pdm) return;

    // New tag - Start new data collect
    if (NULL != last_dm_content) {
        free(last_dm_content);
        last_dm_content = NULL;
    }

    if ((0 == vscp_strcasecmp(name, "dm")) && (0 == depth_dm_parser)) {
        // just increase the depth
    } else if ((0 == vscp_strcasecmp(name, "row")) && (1 == depth_dm_parser)) {

        // New row
        if (NULL != pnew_dm_item) {
            delete pnew_dm_item;
            pnew_dm_item = NULL;
        }

        pnew_dm_item = (dmElement *)new dmElement;
        if (NULL == pnew_dm_item) return;

        //memset(pnew_dm_item, 0, sizeof(dmElement));
        pnew_dm_item->m_bStatic = true; // Can not be edited/saved
        pnew_dm_item->m_bEnable = false;

        for (int i = 0; attr[i]; i += 2) {
            std::string attribute = attr[i + 1];
            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                vscp_trim(attribute);
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pnew_dm_item->enableRow();
                } else {
                    pnew_dm_item->disableRow();
                }
            } else if (0 == vscp_strcasecmp(attr[i], "groupid")) {
                vscp_trim(attribute);
                if (attribute.length()) {
                    pnew_dm_item->m_strGroupID = attribute;
                }
            }
        }
    } else if ((0 == vscp_strcasecmp(name, "comment")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "mask")) && (2 == depth_dm_parser)) {
        for (int i = 0; attr[i]; i += 2) {
            std::string attribute = attr[i + 1];
            if (0 == vscp_strcasecmp(attr[i], "priority")) {
                pnew_dm_item->m_vscpfilter.mask_priority =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "class")) {
                pnew_dm_item->m_vscpfilter.mask_class =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "type")) {
                pnew_dm_item->m_vscpfilter.mask_type =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "GUID")) {
                vscp_getGuidFromStringToArray(
                  pnew_dm_item->m_vscpfilter.mask_GUID, attribute);
            }
        }
    } else if ((0 == vscp_strcasecmp(name, "filter")) &&
               (2 == depth_dm_parser)) {
        for (int i = 0; attr[i]; i += 2) {
            std::string attribute = attr[i + 1];
            if (0 == vscp_strcasecmp(attr[i], "priority")) {
                pnew_dm_item->m_vscpfilter.filter_priority =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "class")) {
                pnew_dm_item->m_vscpfilter.filter_class =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "type")) {
                pnew_dm_item->m_vscpfilter.filter_type =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "GUID")) {
                vscp_getGuidFromStringToArray(
                  pnew_dm_item->m_vscpfilter.filter_GUID, attribute);
            }
        }
    } else if ((0 == vscp_strcasecmp(name, "action")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "param")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "allowed_from")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "allowed_to")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "allowed_weekdays")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "allowed_time")) &&
               (2 == depth_dm_parser)) {
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "index")) &&
               (2 == depth_dm_parser)) {
        pnew_dm_item->m_bCheckIndex = true;

        for (int i = 0; attr[i]; i += 2) {
            std::string attribute = attr[i + 1];
            if (0 == vscp_strcasecmp(attr[i], "measurementindex")) {
                vscp_trim(attribute);
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pnew_dm_item->m_bCheckMeasurementIndex = true;
                } else {
                    pnew_dm_item->m_bCheckMeasurementIndex = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "zone")) {
                pnew_dm_item->m_bCheckZone = true;
                pnew_dm_item->m_zone       = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "subzone")) {
                pnew_dm_item->m_bCheckSubZone = true;
                pnew_dm_item->m_subzone       = vscp_readStringValue(attribute);
            }
        }
    } else if ((0 == vscp_strcasecmp(name, "zone")) && (2 == depth_dm_parser)) {
        pnew_dm_item->m_bCheckZone = true;
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "subzone")) &&
               (2 == depth_dm_parser)) {
        pnew_dm_item->m_bCheckSubZone = true;
        // Picked up in data part
    } else if ((0 == vscp_strcasecmp(name, "measurement")) &&
               (2 == depth_dm_parser)) {

        pnew_dm_item->m_bCompareMeasurement = true;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            if (0 == vscp_strcasecmp(attr[i], "compare")) {
                vscp_trim(attribute);
                pnew_dm_item->m_measurementCompareCode =
                  pdm->getCompareCodeFromToken(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "unit")) {
                vscp_trim(attribute);
                pnew_dm_item->m_measurementUnit =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "value")) {
                vscp_trim(attribute);
                pnew_dm_item->m_measurementValue = std::stod(attribute);
            }
        }
    }

    depth_dm_parser++;
}

static void
handle_dm_data(void *data, const char *content, int length)
{
    CDM *pdm = (CDM *)data;
    if (NULL == pdm) return;

    if (NULL == last_dm_content) {
        last_dm_content = (char *)malloc(length + 1);
        strncpy(last_dm_content, content, length);
        last_dm_content[length] = '\0';
    } else {
        // Concatenate
        int newlen = length + 1 + strlen(last_dm_content);
        char *p    = (char *)realloc(last_dm_content, newlen);
        if (NULL == p) {
            syslog(LOG_ERR, "Memory allocation failure");
            free(last_dm_content);
            last_dm_content = NULL;
            return;
        }
        last_dm_content = p;
        strncat(last_dm_content, content, length);
    }
}

static void
endDMParser(void *data, const char *name)
{
    std::string str;
    CDM *pdm = (CDM *)data;
    if (NULL == pdm) return;

    depth_dm_parser--;

    // If no data there is nothing more to do
    if (NULL != last_dm_content) {
        str = last_dm_content;
        vscp_trim(str);
        // Free the allocated data
        free(last_dm_content);
        last_dm_content = NULL;
    }

    if ((0 == vscp_strcasecmp(name, "row")) && (1 == depth_dm_parser)) {
        // add the DM row to the matrix
        pdm->addMemoryElement(pnew_dm_item);
        pnew_dm_item = NULL;
    } else if ((0 == vscp_strcasecmp(name, "comment")) &&
               (2 == depth_dm_parser)) {
        vscp_trim(str);
        if (str.length()) {
            pnew_dm_item->m_comment = str;
        }
    } else if ((0 == vscp_strcasecmp(name, "action")) &&
               (2 == depth_dm_parser)) {
        pnew_dm_item->m_actionCode = vscp_readStringValue(str);
    } else if ((0 == vscp_strcasecmp(name, "zone")) && (2 == depth_dm_parser)) {
        pnew_dm_item->m_zone = vscp_readStringValue(str);
    } else if ((0 == vscp_strcasecmp(name, "subzone")) &&
               (2 == depth_dm_parser)) {
        pnew_dm_item->m_subzone = vscp_readStringValue(str);
    } else if ((0 == vscp_strcasecmp(name, "param")) &&
               (2 == depth_dm_parser)) {
        vscp_trim(str);
        if (str.length()) {
            pnew_dm_item->m_actionparam = str;
        }
    } else if ((0 == vscp_strcasecmp(name, "allowed_from")) &&
               (2 == depth_dm_parser)) {
        vscp_trim(str);
        if (0 != str.length()) {
            pnew_dm_item->m_timeAllow.setStartTime(str);
        } else {
            pnew_dm_item->m_timeAllow.setStartTime("*");
        }
    } else if ((0 == vscp_strcasecmp(name, "allowed_to")) &&
               (2 == depth_dm_parser)) {
        vscp_trim(str);
        if (0 != str.length()) {
            pnew_dm_item->m_timeAllow.setEndTime(str);
        } else {
            pnew_dm_item->m_timeAllow.setEndTime("*");
        }
    } else if ((0 == vscp_strcasecmp(name, "allowed_weekdays")) &&
               (2 == depth_dm_parser)) {
        vscp_trim(str);
        if ("*" == str) str = "mtwtfss";
        if (0 != str.length()) {
            pnew_dm_item->m_timeAllow.setWeekDays(str);
        } else {
            pnew_dm_item->m_timeAllow.setWeekDays("mtwtfss");
        }
    } else if ((0 == vscp_strcasecmp(name, "allowed_time")) &&
               (2 == depth_dm_parser)) {
        vscp_trim(str);
        if ("*" == str) str = "*-*-* *:*:*";
        if (0 != str.length()) {
            pnew_dm_item->m_timeAllow.parseActionTime(str);
        } else {
            pnew_dm_item->m_timeAllow.parseActionTime("*-*-* *:*:*");
        }
    }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// loadFromXML
//
// Read decision matrix from file
//

bool
CDM::loadFromXML(void)
{
    syslog(LOG_DEBUG,
           "[DM] Loading decision matrix from: %s",
           m_staticXMLPath.c_str());

    // File must exist
    if (!vscp_fileExists(m_staticXMLPath)) {
        syslog(LOG_ERR,
               "[DM] Decision matrix file does not exist. %s",
               m_staticXMLPath.c_str());
        return false;
    }

    FILE *fpxml;
    fpxml = fopen(m_staticXMLPath.c_str(), "r");
    if (NULL == fpxml) {
        syslog(LOG_ERR,
               "[DM] Failed to open decision matrix file [%s]",
               m_staticXMLPath.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(xmlParser, startDMParser, endDMParser);
    XML_SetCharacterDataHandler(xmlParser, handle_dm_data);

    int bytes_read;
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fpxml);
        syslog(LOG_ERR,
               "[DM ]Failed to allocate buffer for decision matrix "
               "file [%s]",
               m_staticXMLPath.c_str());
        return false;
    };

    size_t file_size = 0;
    file_size        = fread(buf, sizeof(char), XML_BUFF_SIZE, fpxml);

    if (!XML_ParseBuffer(xmlParser, file_size, file_size == 0)) {
        syslog(LOG_ERR,
               "[DM] Failed to parse DM XML file %s",
               m_staticXMLPath.c_str());
        fclose(fpxml);
        XML_ParserFree(xmlParser);
        return false;
    }

    fclose(fpxml);
    XML_ParserFree(xmlParser);

    if (gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM) {
        syslog(LOG_DEBUG, "[DM]  Read success.");
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// saveToXML
//
// Write decision matrix to file
//

bool
CDM::saveToXML(bool bAll)
{
    std::string strLog;
    std::string buf;

    syslog(
      LOG_INFO, "[DM] Saving decision matrix to: %s", m_staticXMLPath.c_str());

    // Make a copy before we save
    // xxCopyFile( m_staticXMLPath, m_staticXMLPath + "~" );  TODO
    std::ifstream src(m_staticXMLPath, std::ios::binary);
    std::ofstream dst(m_staticXMLPath + "~", std::ios::binary);
    dst << src.rdbuf();

    std::ofstream fxml;
    fxml;

    try {
        fxml.open(m_staticXMLPath.c_str());
    } catch (...) {
        syslog(LOG_ERR,
               "[DM] Can not open file [%s] for writing.",
               m_staticXMLPath.c_str());
        return false;
    }

    if (!fxml.is_open()) {
        syslog(LOG_ERR,
               "[DM] Can not open file [%s] for writing.",
               m_staticXMLPath.c_str());
        return false;
    }

    try {

        fxml << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>";

        pthread_mutex_lock(&m_mutexDM);

        // DM matrix information start
        fxml << "<dm>";

        std::deque<dmElement *>::iterator it;
        for (it = m_DMList.begin(); it != m_DMList.end(); ++it) {
            dmElement *pDMitem = *it;

            if (NULL != pDMitem) { // Must be an dmElement to work with
                                   // m_strGroupID

                // Static records is loaded from XML file and if bAll is
                // false only static records should be saved.
                if (!bAll && !pDMitem->m_bStatic) continue;

                fxml << "  <row enable=\"";
                if (pDMitem->isEnabled()) {
                    fxml << "true\" ";
                } else {
                    fxml << "false\" ";
                }

                fxml << "groupid=\"";
                fxml << pDMitem->m_strGroupID;
                fxml << "\" >";

                fxml << "    <mask ";
                fxml << vscp_str_format(" priority=\"%d\" ",
                                           pDMitem->m_vscpfilter.mask_priority);

                buf = vscp_str_format(" class=\"%d\" ",
                                         pDMitem->m_vscpfilter.mask_class);
                fxml << buf;

                buf = vscp_str_format(" type=\"%d\" ",
                                         pDMitem->m_vscpfilter.mask_type);
                fxml << buf;

                fxml << " GUID=\" ";
                std::string strGUID;
                vscp_writeGuidArrayToString(pDMitem->m_vscpfilter.mask_GUID,
                                            strGUID);
                fxml << strGUID;
                fxml << "\" > ";
                fxml << "</mask>";

                fxml << "    <filter ";
                buf = vscp_str_format(" priority=\"%d\" ",
                                         pDMitem->m_vscpfilter.filter_priority);
                fxml << buf;
                buf = vscp_str_format(" class=\"%d\" ",
                                         pDMitem->m_vscpfilter.filter_class);
                fxml << buf;
                buf = vscp_str_format(" type=\"%d\" ",
                                         pDMitem->m_vscpfilter.filter_type);
                fxml << buf;
                buf = vscp_str_format(" GUID=\" ");
                fxml << buf;
                vscp_writeGuidArrayToString(pDMitem->m_vscpfilter.filter_GUID,
                                            strGUID);
                fxml << strGUID;
                fxml << "\" > ";
                fxml << "</filter>";

                fxml << "    <action>";
                buf = vscp_str_format("0x%x", pDMitem->m_actionCode);
                fxml << buf;
                fxml << "</action>";

                fxml << "    <param>";
                fxml << pDMitem->m_actionparam;
                fxml << "</param>";

                fxml << "    <comment>";
                fxml << pDMitem->m_comment;
                fxml << "</comment>";

                fxml << "    <allowed_from>";
                {
                    std::string str =
                      pDMitem->m_timeAllow.getStartTime().getISODateTime();
                    fxml << str;
                }
                fxml << "</allowed_from>";

                fxml << "    <allowed_to>";
                {
                    std::string str =
                      pDMitem->m_timeAllow.getEndTime().getISODateTime();
                    fxml << str;
                }
                fxml << "</allowed_to>";

                fxml << "    <allowed_weekdays>";
                {
                    std::string str = pDMitem->m_timeAllow.getWeekDays();
                    fxml << str;
                }
                fxml << "</allowed_weekdays>";

                fxml << "    <allowed_time>";
                {
                    std::string str =
                      pDMitem->m_timeAllow.getActionTimeAsString();
                    fxml << str;
                }
                fxml << "</allowed_time>";

                // Index
                fxml << "    <index ";
                buf = vscp_str_format(
                  " bCheckMeasurementIndex=\"%s\" ",
                  (pDMitem->m_bCheckMeasurementIndex) ? "true" : "false");
                fxml << buf;
                fxml << " > ";
                buf = vscp_str_format("%d", pDMitem->m_index);
                fxml << buf;
                fxml << "</index>";

                // Zone
                fxml << "    <zone>";
                buf = vscp_str_format("%d", pDMitem->m_zone);
                fxml << buf;
                fxml << "</zone>";

                // Subzone
                fxml << "    <subzone>";
                buf = vscp_str_format("%d", pDMitem->m_subzone);
                fxml << buf;
                fxml << "</subzone>";

                // measurement
                fxml << "    <measurement ";
                if (pDMitem->m_bCompareMeasurement) {
                    fxml << "enable=\"true\" ";
                } else {
                    fxml << "enable=\"false\" ";
                }

                std::string strCode =
                  getTokenFromCompareCode(pDMitem->m_measurementCompareCode);
                buf = "compare=\"" + strCode + "\" ";
                fxml << buf;

                buf = vscp_str_format("unit=\"%d\" ",
                                         pDMitem->m_measurementUnit);
                fxml << buf;

                buf = vscp_str_format("value=\"%lf\" ",
                                         pDMitem->m_measurementValue);
                fxml << buf;

                fxml << " />";

                fxml << "  </row>\n";
            }
        }

        // DM matrix information end
        fxml << "</dm>";

        pthread_mutex_unlock(&m_mutexDM);
    } catch (...) {
        syslog(
          LOG_ERR, "[DM] Failed to write file [%s].", m_staticXMLPath.c_str());
        fxml.close();
        return false;
    }

    // Close the file
    fxml.close();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// feed
//

bool
CDM::feed(vscpEvent *pEvent)
{
    pthread_mutex_lock(&m_mutexDM);

    std::deque<dmElement *>::iterator it;
    for (it = m_DMList.begin(); it != m_DMList.end(); ++it) {
        dmElement *pDMitem = *it;

        // Skip if disabled
        if (!pDMitem->isEnabled()) continue;

        if (vscp_doLevel2Filter(pEvent, &pDMitem->m_vscpfilter) &&
            pDMitem->m_timeAllow.ShouldWeDoAction()) {
            if (pDMitem->isCheckIndexSet()) {
                if (pDMitem->m_bCheckMeasurementIndex) {
                    if ((0 == pEvent->sizeData) ||
                        ((pEvent->pdata[0] & 7) != pDMitem->m_index))
                        continue;
                } else {
                    if ((0 == pEvent->sizeData) ||
                        (pEvent->pdata[0] != pDMitem->m_index))
                        continue;
                }
            }

            if (pDMitem->isCheckZoneSet()) {
                if ((2 > pEvent->sizeData) ||
                    (pEvent->pdata[1] != pDMitem->m_zone))
                    continue;
            }

            if (pDMitem->isCheckSubZoneSet()) {
                if ((3 > pEvent->sizeData) ||
                    (pEvent->pdata[2] != pDMitem->m_subzone))
                    continue;
            }

            // Check if measurement value should be compared.
            if (pDMitem->isCompareMeasurementSet()) {
                // Must be a measurement event
                if (!vscp_isMeasurement(pEvent)) continue;

                // Unit must be same
                if (pDMitem->m_measurementUnit !=
                    vscp_getMeasurementUnit(pEvent))
                    continue;

                double value;
                if (!vscp_getMeasurementAsDouble(pEvent, &value)) {
                    std::string strEvent;
                    vscp_convertEventExToEvent(pEvent, strEvent);
                    syslog(LOG_ERR,
                           "[DM] Conversion to double failed for "
                           "measurement. Event= %s",
                           strEvent.c_str());
                    continue;
                }

                switch (pDMitem->m_measurementCompareCode) {
                    case DM_MEASUREMENT_COMPARE_NOOP:
                        // We always accept
                        break;

                    case DM_MEASUREMENT_COMPARE_EQ:
                        if (!vscp_almostEqualRelativeDouble(
                              value, pDMitem->m_measurementValue))
                            continue;
                        break;

                    case DM_MEASUREMENT_COMPARE_NEQ:
                        if (vscp_almostEqualRelativeDouble(
                              value, pDMitem->m_measurementValue))
                            continue;
                        break;

                    case DM_MEASUREMENT_COMPARE_LT:
                        if (vscp_almostEqualRelativeDouble(
                              value, pDMitem->m_measurementValue) ||
                            (value > pDMitem->m_measurementValue))
                            continue;
                        break;

                    case DM_MEASUREMENT_COMPARE_LTEQ:
                        if (value > pDMitem->m_measurementValue) continue;
                        break;

                    case DM_MEASUREMENT_COMPARE_GT:
                        if (vscp_almostEqualRelativeDouble(
                              value, pDMitem->m_measurementValue) ||
                            (value < pDMitem->m_measurementValue))
                            continue;
                        break;

                    case DM_MEASUREMENT_COMPARE_GTEQ:
                        if (value < pDMitem->m_measurementValue) continue;
                        break;
                }
            }

            // Match do action for this row
            pDMitem->doAction(pEvent);
        }
    }

    pthread_mutex_unlock(&m_mutexDM);

    return true;
}

//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// actionThreadURL
//
// This thread connects to a HTTP server on a specified port
//

actionThread_URL::actionThread_URL(CControlObject *pCtrlObject,
                                   std::string &url,
                                   accessmethod_t nAccessMethod,
                                   std::string &putdata,
                                   std::string &extraheaders,
                                   std::string &proxy)
{
    /* TODO
    m_bOK = true;

    //OutputDebugString( "actionThreadURL: Create";

    // Set URL
    if (  xxURL_NOERR != m_url.SetURL( url ) ) {
        m_bOK = false;
    }

    m_extraheaders.empty();

    int pos;
    std::string strwrk = extraheaders;
    while ( str.npos != ( pos = strwrk.find('\\') ) )
    {
        m_extraheaders += strwrk.substr( pos );
        m_extraheaders += "\r";
        strwrk = strwrk.substr( strwrk.length()-pos-2 );
    }

    m_acessMethod = nAccessMethod;
    m_putdata = putdata;

    m_url.SetProxy( proxy );
  */
}

actionThread_URL::~actionThread_URL()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// http://www.jmarshall.com/easy/http/
//

void *
actionThread_URL(void *pData)
{
    // m_pCtrlObject->logMsg ( "[DM] " + ( "TCP actionThreadURL: Quit." ),
    //                            DAEMON_LOGMSG_INFO );
    /* TODO
    xxIPV4address addr;
    xxSocketClient sock;

    addr.Hostname( m_url.GetServer() );
    addr.Service( m_url.GetPort() );

    if ( sock.Connect( addr ) ) {

        std::string str;
        std::string xxwork;

        // Check if access method is GET
        if ( actionThread_URL::GET == m_acessMethod ) {

            str = "GET ";
            str += m_url.GetPath();
            str += "?";
            str += m_url.GetQuery();
            str += " ";
            str += "HTTP/1.1\r";
            str += "Host: ";
            str += m_url.GetServer();
            str += ":";
            str += m_url.GetPort();
            str += "\r";
            str += vscp_str_format( "User-Agent: VSCPD/%s\r",
                                        VSCPD_DISPLAY_VERSION );

            // Add extra headers if there are any
            if ( m_extraheaders.length() ) {
                str += m_extraheaders;
            }
            str += "\r\n\r";

        }
        // OK the access method is POST
        else if ( actionThread_URL::POST == m_acessMethod ) {

            str = "POST ";
            str += m_url.GetPath();
            str += " ";
            str += "HTTP/1.1\r";
            str += "Host: ";
            str += m_url.GetServer();
            str += ":";
            str += m_url.GetPort();
            str += "\r";
            str += vscp_str_format( "User-Agent: VSCPD/%s\r",
                                        VSCPD_DISPLAY_VERSION );
            // Add extra headers if there are any
            if ( m_extraheaders.length() ) {
                str += m_extraheaders;
            }

            str += "Accept: */
    /*\r";
            str += "Content-Type: application/x-www-form-urlencoded\r";
            str += "Content-Length: ";
            str += vscp_str_format("%ld",m_putdata.length());
            str += "\r\n\r";
            str += m_putdata;
            str += "\r";

        }
        // OK the access method is PUT
        else if ( actionThread_URL::PUT == m_acessMethod ) {

            str = "PUT ";
            str += m_url.GetPath();
            str += " ";
            str += "HTTP/1.1\r";
            str += "Host: ";
            str += m_url.GetServer();
            str += ":";
            str += m_url.GetPort();
            str += "\r";
            str += vscp_str_format( "User-Agent: VSCPD/%s\r",
                                        VSCPD_DISPLAY_VERSION );

            // Add extra headers if there are any
            if ( m_extraheaders.length() ) {
                str += m_extraheaders;
            }

            str += "Content-Type: application/x-www-form-urlencoded\r";
            str += "Content-Length: ";
            str += vscp_str_format("%ld",m_putdata.length());
            str += "\r\n\r";
            str += m_putdata;
            str += "\r";

        }
        else {

            // Invalid method
            syslog(  LOG_ERR, "[DM] " + ( "actionThreadURL: Invalid http "
                                           "access method: " ) +
                                m_url.GetServer() +
                                "," +
                                m_url.GetPort() +
                                "," +
                                m_url.GetPath() +
                                "," +
                                vscp_str_format( "acessMethod = %d" ),
                                                  m_acessMethod ) +
                                " "   );

        }

        if ( gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM ) {
            syslog(  LOG_ERR, "[DM] " + ( "actionThreadURL: Request: " ) +
                            str  );
        }

        // Send the request
        sock.Write( str, str.length() );

        if ( sock.Error() || (  str.length() != sock.LastWriteCount() ) ) {

            // There was an error
            gpobj->logMsg("[DM] " + ( "actionThreadURL: Error writing
    request: "
    )
    + m_url.GetServer() +
                                "," +
                                m_url.GetPort() +
                                "," +
                                m_url.GetPath() +
                                "," +
                                vscp_str_format( "acessMethod = %d" ),
    m_acessMethod ) + " "   );
        }

        // Get the response
        char buffer[ 8192 ];
        std::string strReponse;

        //while ( !sock.Read( buffer, sizeof( buffer ) ).LastReadCount() );
        sock.Read( buffer, sizeof( buffer ) );
        if ( !sock.Error() ) {

            // OK, Check the response
            strReponse = std::string( buffer );

            // Log response
            if ( gpobj->m_debugFlags[0] & VSCP_DEBUG1_DM ) {
                syslog(  LOG_ERR, "[DM] " + ( "actionThreadURL: OK Response:
    " )
    + strReponse + " "  );
            }

              std::deque<std::string> tokens;
              vscp_split(tokens, strReponse, ";");

            if ( !tokens.empty() ) {

                std::string str = tokens.front();
                tokens.pop_front();
                if ( str.npos != str.find"OK" ) ) {

                    // Something is wrong
                    syslog(  LOG_ERR, "[DM] " + ( "actionThreadURL: Error
    reading response: " ) + m_url.GetServer() +
                                        "," +
                                        m_url.GetPort() +
                                        "," +
                                        m_url.GetPath() +
                                        "," +
                                        vscp_str_format( "acessMethod =
    %d"
    ), m_acessMethod ) +
                                        ", Response = " +
                                        strReponse +
                                        " "   );

                }
            }

        }
        else {
            // There was an error
            gpobj->logMsg ( "[DM] " + ( "actionThreadURL: Error reading
    response: " ) + m_url.GetServer() +
                                "," +
                                m_url.GetPort() +
                                "," +
                                m_url.GetPath() +
                                "," +
                                ( m_acessMethod ? "PUT" : "GET" ) +
                                " "  );
        }

        // Close the socket
        sock.Close();

    }
    else {
        // There was an error connecting
        syslog(  LOG_ERR, "[DM] " + ( "actionThreadURL: Unable to connect: "
    ) + m_url.GetServer() +
                                "," +
                                m_url.GetPort() +
                                "," +
                                m_url.GetPath() +
                                "," +
                                vscp_str_format( "acessMethod = %d" ),
    m_acessMethod ) + " "   );
    }
  */
    return NULL;
}

//------------------------------------------------------------------------------
//                                 VSCP server
//------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// actionThread_VSCPSrv
//
// This thread connects to remote server and send event.
//

actionThread_VSCPSrv::actionThread_VSCPSrv(CControlObject *pCtrlObject,
                                           std::string &strHostname,
                                           short port,
                                           std::string &strUsername,
                                           std::string &strPassword,
                                           std::string &strEvent)
{
    // OutputDebugString( "actionThreadURL: Create";
    m_strHostname = strHostname;
    m_port        = port;
    m_strUsername = strUsername;
    m_strPassword = strPassword;
    vscp_convertStringToEventEx(&m_eventEx, strEvent);
}

actionThread_VSCPSrv::~actionThread_VSCPSrv() {}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
//

#if 0
void *
actionThread_VSCPSrv(void *pData)
{
    int rv;
    int tries;
    VscpRemoteTcpIf client;
    // TODO
    //std::string interface = vscp_str_format(
    //  "tcp://%s:%d", (const char *)m_strHostname.c_str(), m_port);
    // Allow for **VERY** slower clients
    client.setResponseTimeout(5000);

    tries = 3;
    while (true) {
        // TODO
        if (CANAL_ERROR_SUCCESS ) { //==
        //    (rv = client.doCmdOpen(interface, m_strUsername, m_strPassword))) {
            break;
        } else {
            if (CANAL_ERROR_TIMEOUT != rv) {
                goto connect_error;
            }

            tries--;
            if (tries > 0) {
                sleep(1);
                continue;
            }

        connect_error:
            // Failed to connect
            syslog(LOG_ERR,
                   "[DM] " +
                     ("actionThreadVSCPSrv: Unable to connect to "
                      "remote "
                      "server : %s "
                      "Return code = %d (%d) ",
                     m_strHostname.c_str(), rv, tries );
            return NULL;
        }
    } // while

    // Connected - Send the event

    tries = 3;
    while (true) {
        if (CANAL_ERROR_SUCCESS /*== client.doCmdSendEx(&m_eventEx) TODO*/) {
            break;
        } else {
            tries--;
            if (tries > 0) {
                sleep(1);
                continue;
            }

            // Failed to send event
            /* TODO syslog(LOG_ERR,
                   "[DM] actionThreadVSCPSrv: Unable to send event to "
                   "remote server : ",
                   m_strHostname.c_str());*/
            break;
        }

    } // while

    if (CANAL_ERROR_SUCCESS != client.doCmdClose() {
        // Failed to send event
        /* TODO syslog(LOG_ERR,
               "[DM] actionThreadVSCPSrv: Unable to close "
               "connection to remote "
               "server : ",
               m_strHostname.c_str()); */
    }

    return NULL;
}
#endif

//------------------------------------------------------------------------------
//                                     TABLE
//------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// actionThread_Table
//
// This thread handles a table action
//

actionThread_Table::actionThread_Table(std::string &strParam, vscpEvent *pEvent)
{
    m_pFeedEvent = NULL;
    m_strParam   = strParam;

    if (NULL != pEvent) {
        m_pFeedEvent = new vscpEvent;
        if (NULL != m_pFeedEvent) {
            vscp_copyEvent(m_pFeedEvent, pEvent);
        }
    }
}

actionThread_Table::~actionThread_Table()
{
    if (NULL != m_pFeedEvent) {
        vscp_deleteEvent(m_pFeedEvent);
        m_pFeedEvent = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Actionparameter:
//      0;tablename;datetime;value[;[BASE64:]sql-expression]
//      1;tablename;[BASE64:]sql-expression

#if 0
void *
actionThread_Table(void *pData)
{
    CVSCPTable *pTable; // Table object
    std::string strErr;
    vscpdatetime dt = vscpdatetime::Now();
    double value    = 0;
    int type;         // Action parameter type
    std::string name; // Table name
    std::string sql;  // sql expression

    std::deque<std::string> tokens;
    vscp_split(tokens, m_strParam, ";");

    if (tokens.empty()) {
        // Strange action parameter
        syslog(LOG_ERR,
               "[DM] [Action] Write Table: Action parameter is not correct. "
               "Parameter=%s",
               m_strParam.c_str());
        return NULL;
    }

    // Get action parameter format
    type = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    if (tokens.empty()) {
        // Missing table name
        syslog(LOG_ERR,
               "[DM] [Action] Write Table: Action parameter is not "
               "correct. No tablename. Parameter= %s",
               m_strParam.c_str());
        return NULL;
    }

    // Get table name
    name = tokens.front();
    tokens.pop_front();

    // Get table object
    if (NULL == (pTable = gpobj->m_userTableObjects.getTable(name))) {
        syslog(LOG_ERR,
               "[DM] [Action] Write Table: A table with that name was not "
               "found. Parameter= %s",
               m_strParam.c_str());
        return NULL;
    }

#if 0 // Test of getNumberOfRecordsForRange
    vscpdatetime from,to;
    from.set"0000-01-01 00:00:00";
    to.set"9999-06-01 00:00:00";
    double count;
    pTable->getNumberOfRecordsForRange( from, to, &count );
#endif

#if 0 // Test of getxxxxxValue
    vscpdatetime from,to;
    from.set"0000-01-01 00:00:00";
    to.set"9999-06-01 00:00:00";
    double val;
    //pTable->getSumValue( from, to, &val );
    //pTable->getMinValue( from, to, &val );
    //pTable->getMaxValue( from, to, &val );
    //pTable->getAverageValue( from, to, &val );
    //pTable->getMedianValue( from, to, &val );
    //pTable->getStdevValue( from, to, &val );
    //pTable->getVarianceValue( from, to, &val );
    //pTable->getModeValue( from, to, &val );
    pTable->getUppeQuartileValue( from, to, &val );
#endif

#if 0 // TEST Get range of data
    xxPrintf( ">>>>>" + vscpdatetime::Now().getISODateTime() + "" );

    sqlite3_stmt *ppStmt;
    vscpdatetime from,to;
    from.set"0000-01-01 00:00:00";
    to.set"9999-06-01 00:00:00";

    if ( pTable->prepareRangeOfData( from, to, &ppStmt, true ) ) {

        std::string rowData;
        while ( pTable->getRowRangeOfData( ppStmt, rowData ) ) {
            syslog(  LOG_ERR, "[DM] " + rowData  );
        }

        pTable->finalizeRangeOfData( ppStmt );
    }

    xxPrintf( ">>>>>" + vscpdatetime::Now().getISODateTime() + "" );
#endif

    if (0 == type) {
        if (tokens.empty()) {
            syslog(LOG_ERR,
                   "[DM] [Action] Write Table: Action parameter is "
                   "not correct. Datetime is missing. Parameter= %s",
                   m_strParam.c_str());
            return NULL;
        }

        // Get date/time
        dt.set(tokens.front());
        tokens.pop_front();

        if (tokens.empty()) {
            syslog(LOG_ERR,
                   "[DM] [Action] Write Table: Action parameter is not "
                   "correct. value is missing. Parameter=%s ",
                   m_strParam.c_str());
            return NULL;
        }

        // Get value
        value = stod(tokens.front());
        tokens.pop_front();

        // format: dattime;value
        if (tokens.empty()) {
            std::string sql = pTable->getSQLInsert();

            // Escapes
            dmElement::handleEscapes(m_pFeedEvent, sql);

            // Log the data
            // TODO dt.SetMillisecond( m_pFeedEvent->timestamp /
            // 1000 );
            if (!pTable->logData(dt, value, sql)) {
                syslog(LOG_ERR,
                       "[DM] [Action] Write Table: "
                       "Failed to log data (datetime,value)");
            }
        }
        // format: datetime;value;sql
        else {
            // Get custom sql expression
            sql = tokens.front();
            tokens.pop_front();

            std::string strResult;
            if (!vscp_std_decodeBase64IfNeeded(sql, strResult)) {
                syslog(LOG_ERR,
                       "[DM] [Action] Write Table: "
                       "Failed to decode SQL string. Will continue anyway.");
                strResult = sql;
            }

            // Escapes
            dmElement::handleEscapes(m_pFeedEvent, strResult);

            // Log the data
            dt.SetMillisecond(m_pFeedEvent->timestamp / 1000);
            if (!pTable->logData(dt, value, strResult)) {
                syslog(LOG_ERR,
                       "[DM] [Action] Write Table: Failed "
                       "to log data (datetime,value,sql)");
            }
        }
    }
    // Format: [BASE64:]sql-expression
    else if (1 == type) {

        if (tokens.empty()) {
            syslog(LOG_ERR,
                   "[DM] [Action] Write Table: Action parameter is not "
                   "correct. SQL expression is missing. Parameter=%s ",
                   m_strParam.c_str());
            return NULL;
        }

        // Get custom sql expression
        sql = tokens.front();
        tokens.pop_front();

        std::string strResult;
        if (!vscp_std_decodeBase64IfNeeded(sql, strResult)) {
            syslog(LOG_ERR,
                   "[DM] [Action] Write Table: Failed to "
                   "decode sql string. Will continue "
                   "anyway. sql=%s",
                   sql.c_str());
            strResult = sql;
        }

        // Log the data
        if (!pTable->logData(strResult)) {
            syslog(LOG_ERR,
                   "[DM] [Action] Write Table: "
                   "Failed to log data (sql=%s)",
                   sql.c_str());
        }
    } else {
        syslog(LOG_ERR,
               "[DM] [Action] Write Table: Invalid type. "
               "Parameter=%s ",
               m_strParam.c_str());
    }

    return NULL;
}

#endif
