// vscpdb.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2016
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//


// VSCP database ordinals etc is defined in this file

#if !defined(VSCPDB__INCLUDED_)
#define VSCPDB__INCLUDED_

//*****************************************************************************
//                               SETTINGS
//*****************************************************************************

#define VSCPDB_SETTINGS_CREATE "CREATE TABLE \"Settings\" ("\
	"`vscpd_idx_settings`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`vscpd_dbversion`                  INTEGER NOT NULL DEFAULT 1,"\
	"`vscpd_LogLevel`                   INTEGER DEFAULT 1,"\
        "'vscpd.maxqueue'                   INTEGER DEFAULT 8191,"\
	"`vscpd_RunAsUser`                  TEXT DEFAULT '',"\
	"`vscpd_GUID`                       TEXT '00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',"\
	"`vscpd_Name`                       TEXT,"\
	"`vscpd_Syslog_Enable`              INTEGER DEFAULT 1,"\
        "`vscpd_LogDB_Enable`               INTEGER DEFAULT 1,"\
        "`vscpd_db_log_path`                TEXT"\
	"`vscpd_GeneralLogFile_Enable`      INTEGER DEFAULT 1,"\
	"`vscpd_GeneralLogFile_Path`        TEXT,"\
	"`vscpd_SecurityLogFile_Enable`     INTEGER DEFAULT 1,"\
	"`vscpd_SecurityLogFile_Path`       TEXT,"\
	"`vscpd_AccessLogFile_Enable`       INTEGER DEFAULT 1,"\
	"`vscpd_AccessLogFile_Path`         TEXT,"\
	"`vscpd_TcpipInterface_Address`     TEXT,"\
	"`vscpd_MulticastInterface_Port`    TEXT,"\
	"`vscpd_MulicastInterface_ttl`      INTEGER,"\
	"`vscpd_UdpSimpleInterface_Enable`  INTEGER,"\
	"`vscpd_UdpSimpleInterface_Address` TEXT,"\
	"`vscpd_DM_DB_Path`                 TEXT,"\
        "`vscpd_DM_XML_Path`                TEXT,"\
	"`vscpd_DM_Logging_Enable`          INTEGER,"\
	"`vscpd_DM_Logging_Path`            TEXT,"\
	"`vscpd_DM_Logging_Level`           INTEGER,"\
	"`vscpd_Variables_DB_Path`          TEXT,"\
        "`vscpd_Variables_XML_Path`         TEXT,"\
	"`vscpd_VSCPD_DefaultClientBufferSize`      INTEGER,"\
	"`vscpd_Webserver_Authentication_enable`    INTEGER,"\
	"`vscpd_Webserver_RootPath`         TEXT,"\
	"`vscpd_Webserver_Address`          TEXT,"\
	"`vscpd_Webserver_PathCert`         TEXT,"\
	"`vscpd_Webserver_AuthDomain`       TEXT DEFAULT 'mydomain.com',"\
	"`vscpd_Webserver_CgiInterpreter`   TEXT,"\
	"`vscpd_Webserver_CgiPattern`       TEXT,"\
	"`vscpd_Webserver_EnableDirectoryListings`  INTEGER,"\
	"`vscpd_Webserver_HideFilePatterns` TEXT,"\
	"`vscpd_Webserver_IndexFiles`       TEXT,"\
	"`vscpd_Webserver_ExtraMimeTypes`   TEXT,"\
	"`vscpd_Webserver_UrlRewrites`      TEXT,"\
	"`vscpd_Webserver_SSIPattern`       NUMERIC,"\
	"`vscpd_Webserver_RunAsUser`        TEXT,"\
	"`vscpd_Webserver_PerDirectoryAuthFile`     TEXT,"\
	"`vscpd_Webserver_GlobalAuthFile`   TEXT,"\
	"`vscpd_Webserver_IpAcl`           TEXT,"\
	"`vscpd_Webserver_DavDocumentRoot`  TEXT,"\
	"`vscpd_WebSocket_EnableAuth`       INTEGER,"\
	"`vscpd_MqttBroker_Enable`          INTEGER,"\
	"`vscpd_MqttBroker_Address`            TEXT,"\
	"`vscpd_CoapServer_Enable`          INTEGER,"\
	"`vscpd_CoapServer_Address`            TEXT,"\
	"`vscpd_Automation_Enable`          INTEGER,"\
	"`vscpd_Automation_Zone`            INTEGER,"\
	"`vscpd_Automation_SubZone`         INTEGER,"\
	"`vscpd_Automation_Longitude`       REAL,"\
	"`vscpd_Automation_Latitude`                        REAL,"\
	"`vscpd_Automation_Timezone`                        TEXT,"\
	"`vscpd_Automation_Sunrise_Enable`          INTEGER,"\
	"`vscpd_Automation_Sunset_Enable`           TEXT,"\
	"`vscpd_Automation_SunsetTwilight_Enable`   INTEGER,"\
	"`vscpd_Automation_SunriseTwilight_Enable`  INTEGER,"\
	"`vscpd_Automation_SegmentControllerEvent_Enable`   INTEGER,"\
	"`vscpd_Automation_SegmentControllerEvent_Interval` INTEGER,"\
	"`vscpd_Automation_HeartbeatEvent_Enable`   INTEGER,"\
	"`vscpd_Automation_HeartbeatEvent_Interval` INTEGER,"\
	"`vscpd_db_data_path`                       TEXT,"\
        "`vscpd_db_vscpconf_path`                   TEXT,"\
        ");";

