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
	"`vscpd_idx_settings`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
	"`vscpd_dbversion`	INTEGER NOT NULL,"\
	"`vscpd_LogLevel`	INTEGER,"\
	"`vscpd_RunAsUser`	TEXT,"\
	"`vscpd_GUID`	TEXT,"\
	"`vscpd_Name`	TEXT,"\
	"`vscpd_Syslog_Enable`	INTEGER,"\
	"`vscpd_GeneralLogFile_Enable`	INTEGER,"\
	"`vscpd_GeneralLogFile_Path`	TEXT,"\
	"`vscpd_SecurityLogFile_Enable`	INTEGER,"\
	"`vscpd_SecurityLogFile_Path`	TEXT,"\
	"`vscpd_AccessLogFile_Enable`	INTEGER,"\
	"`vscpd_AccessLogFile_Path`	TEXT,"\
	"`vscpd_TcpipInterface_Port`	REAL,"\
	"`vscpd_MulticastInterface_Port`	TEXT,"\
	"`vscpd_MulicastInterface_ttl`	INTEGER,"\
	"`vscpd_UdpSimpleInterface_Enable`	INTEGER,"\
	"`vscpd_UdpSimpleInterface_Port`	TEXT,"\
	"`vscpd_DM_Path`	NUMERIC,"\
	"`vscpd_DM_Logging_Enable`	INTEGER,"\
	"`vscpd_DM_Logging_Path`	TEXT,"\
	"`vscpd_DM_Logging_Level`	INTEGER,"\
	"`vscpd_Variables_Path`	TEXT,"\
	"`vscpd_VSCPD_DefaultClientBufferSize`	INTEGER,"\
	"`vscpd_Webserver_DisableAuthentication`	INTEGER,"\
	"`vscpd_Webserver_RootPath`	TEXT,"\
	"`vscpd_Webserver_Port`	TEXT,"\
	"`vscpd_Webserver_PathCert`	TEXT,"\
	"`vscpd_Webserver_AuthDomain`	TEXT,"\
	"`vscpd_Webserver_CgiInterpreter`	TEXT,"\
	"`vscpd_Webserver_CgiPattern`	TEXT,"\
	"`vscpd_Webserver_EnableDirectoryListings`	INTEGER,"\
	"`vscpd_Webserver_HideFilePatterns`	TEXT,"\
	"`vscpd_Webserver_IndexFiles`	TEXT,"\
	"`vscpd_Webserver_ExtraMimeTypes`	TEXT,"\
	"`vscpd_Webserver_UrlRewrites`	TEXT,"\
	"`vscpd_Webserver_SSIPattern`	NUMERIC,"\
	"`vscpd_Webserver_RunAsUser`	REAL,"\
	"`vscpd_Webserver_PerDirectoryAuthFile`	TEXT,"\
	"`vscpd_Webserver_GlobalAuthFile`	TEXT,"\
	"`vscpd_Webserver__IpAcl`	TEXT,"\
	"`vscpd_Webserver_DavDocumentRoot`	TEXT,"\
	"`vscpd_WebSocket_EnableAuth`	INTEGER,"\
	"`vscpd_MqttBroker_Enable`	INTEGER,"\
	"`vscpd_MqttBroker_Port`	TEXT,"\
	"`vscpd_CoapServer_Enable`	INTEGER,"\
	"`vscpd_CoapServer_Port`	TEXT,"\
	"`vscpd_Automation_Enable`	INTEGER,"\
	"`vscpd_Automation_Zone`	INTEGER,"\
	"`vscpd_Automation_SubZone`	INTEGER,"\
	"`vscpd_Automation_Longitude`	REAL,"\
	"`vscpd_Automation_Latitude`	REAL,"\
	"`vscpd_Automation_Timezone`	TEXT,"\
	"`vscpd_Automation_Sunrise_Enable`	INTEGER,"\
	"`vscpd_Automation_Sunset_Enable`	TEXT,"\
	"`vscpd_Automation_SunsetTwilight_Enable`	INTEGER,"\
	"`vscpd_Automation_SunriseTwilight_Enable`	INTEGER,"\
	"`vscpd_Automation_SegmentControllerEvent_Enable`	INTEGER,"\
	"`vscpd_Automation_SegmentControllerEvent_Interval`	INTEGER,"\
	"`vscpd_Automation_HeartbeatEvent_Enable`	INTEGER,"\
	"`vscpd_Automation_HeartbeatEvent_Interval`	INTEGER,"\
	"`vscpd_db_data_path`	TEXT,"\
	"`vscpd_db_log_path`	TEXT"\
        ");";