#define VSCPDB_CONFIG_SET_DEFAULTS  ""        
        

#define VSCPDB_CONFIG_UPDATE_ITEM "UPDATE 'Settings' SET ( %s='%s' ) WHERE id='%d' "


#define VSCPDB_ORDINAL_CONFIG_ID                                            0
#define VSCPDB_ORDINAL_CONFIG_DBVERSION                                     1
#define VSCPDB_ORDINAL_CONFIG_LOGLEVEL                                      2
#define VSCPDB_ORDINAL_CONFIG_MAXQUEUE                                      3
#define VSCPDB_ORDINAL_CONFIG_RUNASUSER                                     4
#define VSCPDB_ORDINAL_CONFIG_GUID                                          5
#define VSCPDB_ORDINAL_CONFIG_NAME                                          6
#define VSCPDB_ORDINAL_CONFIG_SYSLOG_ENABLE                                 7
#define VSCPDB_ORDINAL_CONFIG_LOGDB_ENABLE                                  8
#define VSCPDB_ORDINAL_CONFIG_DB_LOG_PATH                                   9
#define VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE                         10
#define VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_PATH                           11
#define VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE                        12
#define VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH                          13
#define VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE                          14
#define VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH                            15
#define VSCPDB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT                           16
#define VSCPDB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT                       17
#define VSCPDB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL                         18
#define VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE                     19
#define VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT                       20
#define VSCPDB_ORDINAL_CONFIG_DM_DB_PATH                                    21
#define VSCPDB_ORDINAL_CONFIG_DM_XML_PATH                                   22
#define VSCPDB_ORDINAL_CONFIG_DM_LOGGING_ENABLE                             23
#define VSCPDB_ORDINAL_CONFIG_DM_LOGGING_PATH                               24
#define VSCPDB_ORDINAL_CONFIG_DM_LOGGING_LEVEL                              25
#define VSCPDB_ORDINAL_CONFIG_VARIABLES_DB_PATH                             26
#define VSCPDB_ORDINAL_CONFIG_VARIABLES_XML_PATH                            27
#define VSCPDB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE                 28
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION               29
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH                            30
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_PORT                                31
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_PATHCERT                            32
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN                          33
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER                      34
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN                          35
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS             36
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS                    37
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES                          38
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES                      39
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES                         40
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN                          41
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER                           42
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE                43
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE                      44
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER__IPACL                              45
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT                     46
#define VSCPDB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH                          47
#define VSCPDB_ORDINAL_CONFIG_MQTTBROKER_ENABLE                             48
#define VSCPDB_ORDINAL_CONFIG_MQTTBROKER_PORT                               49
#define VSCPDB_ORDINAL_CONFIG_COAPSERVER_ENABLE                             50
#define VSCPDB_ORDINAL_CONFIG_COAPSERVER_PORT                               51
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_ZONE                               52
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUBZONE                            53
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE                          54
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_LATITUDE                           55
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_TIMEZONE                           56
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE                     57
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE                      58
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE              59
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE             60
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE      61
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL    62
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE              63
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL            64
#define VSCPDB_ORDINAL_CONFIG_DB_VSCPDATA_PATH                              65
#define VSCPDB_ORDINAL_CONFIG_DB_VSCPCONF_PATH                              66

#define VSCPDB_LOG_CREATE "CREATE TABLE \"log\" ("\
	"`idx_log`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`type`	INTEGER,"\
	"`date`	TEXT,"\
	"`level`	INTEGER,"\
	"`message`	TEXT"\
        ");";\

#define VSCPDB_ORDINAL_LOG_ID                       0   // 
#define VSCPDB_ORDINAL_LOG_TYPE                     1   // 
#define VSCPDB_ORDINAL_LOG_DATE                     2   // 
#define VSCPDB_ORDINAL_LOG_LEVEL                    3   // 
#define VSCPDB_ORDINAL_LOG_MESSAGE                  4   // 

//*****************************************************************************
//                                 USER
//*****************************************************************************

#define VSCPDB_USER_CREATE "CREATE TABLE 'user' ("\
	"`idx_user`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`username`	TEXT NOT NULL UNIQUE,"\
	"`password`	TEXT NOT NULL,"\
	"`fullname`	TEXT NOT NULL,"\
	"`permission`	INTEGER NOT NULL,"\
	"`note`	TEXT NOT NULL"\
        ");"

#define VSCPDB_ORDINAL_USER_ID                       0   // 
#define VSCPDB_ORDINAL_USER_USERNAME                 1   // 
#define VSCPDB_ORDINAL_USER_PASSWORD                 2   // 
#define VSCPDB_ORDINAL_USER_FULLNAME                 3   // 
#define VSCPDB_ORDINAL_USER_PERMISSION               4   // 
#define VSCPDB_ORDINAL_USER_NOTE                     5   // 

//*****************************************************************************
//                                GROUP
//*****************************************************************************

#define VSCPDB_GROUP_CREATE "CREATE TABLE 'group' ("\
	"`idx_group`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`groupname`	TEXT NOT NULL,"\
	"`permission`	INTEGER NOT NULL,"\
	"`note`	TEXT"\
        ");"

#define VSCPDB_ORDINAL_GROUP_ID                     0   // 
#define VSCPDB_ORDINAL_GROUP_USERNAME               1   // 
#define VSCPDB_ORDINAL_GROUP_PERMISSION             2   // 
#define VSCPDB_ORDINAL_GROUP_NOTE                   3   // 

//*****************************************************************************
//                          external VARIABLE
//*****************************************************************************

#define VSCPDB_VARIABLE_CREATE  "CREATE TABLE 'variable' ("\
"'idx_variableex'  INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
"'lastchange'      TEXT NOT NULL,"\
"'name'            TEXT NOT NULL UNIQUE,"\
"'type'            INTEGER NOT NULL DEFAULT 0,"\
"'value'           TEXT NOT NULL,"\
"'bPersistent'     INTEGER NOT NULL DEFAULT 0,"\
"'link_to_user'    INTEGER NOT NULL,"\
"'link_to_group'   INTEGER NOT NULL,"\
"'permission'      INTEGER NOT NULL DEFAULT 777,"\
"'note'	TEXT"\
");"