#define VSCPDB_ORDINAL_CONFIG_ID                                            0
#define VSCPDB_ORDINAL_CONFIG_DBVERSION                                     1
#define VSCPDB_ORDINAL_CONFIG_LOGLEVEL                                      2
#define VSCPDB_ORDINAL_CONFIG_RUNASUSER                                     3
#define VSCPDB_ORDINAL_CONFIG_GUID                                          4
#define VSCPDB_ORDINAL_CONFIG_NAME                                          5
#define VSCPDB_ORDINAL_CONFIG_SYSLOG_ENABLE                                 6
#define VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE                         7
#define VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_PATH                           8
#define VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE                        9
#define VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH                          10
#define VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE                          11
#define VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH                            12
#define VSCPDB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT                           13
#define VSCPDB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT                       14
#define VSCPDB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL                         15
#define VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE                     16
#define VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT                       17
#define VSCPDB_ORDINAL_CONFIG_DM_PATH                                       18
#define VSCPDB_ORDINAL_CONFIG_DM_LOGGING_ENABLE                             19
#define VSCPDB_ORDINAL_CONFIG_DM_LOGGING_PATH                               20
#define VSCPDB_ORDINAL_CONFIG_DM_LOGGING_LEVEL                              21
#define VSCPDB_ORDINAL_CONFIG_VARIABLES_PATH                                22
#define VSCPDB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE                 23
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION               24
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH                            25
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_PORT                                26
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_PATHCERT                            27
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN                          28
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER                      29
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN                          30
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS             31
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS                    32
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES                          33
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES                      34
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES                         35
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN                          36
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER                           37
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE                38
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE                      39
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER__IPACL                              40
#define VSCPDB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT                     41
#define VSCPDB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH                          42
#define VSCPDB_ORDINAL_CONFIG_MQTTBROKER_ENABLE                             43
#define VSCPDB_ORDINAL_CONFIG_MQTTBROKER_PORT                               44
#define VSCPDB_ORDINAL_CONFIG_COAPSERVER_ENABLE                             45
#define VSCPDB_ORDINAL_CONFIG_COAPSERVER_PORT                               46
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_ZONE                               47
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUBZONE                            48
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE                          49
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_LATITUDE                           50
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_TIMEZONE                           51
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE                     52
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE                      53
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE              54
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE             55
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE      56
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL    57
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE              58
#define VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL            59
#define VSCPDB_ORDINAL_CONFIG_DB_DATA_PATH                                  60
#define VSCPDB_ORDINAL_CONFIG_DB_LOG_PATH                                   61


#define VSCPDB_LOG_CREATE "CREATE TABLE \"log\" ("\
	"`idx_log`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
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

#define VSCPDB_USER_CREATE "CREATE TABLE \"user\" ("\
	"`idx_user`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
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
//                                 GROUP
//*****************************************************************************

#define VSCPDB_GROUP_CREATE "CREATE TABLE \"group\" ("\
	"`idx_group`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
	"`groupname`	TEXT NOT NULL,"\
	"`permission`	INTEGER NOT NULL,"\
	"`note`	TEXT"\
        ");"

#define VSCPDB_ORDINAL_GROUP_ID                     0   // 
#define VSCPDB_ORDINAL_GROUP_USERNAME               1   // 
#define VSCPDB_ORDINAL_GROUP_PERMISSION             2   // 
#define VSCPDB_ORDINAL_GROUP_NOTE                   3   // 