#define VSCPDB_ORDINAL_VARIABLE_ID              0   //
#define VSCPDB_ORDINAL_VARIABLE_LASTCHANGE      1   //
#define VSCPDB_ORDINAL_VARIABLE_NAME            2   // 
#define VSCPDB_ORDINAL_VARIABLE_TYPE            3   // 
#define VSCPDB_ORDINAL_VARIABLE_VALUE           4   // 
#define VSCPDB_ORDINAL_VARIABLE_PERSISTENT      5   // 
#define VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER    6   // 
#define VSCPDB_ORDINAL_VARIABLE_LINK_TO_GROUP   7   // 
#define VSCPDB_ORDINAL_VARIABLE_PERMISSION      8   // 
#define VSCPDB_ORDINAL_VARIABLE_NOTE            9   // 



//*****************************************************************************
//                                 DM
//*****************************************************************************



#define VSCPDB_DM_CREATE  "CREATE TABLE 'dm' ("\
	"`id`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`GroupID`	TEXT NOT NULL,"\
	"`bEnable`	INTEGER NOT NULL DEFAULT 0,"\
	"`maskPriority`	INTEGER NOT NULL DEFAULT 0,"\
	"`maskClass`	NUMERIC NOT NULL DEFAULT 0,"\
	"`maskType`	INTEGER NOT NULL DEFAULT 0,"\
	"`maskGUID`	TEXT NOT NULL DEFAULT DEFAULT '00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',"\
	"`filterPriority`	INTEGER NOT NULL DEFAULT 0,"\
	"`filterClass`	INTEGER NOT NULL DEFAULT 0,"\
	"`filterType`	INTEGER NOT NULL DEFAULT 0,"\
	"`filterGUID`	TEXT NOT NULL,"\
	"`allowedFrom`	TEXT NOT NULL,"\
	"`allowedTo`	TEXT NOT NULL,"\
	"`allowedMonday`	INTEGER NOT NULL,"\
	"`allowedTuesday`	INTEGER NOT NULL,"\
	"`allowsWednesday`	INTEGER NOT NULL,"\
	"`allowedThursday`	INTEGER NOT NULL,"\
	"`allowedFriday`	INTEGER NOT NULL,"\
	"`allowedSaturday`	NUMERIC NOT NULL,"\
	"`allowedSunday`	INTEGER NOT NULL,"\
	"`allowedTime`          TEXT NOT NULL,"\
	"`bCheckIndex`          INTEGER NOT NULL,"\
	"`index`                TEXT NOT NULL,"\
	"`bCheckZone`           TEXT NOT NULL,"\
	"`zone`                 INTEGER NOT NULL,"\
	"`bCheckSubZone`	INTEGER NOT NULL,"\
	"`subzone`              INTEGER NOT NULL,"\
	"`bCheckMeasurementIndex`	INTEGER NOT NULL,"\
	"`meaurementIndex`	INTEGER NOT NULL,"\
	"`actionCode`           TEXT NOT NULL,"\
	"`actionParameter`	NUMERIC NOT NULL,"\
	"`measurementValue`	REAL,"\
	"`measurementUnit`	INTEGER,"\
	"`measurementCompare`	INTEGER,"\
	"`comment`              TEXT"\
    ")"

// The internal table does not have an autoincrement id as it is filled with 
// the id from the external database.
#define VSCPDB_DM_INTERNAL_CREATE  "CREATE TABLE 'dm' ("\
	"`id`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`GroupID`	TEXT NOT NULL,"\
	"`bEnable`	INTEGER NOT NULL DEFAULT 0,"\
	"`maskPriority`	INTEGER NOT NULL DEFAULT 0,"\
	"`maskClass`	NUMERIC NOT NULL DEFAULT 0,"\
	"`maskType`	INTEGER NOT NULL DEFAULT 0,"\
	"`maskGUID`	TEXT NOT NULL,"\
	"`filterPriority`	INTEGER NOT NULL DEFAULT 0,"\
	"`filterClass`	INTEGER NOT NULL DEFAULT 0,"\
	"`filterType`	INTEGER NOT NULL DEFAULT 0,"\
	"`filterGUID`	TEXT NOT NULL,"\
	"`allowedStart`	TEXT,"\
	"`allowedEnd`	TEXT,"\
	"`allowedMonday`	INTEGER NOT NULL,"\
	"`allowedTuesday`	INTEGER NOT NULL,"\
	"`allowsWednesday`	INTEGER NOT NULL,"\
	"`allowedThursday`	INTEGER NOT NULL,"\
	"`allowedFriday`	INTEGER NOT NULL,"\
	"`allowedSaturday`	NUMERIC NOT NULL,"\
	"`allowedSunday`	INTEGER NOT NULL,"\
	"`allowedTime`	TEXT,"\
	"`bCheckIndex`	INTEGER NOT NULL,"\
	"`index`	TEXT NOT NULL,"\
	"`bCheckZone`	TEXT NOT NULL,"\
	"`zone`	INTEGER NOT NULL,"\
	"`bCheckSubZone`	INTEGER NOT NULL,"\
	"`subzone`	INTEGER NOT NULL,"\
	"`bCheckMeasurementIndex`	INTEGER NOT NULL,"\
	"`meaurementIndex`	INTEGER NOT NULL,"\
	"`actionCode`	TEXT NOT NULL,"\
	"`actionParameter`	NUMERIC NOT NULL,"\
	"`measurementValue`	REAL,"\
	"`measurementUnit`	INTEGER,"\
	"`measurementCompare`	INTEGER,"\
	"`comment`	TEXT"\
    ")"

#define VSCPDB_DM_INSERT "INSERT INTO 'dm' "\
                "(GroupID,bEnable,maskPriority,maskClass,maskType,maskGUID,filterPriority,filterClass,filterType,filterGUID,"\
                "allowedStart,allowedEnd,allowedMonday,allowedTuesday,allowsWednesday,allowedThursday,allowedFriday,allowedSaturday,"\
                "allowedSunday,allowedTime,bCheckIndex,index,bCheckZone,zone,bCheckSubZone,subzone,bCheckMeasurementIndex,"\
                "meaurementIndex,actionCode,actionParameter,measurementValue,measurementUnit,measurementCompare,comment"\
                " )VALUES ('%s','%d','%d','%d','%d','%s','%d','%d','%d','%s',"\
                "'%s','%s','%d','%d','%d','%d','%d','%d','%d',"\
                "'%d','%s','%d','%d','%d','%d','%d','%d','%d','%d','%d','%s','%f','%d','%s'"\
                ")"
                        
#define VSCPDB_DM_UPDATE "UPDATE 'dm' "\
                "SET (GroupID='%s',bEnable='%d',maskPriority='%d',maskClass=%d',maskType=%d',maskGUID=%s',filterPriority=%d',filterClass=%d',filterType=%d',filterGUID=%s',"\
                "allowedStart=%s',allowedEnd=%s',allowedMonday='%d',allowedTuesday='%d',allowsWednesday='%d',allowedThursday='%d',allowedFriday='%d',allowedSaturday='%d',"\
                "allowedSunday,allowedTime,bCheckIndex,index,bCheckZone,zone,bCheckSubZone,subzone,bCheckMeasurementIndex,"\
                "meaurementIndex='%d',actionCode='%d',actionParameter='%s',measurementValue='%f',measurementUnit='%d',measurementCompare='%d'"\
                ") WHERE id='%d'" 

#define VSCPDB_DM_UPDATE_ITEM "UPDATE 'dm' SET ( %s='%s' ) WHERE id='%d' "