//*****************************************************************************
//                           external VARIABLE
//*****************************************************************************

#define VSCPDB_VARIABLE_EXT_CREATE  "CREATE TABLE 'variableEx' ("\
"'idx_variableex'  INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
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

#define VSCPDB_ORDINAL_VARIABLE_EXT_ID              0   //
#define VSCPDB_ORDINAL_VARIABLE_EXT_LASTCHANGE      1   //
#define VSCPDB_ORDINAL_VARIABLE_EXT_NAME            2   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_TYPE            3   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_VALUE           4   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_PERSISTENT      5   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_LINK_TO_USER    6   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_LINK_TO_GROUP   7   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_PERMISSION      8   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_NOTE            9   // 

//*****************************************************************************
//                             internal VARIABLE
//*****************************************************************************
        
#define VSCPDB_VARIABLE_INT_CREATE "CREATE TABLE 'variableInt' ("\
	"`idx_variableint`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
        "`lastchange`	TEXT NOT NULL,"\
	"`name`	TEXT NOT NULL UNIQUE,"\
	"`type`	INTEGER NOT NULL DEFAULT 1,"\
	"`value`	TEXT NOT NULL,"\
        "'bPersistent'     INTEGER NOT NULL DEFAULT 0,"\
	"`link_to_user`	INTEGER NOT NULL DEFAULT 0,"\
	"`link_to_group`	INTEGER NOT NULL DEFAULT 0,"\
	"`permission`	INTEGER NOT NULL DEFAULT 777,"\
        "`note`	TEXT"\
        ");"      

#define VSCPDB_ORDINAL_VARIABLE_INT_ID              0   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_LASTCHANGE      1   //
#define VSCPDB_ORDINAL_VARIABLE_INT_NAME            2   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_TYPE            3   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_VALUE           4   // 
#define VSCPDB_ORDINAL_VARIABLE_EXT_PERSISTENT      5   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_USER    6   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_LINK_TO_GROUP   7   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_PERMISSION      8   // 
#define VSCPDB_ORDINAL_VARIABLE_INT_NOTE            9   //



//*****************************************************************************
//                                 DM
//*****************************************************************************



#define VSCPDB_DM_CREATE  "CREATE TABLE 'dm' ("\
	"`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,"\
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
	"`allowedFrom`	TEXT NOT NULL,"\
	"`allowedTo`	TEXT NOT NULL,"\
	"`allowedMonday`	BLOB NOT NULL,"\
	"`allowedTuesday`	INTEGER NOT NULL,"\
	"`allowsWednesday`	INTEGER NOT NULL,"\
	"`allowedThursday`	INTEGER NOT NULL,"\
	"`allowedFriday`	INTEGER NOT NULL,"\
	"`allowedSaturday`	NUMERIC NOT NULL,"\
	"`allowedSunday`	BLOB NOT NULL,"\
	"`allowedTime`	TEXT NOT NULL,"\
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

#define VSCPDB_ORDINAL_DM_ID                        0   // 
#define VSCPDB_ORDINAL_DM_GROUPID                   1   //
#define VSCPDB_ORDINAL_DM_ENABLE                    2   //
#define VSCPDB_ORDINAL_DM_MASK_PRIORITY             3   //
#define VSCPDB_ORDINAL_DM_MASK_CLASS                4   //
#define VSCPDB_ORDINAL_DM_MASK_FILTER               5   //
#define VSCPDB_ORDINAL_DM_MASK_GUID                 6   //
#define VSCPDB_ORDINAL_DM_FILTER_PRIORITY           7   //
#define VSCPDB_ORDINAL_DM_FILTER_CLASS              8   //
#define VSCPDB_ORDINAL_DM_FILTER_FILTER             9   //
#define VSCPDB_ORDINAL_DM_FILTER_GUID               10   //
#define VSCPDB_ORDINAL_DM_ALLOWED_FROM              11   //
#define VSCPDB_ORDINAL_DM_ALLOWED_TO                12   //
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


#endif  // compile