#define VSCPDB_ORDINAL_DM_ID                        0   // 
#define VSCPDB_ORDINAL_DM_ENABLE                    1   //
#define VSCPDB_ORDINAL_DM_GROUPID                   2   //
#define VSCPDB_ORDINAL_DM_MASK_PRIORITY             3   //
#define VSCPDB_ORDINAL_DM_MASK_CLASS                4   //
#define VSCPDB_ORDINAL_DM_MASK_TYPE                 5   //
#define VSCPDB_ORDINAL_DM_MASK_GUID                 6   //
#define VSCPDB_ORDINAL_DM_FILTER_PRIORITY           7   //
#define VSCPDB_ORDINAL_DM_FILTER_CLASS              8   //
#define VSCPDB_ORDINAL_DM_FILTER_TYPE               9   //
#define VSCPDB_ORDINAL_DM_FILTER_GUID               10   //
#define VSCPDB_ORDINAL_DM_ALLOWED_START             11   //
#define VSCPDB_ORDINAL_DM_ALLOWED_END               12   //
#define VSCPDB_ORDINAL_DM_ALLOWED_MONDAY            13   //
#define VSCPDB_ORDINAL_DM_ALLOWED_TUESDAY           14   //
#define VSCPDB_ORDINAL_DM_ALLOWED_WEDNESDAY         15   //
#define VSCPDB_ORDINAL_DM_ALLOWED_THURSDAY          16   //
#define VSCPDB_ORDINAL_DM_ALLOWED_FRIDAY            17   //
#define VSCPDB_ORDINAL_DM_ALLOWED_SATURDAY          18   //
#define VSCPDB_ORDINAL_DM_ALLOWED_SUNDAY            19   //
#define VSCPDB_ORDINAL_DM_ALLOWED_TIME              20   //
#define VSCPDB_ORDINAL_DM_CHECK_INDEX               21   //
#define VSCPDB_ORDINAL_DM_INDEX                     22   //
#define VSCPDB_ORDINAL_DM_CHECK_ZONE                23   //
#define VSCPDB_ORDINAL_DM_ZONE                      24   //
#define VSCPDB_ORDINAL_DM_CHECK_SUBZONE             25   //
#define VSCPDB_ORDINAL_DM_SUBZONE                   26   //
#define VSCPDB_ORDINAL_DM_CHECK_MEASUREMENT_INDEX   27   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_INDEX         28   //
#define VSCPDB_ORDINAL_DM_ACTIONCODE                29   //
#define VSCPDB_ORDINAL_DM_ACTIONPARAMETER           30   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE         31   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT          32   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE       33   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_COMMENT       34   //


/*
 // Configuration database ordinals
enum {
    DAEMON_DB_ORDINAL_CONFIG_ID = 0,
    DAEMON_DB_ORDINAL_CONFIG_DBVERSION,
    DAEMON_DB_ORDINAL_CONFIG_LOGLEVEL,
    DAEMON_DB_ORDINAL_CONFIG_RUNASUSER,
    DAEMON_DB_ORDINAL_CONFIG_GUID,
    DAEMON_DB_ORDINAL_CONFIG_NAME,
    DAEMON_DB_ORDINAL_CONFIG_SYSLOG_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_GENERALLOGFILE_PATH,
    DAEMON_DB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH,
    DAEMON_DB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH,
    DAEMON_DB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT,
    DAEMON_DB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT,
    DAEMON_DB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL,
    DAEMON_DB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT,
    DAEMON_DB_ORDINAL_CONFIG_DM_PATH,
    DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_PATH,
    DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_LEVEL,
    DAEMON_DB_ORDINAL_CONFIG_VARIABLES_PATH,
    DAEMON_DB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PORT,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PATHCERT,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER__IPACL,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT,
    DAEMON_DB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH,
    DAEMON_DB_ORDINAL_CONFIG_MQTTBROKER_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_MQTTBROKER_PORT,
    DAEMON_DB_ORDINAL_CONFIG_COAPSERVER_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_COAPSERVER_PORT,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_ZONE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUBZONE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_LATITUDE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_TIMEZONE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL,
    DAEMON_DB_ORDINAL_CONFIG_DB_DATA_PATH,
    DAEMON_DB_ORDINAL_CONFIG_DB_LOG_PATH,
};
 */

#endif  // compile

        