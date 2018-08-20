// vscpdb.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// MIT License 
//
// Copyright (C) 2000-2018
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, merge, 
// publish, distribute, sublicense, and/or sell copies of the Software, 
// and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
// THE SOFTWARE.
//

// VSCP database ordinals etc is defined in this file

#if !defined(VSCPDB__INCLUDED_)
#define VSCPDB__INCLUDED_

//*****************************************************************************
//                                 CONFIG
//*****************************************************************************

#define VSCPDB_CONFIG_CREATE "CREATE TABLE IF NOT EXISTS 'config' ("\
	"`vscpd_idx_config`               INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`vscpd_name`                     TEXT NOT NULL,"\
        "`vscpd_value`                    TEXT DEFAULT ''"\
        ");";

#define VSCPDB_CONFIG_CREATE_INDEX "CREATE INDEX `idxname` ON config ('vscpd_name');"

#define VSCPDB_CONFIG_FIND_ALL "SELECT * FROM config;"
#define VSCPDB_CONFIG_FIND_ALL_SORT_NAME "SELECT * FROM config ORDER BY vscpd_name;"
#define VSCPDB_CONFIG_FIND_ITEM "SELECT * FROM config WHERE vscpd_name='%q';"
#define VSCPDB_CONFIG_UPDATE_ITEM "UPDATE 'config' SET 'vscpd_value' ='%q' WHERE vscpd_name='%q';"
#define VSCPDB_CONFIG_INSERT "INSERT INTO 'config' (vscpd_name,vscpd_value) VALUES ('%q','%q');"

#define VSCPDB_ORDINAL_CONFIG_ID                        0
#define VSCPDB_ORDINAL_CONFIG_NAME                      1
#define VSCPDB_ORDINAL_CONFIG_VALUE                     2


// Configuration defaults

#define VSCPDB_CONFIG_NAME_DBVERSION                    "dbversion"
#define VSCPDB_CONFIG_DEFAULT_DBVERSION                 "1"

#define VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE             "client_buffer_size"
#define VSCPDB_CONFIG_DEFAULT_CLIENTBUFFERSIZE          "8192"

#define VSCPDB_CONFIG_NAME_GUID                         "guid"
#define VSCPDB_CONFIG_DEFAULT_GUID                      "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

#define VSCPDB_CONFIG_NAME_SERVERNAME                   "servername"
#define VSCPDB_CONFIG_DEFAULT_SERVERNAME                "THE-VSCP-DAEMON"

#define VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR                "announceinterface_address"
#define VSCPDB_CONFIG_DEFAULT_ANNOUNCE_ADDR             "udp://9598"

#define VSCPDB_CONFIG_NAME_ANNOUNCE_TTL                 "announceinterface_ttl"
#define VSCPDB_CONFIG_DEFAULT_ANNOUNCE_TTL              "1"


// TCP/IP

#define VSCPDB_CONFIG_NAME_TCPIP_ADDR                     "tcpipinterface_address"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_ADDR                  "9598"

#define VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION               "tcpip_encryption"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_ENCRYPTION            ""

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE          "tcpip_ssl_certificate"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CERTIFICATE       "/srv/vscp/certs/tcpip_server.pem"

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN     "tcpip_ssl_certificate_chain"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CERTIFICAT_CHAIN  ""

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER          "tcpip_ssl_verify_peer"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_VERIFY_PEER       "0"

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH              "tcpip_ssl_ca_path"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CA_PATH           ""

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE              "tcpip_ssl_ca_file"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CA_FILE           ""

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH         "tcpip_ssl_verify_depth"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_VERIFY_DEPTH      "9"

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS     "tcpip_ssl_default_verify_paths"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_DEFAULT_VERIFY_PATHS  "1"

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST         "tcpip_ssl_cipher_list"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CHIPHER_LIST      "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256"

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION     "tcpip_ssl_protocol_version"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_PROTOCOL_VERSION  "0"

#define VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST          "tcpip_ssl_short_trust"
#define VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_SHORT_TRUST       "0"


// UDP

#define VSCPDB_CONFIG_NAME_UDP_ENABLE                   "udp_enable"
#define VSCPDB_CONFIG_DEFAULT_UDP_ENABLE                "1"

#define VSCPDB_CONFIG_NAME_UDP_ADDR                     "udp_address"
#define VSCPDB_CONFIG_DEFAULT_UDP_ADDR                  "udp://33333"

#define VSCPDB_CONFIG_NAME_UDP_USER                     "udp_user"
#define VSCPDB_CONFIG_DEFAULT_UDP_USER                  ""

#define VSCPDB_CONFIG_NAME_UDP_PASSWORD                 "udp_password"
#define VSCPDB_CONFIG_DEFAULT_UDP_PASSWORD              ""

#define VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE          "udp_unsecure_enable"
#define VSCPDB_CONFIG_DEFAULT_UDP_UNSECURE_ENABLE       "1"

#define VSCPDB_CONFIG_NAME_UDP_FILTER                   "udp_filter"
#define VSCPDB_CONFIG_DEFAULT_UDP_FILTER                ""

#define VSCPDB_CONFIG_NAME_UDP_MASK                     "udp_mask"
#define VSCPDB_CONFIG_DEFAULT_UDP_MASK                  ""

#define VSCPDB_CONFIG_NAME_UDP_GUID                     "udp_guid"
#define VSCPDB_CONFIG_DEFAULT_UDP_GUID                  "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"

#define VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE               "udp_ack_enable"
#define VSCPDB_CONFIG_DEFAULT_UDP_ACK_ENABLE            "0"

#define VSCPDB_CONFIG_NAME_MULTICAST_ENABLE             "muticast_enable"
#define VSCPDB_CONFIG_DEFAULT_MULTICAST_ENABLE           "0"

// DM

#define VSCPDB_CONFIG_NAME_DM_PATH_DB                   "dm_path_db"
#define VSCPDB_CONFIG_DEFAULT_DM_PATH_DB                "/srv/vscp/dm.sqlite3"

#define VSCPDB_CONFIG_NAME_DM_PATH_XML                  "dm_path_xml"
#define VSCPDB_CONFIG_DEFAULT_DM_PATH_XML               "/srv/vscp/dm.xml"

#define VSCPDB_CONFIG_NAME_DM_ALLOW_XML_SAVE            "dm_allow_xml_save"
#define VSCPDB_CONFIG_DEFAULT_DM_ALLOW_XML_SAVE         "0"

#define VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB            "variable_path_db"
#define VSCPDB_CONFIG_DEFAULT_VARIABLES_PATH_DB         "/srv/vscp/variable.sqlite3"

#define VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML           "variable_path_xml"
#define VSCPDB_CONFIG_DEFAULT_VARIABLES_PATH_XML        "/srv/vscp/variable.xml"

#define VSCPDB_CONFIG_NAME_PATH_DB_DATA                 "path_db_data"
#define VSCPDB_CONFIG_DEFAULT_PATH_DB_DATA              "/srv/vscp/vscp_data.sqlite3"

// WEB server

#define VSCPDB_CONFIG_NAME_WEB_ENABLE                   "web_enable"
#define VSCPDB_CONFIG_DEFAULT_WEB_ENABLE                "1"

#define VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT            "web_document_root"
#define VSCPDB_CONFIG_DEFAULT_WEB_DOCUMENT_ROOT         "/srv/vscp/web"

#define VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS          "web_listening_ports"
#define VSCPDB_CONFIG_DEFAULT_WEB_LISTENING_PORTS       "[::]:8888r,[::]:8843s,8884"

#define VSCPDB_CONFIG_NAME_WEB_INDEX_FILES              "web_index_files"
#define VSCPDB_CONFIG_DEFAULT_WEB_INDEX_FILES           "index.xhtml,index.html,index.htm,index.lp,index.lsp,index.lua,index.cgi,index.shtml,index.php"

#define VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN    "web_authentication_domain"
#define VSCPDB_CONFIG_DEFAULT_WEB_AUTHENTICATION_DOMAIN "mydomain.com"

#define VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK     "web_enable_auth_domain_check"
#define VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_AUTH_DOMAIN_CHECK  "1"

#define VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE          "web_ssl_certificate"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICATE       "/srv/vscp/certs/server.pem"

#define VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN     "web_ssl_certificate_chain"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICAT_CHAIN  ""

#define VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER          "web_ssl_verify_peer"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_PEER       "0"

#define VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH              "web_ssl_ca_path"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_PATH           ""

#define VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE              "web_ssl_ca_file"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_FILE           ""

#define VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH         "web_ssl_verify_depth"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_DEPTH      "9"

#define VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS     "web_ssl_default_verify_paths"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_DEFAULT_VERIFY_PATHS  "1"

#define VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST         "web_ssl_cipher_list"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_CHIPHER_LIST      "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256"

#define VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION     "web_ssl_protocol_version"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_PROTOCOL_VERSION  "0"

#define VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST          "web_ssl_short_trust"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSL_SHORT_TRUST       "0"

#define VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER          "web_cgi_interpreter"
#define VSCPDB_CONFIG_DEFAULT_WEB_CGI_INTERPRETER       ""

#define VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN              "web_cgi_pattern"
#define VSCPDB_CONFIG_DEFAULT_WEB_CGI_PATTERN           "**.cgi$|**.pl$|**.php|**.py"

#define VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT          "web_cgi_environment"
#define VSCPDB_CONFIG_DEFAULT_WEB_CGI_ENVIRONMENT       ""

#define VSCPDB_CONFIG_NAME_WEB_PROTECT_URI              "web_protect_uri"
#define VSCPDB_CONFIG_DEFAULT_WEB_PROTECT_URI           ""

#define VSCPDB_CONFIG_NAME_WEB_TROTTLE                  "web_trottle"
#define VSCPDB_CONFIG_DEFAULT_WEB_TROTTLE               ""

#define VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING     "web_enable_directory_listing"
#define VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_DIRECTORY_LISTING  "1"

#define VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE        "web_enable_keep_alive"
#define VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_KEEP_ALIVE     "0"

#define VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS    "web_keep_alive_timeout_ms"
#define VSCPDB_CONFIG_DEFAULT_WEB_KEEP_ALIVE_TIMEOUT_MS "500"

#define VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST      "web_access_control_list"
#define VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_LIST   ""

#define VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES         "web_extra_mime_types"
#define VSCPDB_CONFIG_DEFAULT_WEB_EXTRA_MIME_TYPES      ""

#define VSCPDB_CONFIG_NAME_WEB_NUM_THREADS              "web_num_threads"
#define VSCPDB_CONFIG_DEFAULT_WEB_NUM_THREADS           "50"

#define VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER              "web_run_as_user"
#define VSCPDB_CONFIG_DEFAULT_WEB_RUN_AS_USER           ""

#define VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS     "web_url_rewrite_patterns"
#define VSCPDB_CONFIG_DEFAULT_WEB_URL_REWRITE_PATTERNS  ""

#define VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS       "web_hide_file_patterns"
#define VSCPDB_CONFIG_DEFAULT_WEB_HIDE_FILE_PATTERNS    ""

#define VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS       "web_request_timeout_ms"
#define VSCPDB_CONFIG_DEFAULT_WEB_REQUEST_TIMEOUT_MS    "30000"

#define VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS        "web_linger_timeout_ms"
#define VSCPDB_CONFIG_DEFAULT_WEB_LINGER_TIMEOUT_MS     "-1"

#define VSCPDB_CONFIG_NAME_WEB_DECODE_URL               "web_decode_url"
#define VSCPDB_CONFIG_DEFAULT_WEB_DECODE_URL            "1"

#define VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE          "web_global_authfile"
#define VSCPDB_CONFIG_DEFAULT_WEB_GLOBAL_AUTHFILE       ""

#define VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE           "web_per_directory_auth_file"
#define VSCPDB_CONFIG_DEFAULT_WEB_PER_DIRECTORY_AUTH_FILE       ""

#define VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS                     "web_ssi_patterns"
#define VSCPDB_CONFIG_DEFAULT_WEB_SSI_PATTERNS                  ""

#define VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN      "web_access_control_allow_origin"
#define VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_ORIGIN   ""

#define VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS     "web_access_control_allow_methods"
#define VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_METHODS  ""

#define VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS     "web_access_control_allow_headers"
#define VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_HEADERS  ""

#define VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES                      "web_error_pages"
#define VSCPDB_CONFIG_DEFAULT_WEB_ERROR_PAGES                   ""

#define VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY                     "web_tcp_nodelay"
#define VSCPDB_CONFIG_DEFAULT_WEB_TCP_NO_DELAY                  "-1"

#define VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE              "web_static_file_max_age"
#define VSCPDB_CONFIG_DEFAULT_WEB_STATIC_FILE_MAX_AGE           "3600"

#define VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE    "web_strict_transport_security_max_age"
#define VSCPDB_CONFIG_DEFAULT_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE "-1"

#define VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL              "web_allow_sendfile_call"
#define VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_SENDFILE_CALL           "1"

#define VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS               "web_additional_headers"
#define VSCPDB_CONFIG_DEFAULT_WEB_ADDITIONAL_HEADERS            ""

#define VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE                 "web_max_request_size"
#define VSCPDB_CONFIG_DEFAULT_WEB_MAX_REQUEST_SIZE              "16384"

#define VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE      "web_allow_index_script_resource"
#define VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_INDEX_SCRIPT_RESOURCE   "0"

#define VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN           "web_duktape_script_pattern"
#define VSCPDB_CONFIG_DEFAULT_WEB_DUKTAPE_SCRIPT_PATTERN        "**.ssjs$"

#define VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE                 "web_lua_preload_file"
#define VSCPDB_CONFIG_DEFAULT_WEB_LUA_PRELOAD_FILE              ""

#define VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN               "web_lua_script_pattern"
#define VSCPDB_CONFIG_DEFAULT_WEB_LUA_SCRIPT_PATTERN            "**.lua$"

#define VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN          "web_lua_server_page_pattern"
#define VSCPDB_CONFIG_DEFAULT_WEB_LUA_SERVER_PAGE_PATTERN       "**.lp$|**.lsp$"

#define VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN            "web_lua_websocket_pattern"
#define VSCPDB_CONFIG_DEFAULT_WEB_LUA_WEBSOCKET_PATTERN         "**.lua$"

#define VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT            "web_lua_background_script"
#define VSCPDB_CONFIG_DEFAULT_WEB_LUA_BACKGROUND_SCRIPT         ""

#define VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS     "web_lua_background_script_params"
#define VSCPDB_CONFIG_DEFAULT_WEB_LUA_BACKGROUND_SCRIPT_PARAMS  ""

// Websockets

#define VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE                     "websocket_enable"
#define VSCPDB_CONFIG_DEFAULT_WEBSOCKET_ENABLE                  "1"

#define VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT              "websocket_document_root"
#define VSCPDB_CONFIG_DEFAULT_WEBSOCKET_DOCUMENT_ROOT           "/srv/vscp/web"

#define VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS                 "websocket_timeout_ms"
#define VSCPDB_CONFIG_DEFAULT_WEBSOCKET_TIMEOUT_MS              "30000"


// Automation

#define VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE                    "automation_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_ENABLE                 "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_ZONE                      "automation_zone"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_ZONE                   "11"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE                   "automation_subzone"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUBZONE                "22"

#define VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE                 "automation_longitude"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_LONGITUDE              "61.7441833"

#define VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE                  "automation_latitude"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_LATITUDE               "15.1604167"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE            "automation_sunrise_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNRISE_ENABLE         "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE             "automation_sunset_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNSET_ENABLE          "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE    "automation_sunset_twilight_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNSET_TWILIGHT_ENABLE "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE    "automation_sunrise_twilight_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNRISE_TWILIGHT_ENABLE "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE       "automation_segment_ctrl_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SEGMENT_CTRL_ENABLE    "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL     "automation_segment_ctrl_interval"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_SEGMENT_CTRL_INTERVAL  "60"

#define VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE          "automation_heartbeat_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_HEARTBEAT_ENABLE       "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL        "automation_heartbeat_interval"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_HEARTBEAT_INTERVAL     "60"

#define VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE       "automation_capabilities_enable"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_CAPABILITIES_ENABLE    "1"

#define VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL     "automation_capabilities_interval"
#define VSCPDB_CONFIG_DEFAULT_AUTOMATION_CAPABILITIES_INTERVAL  "60"




//*****************************************************************************
//                                    LOG
//*****************************************************************************

#define VSCPDB_LOG_CREATE "CREATE TABLE IF NOT EXISTS 'log' ("\
	"`idx_log`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`type`         INTEGER DEFAULT 0,"\
	"`date`         TEXT,"\
	"`level`	INTEGER DEFAULT 0,"\
	"`message`	TEXT"\
        ");";\

#define VSCPDB_LOG_INSERT "INSERT INTO 'log' "\
                "(type,date,level,message) VALUES ('%d','%s','%d','%q');"

#define VSCPDB_LOG_COUNT "SELECT COUNT(*) AS nrows FROM log;"

#define VSCPDB_LOG_CREATE_INDEX "CREATE INDEX `idxdate` ON log ('date');"

// Delete records older than n days
#define VSCPDB_LOG_DELETE_OLD "DELETE FROM log WHERE date < datetime('now', '-%d days');"

#define VSCPDB_ORDINAL_LOG_ID                       0   //
#define VSCPDB_ORDINAL_LOG_TYPE                     1   //
#define VSCPDB_ORDINAL_LOG_DATE                     2   //
#define VSCPDB_ORDINAL_LOG_LEVEL                    3   //
#define VSCPDB_ORDINAL_LOG_MESSAGE                  4   //





//*****************************************************************************
//                                   USER
//*****************************************************************************

/*
 * Defines users
 *
 * permissions
 * ===========
 * uuugggooo
 * uuu – user
 * ggg – group  (not used)
 * ooo - other
 *
 * Each group is rw- and other permissions may be added added in front of this.
 *
 * filter a filter for incoming traffic. Default is open.
 *
 * permission   - Default user permissions uuugggooo
 * rights       - string with a maximum of six numerical (byte) comma separated
 *                  fields. Also 'admin', 'driver' and 'user' can be given.
 */

#define VSCPDB_USER_CREATE  "CREATE TABLE IF NOT EXISTS 'user' ("\
	"`idx_user`             INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`username`             TEXT NOT NULL UNIQUE,"\
	"`password`             TEXT NOT NULL,"\
	"`fullname`             TEXT NOT NULL,"\
        "`filter`               TEXT,"\
        "`rights`               TEXT DEFAULT '00/00/00/00/00/00/00/00',"\
        "`allowedevents`        TEXT DEFAULT '*:*',"\
        "`allowedremotes`       TEXT DEFAULT '*',"\
	"`note`                 TEXT DEFAULT ''"\
        ");"

#define VSCPDB_USER_CREATE_INDEX "CREATE INDEX `idxusername` "\
                "ON user ('username'):"

#define VSCPDB_USER_INSERT "INSERT INTO 'user' "\
                "(username,password,fullname,filter,rights,allowedevents,allowedremotes,note "\
                " ) VALUES ('%s','%s','%s','%s',%d,'%s','%s','%s' );"

#define VSCPDB_USER_UPDATE "UPDATE 'user' "\
                "SET username='%s',password='%s',fullname='%s',filter='%s',rights='%s',allowedevents='%s',allowedremotes='%s',note='%s' "\
                " WHERE idx_user='%ld'"

#define VSCPDB_USER_ALL "SELECT * from 'user'"

#define VSCPDB_USER_CHECK_USER "SELECT idx_user from 'user' WHERE username='%s'"

#define VSCPDB_USER_CHECK_USER_ID "SELECT username FROM 'user' where idx_user='%lu'"

#define VSCPDB_USER_DELETE_USERNAME "DELETE FROM 'user' where username='%s'"

#define VSCPDB_ORDINAL_USER_ID                      0   //
#define VSCPDB_ORDINAL_USER_USERNAME                1   //
#define VSCPDB_ORDINAL_USER_PASSWORD                2   //
#define VSCPDB_ORDINAL_USER_FULLNAME                3   //
#define VSCPDB_ORDINAL_USER_FILTER                  4   //
#define VSCPDB_ORDINAL_USER_RIGHTS                  5   //
#define VSCPDB_ORDINAL_USER_ALLOWED_EVENTS          6   //
#define VSCPDB_ORDINAL_USER_ALLOWED_REMOTES         7   //
#define VSCPDB_ORDINAL_USER_NOTE                    8   //


//*****************************************************************************
//                                  DRIVER
//*****************************************************************************

/*
 * List drivers (Level I and Level II) used by the system
 * bEnable True for driver to be loaded.
 * level Currently Level I (0) or Level II (1)
 * name Name for driver.
 * link_to_guid Pointer to GUID for this driver. All drivers must have a GUID.
 * configuration Semicolon separated configuration string for driver.
 * path Path to driver dl/dll or for level III the url (tcp://127.0.0.1:5005)
 * flags Flags for driver functionality
 * port - for Level II devices.
 * translation Semicolon separated list. Translate Level I events to Level II
 *      (measurements float/string)
 * note Driver notations and/or information.
 */

#define VSCPDB_DRIVER_CREATE "CREATE TABLE IF NOT EXISTS `driver` ("\
	"`idx_driver`       INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`bEnable`          INTEGER NOT NULL,"\
	"`level`            INTEGER NOT NULL DEFAULT 0,"\
	"`name`             TEXT NOT NULL,"\
	"`link_to_guid`     INTEGER,"\
	"`configuration`    TEXT,"\
	"`path`             TEXT,"\
	"`flags`            INTEGER,"\
        "`translation`      TEXT,"\
	"`note`             TEXT"\
        ");"

#define VSCPDB_DRIVER_CREATE_INDEX "CREATE INDEX `idxdrivername` "\
                                   "ON driver ('name'):"

#define VSCPDB_DRIVER_ALL "SELECT * from 'driver'"

#define VSCPDB_ORDINAL_DRIVER_ID                    0   //
#define VSCPDB_ORDINAL_DRIVER_ENABLE                1   //
#define VSCPDB_ORDINAL_DRIVER_LEVEL                 2   //
#define VSCPDB_ORDINAL_DRIVER_NAME                  3   //
#define VSCPDB_ORDINAL_DRIVER_LINK_TO_GUID          4   //
#define VSCPDB_ORDINAL_DRIVER_CONFIGURATION         5   //
#define VSCPDB_ORDINAL_DRIVER_PATH                  6   //
#define VSCPDB_ORDINAL_DRIVER_FLAGS                 7   //
#define VSCPDB_ORDINAL_DRIVER_TRANSLATION           8   //
#define VSCPDB_ORDINAL_DRIVER_NOTE                  9   //


//*****************************************************************************
//                              GUID (Discovery)    
//*****************************************************************************

/*
 * 
 * GUID table
 *
 * type - describes what this GUID is describing, for example an interface, a node etc.
 *       Defined in knownnodes.h  (may be old info. here)
 *   type = 0 - Common GUID.
 *   type = 1 - Interface (on this machine),
 *   type = 2 - Level I hardware. Lives on one of the interfaces of this daemon.
 *   type = 3 - Level II hardware. Lives somewhere given by address.
 *   type = 4 - Level III hardware.
 *   type = 5 - Dumb device (no registers, no MDF).
 *   type = 6 - Driver.
 *   type = 7 - Location.
 *   type = 8 - User interface component.
 *
 * GUID - GUID for the type. Level I hardware use the proxy GUID.
 *
 * date - date time when discovered in ISO format. YY-MM-DDTHH:MM:SS
 *
 * name - Max 64 byte Unicode (UTF8) name set by administrator
 * 
 * devicename - Name set on node in MDF
 *
 * link_to_mdf - For a hardware device.
 *
 * address - is IPv4/IPv6/BT-UID or other address for a Level II hardware type
 *
 * capabilities - see CLASS2.PROTOCOL, Type=20   A 8 byte comma separated list 
 *                with the bytes of the capability 64-bit code.
 *
 * nonstandard - is one or more nonstandard port info as described for 
 *               CLASS2.PROTOCOL, Type=20 in text form in groups of three
 *               service1,port1-msb,port1-lsb,service2,port2-msb,port2-lsb ...
 *
 * description - Device description (UTF8).
 */

#define VSCPDB_GUID_CREATE "CREATE TABLE IF NOT EXISTS `guid` ("\
	"`idx_guid`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`type`         INTEGER NOT NULL DEFAULT 0,"\
	"`guid`         TEXT DEFAULT '00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',"\
        "`date`         TEXT,"\
        "`name`         TEXT,"\
        "`link_to_mdf`	INTEGER DEFAULT 0,"\
        "`address`      TEXT,"\
        "`capabilities` TEXT,"\
        "`nonstandard`  TEXT,"\
        "`description`	TEXT "\
        ");"

#define VSCPDB_GUID_CREATE_INDEX "CREATE INDEX `idxguid` "\
                                 "ON guid ('name'):"

#define VSCPDB_GUID_SELECT_ALL "SELECT * from 'guid'"
#define VSCPDB_GUID_SELECT_PAGE  "SELECT * FROM guid LIMIT %d,%d;"
#define VSCPDB_GUID_SELECT_ID    "SELECT * FROM guid WHERE idx_guid=%ld;"

#define VSCPDB_GUID_DELETE_ID  "DELETE FROM 'guid' WHERE idx_guid=%ld"

#define VSCPDB_GUID_INSERT "INSERT INTO 'guid' "\
                "(type,guid,date,name,link_to_mdf,address,capabilities,nonstandard,description "\
                " ) VALUES (%d,'%s','%s','%q',%d,'%q','%q','%q','%q' );"

#define VSCPDB_GUID_UPDATE       "UPDATE guid SET type=%d,guid='%s',date='%s',"\
                                 "name='%q',link_to_mdf=%d,address='%q',"\
                                 "capabilities='%q',nonstandard='%q',description='%q' "\
                                 " WHERE idx_guid=%ld;"

#define VSCPDB_ORDINAL_GUID_ID                      0   //
#define VSCPDB_ORDINAL_GUID_TYPE                    1   //
#define VSCPDB_ORDINAL_GUID_GUID                    2   //
#define VSCPDB_ORDINAL_GUID_DATE                    3   //
#define VSCPDB_ORDINAL_GUID_NAME                    4   //
#define VSCPDB_ORDINAL_GUID_LINK_TO_MDF             5   //
#define VSCPDB_ORDINAL_GUID_ADDRESS                 6   //
#define VSCPDB_ORDINAL_GUID_CAPABILITIES            7   //
#define VSCPDB_ORDINAL_GUID_NONSTANDARD             8   //
#define VSCPDB_ORDINAL_GUID_DESCRIPTION             9   //


//*****************************************************************************
//                                 ZONE
//*****************************************************************************

#define VSCPDB_ZONE_CREATE "CREATE TABLE IF NOT EXISTS 'zone' ("\
	"`idx_zone`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`name`         TEXT NOT NULL,"\
	"`description`	TEXT"\
        ");"

#define VSCPDB_ZONE_CREATE_INDEX "CREATE INDEX `idxzone` "\
                                 "ON subzone ('name'):"
#define VSCPDB_ZONE_SELECT_ALL   "SELECT * FROM zone;"
#define VSCPDB_ZONE_SELECT_PAGE  "SELECT * FROM zone LIMIT %d,%d;"
#define VSCPDB_ZONE_SELECT_ID    "SELECT * FROM zone WHERE idx_zone=%ld;"
#define VSCPDB_ZONE_UPDATE       "UPDATE zone SET name='%s',description='%s' WHERE idx_zone=%ld;"

#define VSCPDB_ORDINAL_ZONE_ID                   0   //
#define VSCPDB_ORDINAL_ZONE_NAME                 1   // User names of zone
#define VSCPDB_ORDINAL_ZONE_DESCRIPTION          2   // User describes of zone


//*****************************************************************************
//                                  SUBZONE
//*****************************************************************************

#define VSCPDB_SUBZONE_CREATE "CREATE TABLE IF NOT EXISTS `subzone` ("\
	"`idx_subzone`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`name`         TEXT NOT NULL,"\
	"`description`	TEXT"\
        ");"

#define VSCPDB_SUBZONE_CREATE_INDEX "CREATE INDEX `idxsubzone` "\
                                    "ON subzone ('name'):"
#define VSCPDB_SUBZONE_SELECT_ALL   "SELECT * FROM subzone;"
#define VSCPDB_SUBZONE_SELECT_PAGE  "SELECT * FROM subzone LIMIT %d,%d;"
#define VSCPDB_SUBZONE_SELECT_ID    "SELECT * FROM subzone WHERE idx_subzone=%ld;"
#define VSCPDB_SUBZONE_UPDATE       "UPDATE subzone SET name='%s',description='%s' WHERE idx_subzone=%ld;"

#define VSCPDB_ORDINAL_SUBZONE_ID               0   //
#define VSCPDB_ORDINAL_SUBZONE_NAME             1   // User name of subzone
#define VSCPDB_ORDINAL_SUBZONE_DESCRIPTION      2   // User describes of zone



//*****************************************************************************
//                                  LOCATION
//*****************************************************************************

/*
 * Defines locations.
 * A GUID can also be used to identify a location. If so link_to_guid points to it.
 */

#define VSCPDB_LOCATION_CREATE "CREATE TABLE IF NOT EXISTS `location` ("\
	"`idx_location`         INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`link_to_zone`         INTEGER,"\
	"`link_to_subzone`	INTEGER,"\
	"`link_to_guid`         INTEGER,"\
        "`name`                 TEXT NOT NULL UNIQUE,"\
        "`description`	I       TEXT,"\
        "FOREIGN KEY (link_to_zone) REFERENCES zone(idxzone) "\
        "ON UPDATE SET NULL ON DELETE SET NULL,"\
        "FOREIGN KEY (link_to_subzone) REFERENCES zone(idxsubzone) "\
        "ON UPDATE SET NULL ON DELETE SET NULL,"\
        "FOREIGN KEY (link_to_guid) REFERENCES zone(idxguid) "\
        "ON UPDATE SET NULL ON DELETE SET NULL"\
        ");"

#define VSCPDB_LOCATION_CREATE_INDEX "CREATE INDEX `idxlocation` "\
                "ON location ('name'):"

#define VSCPDB_LOCATION_ALL          "SELECT * from 'location'"
#define VSCPDB_LOCATION_SELECT_PAGE  "SELECT * FROM location LIMIT %d,%d;"
#define VSCPDB_LOCATION_SELECT_ID    "SELECT * FROM location WHERE idx_location=%ld;"

#define VSCPDB_LOCATION_DELETE_ID  "DELETE FROM 'location' WHERE idx_location=%ld"

#define VSCPDB_LOCATION_INSERT "INSERT INTO 'location' "\
                "(link_to_zone,link_to_subzone,link_to_guid,name,description "\
                " ) VALUES (%d,%d,%d,'%q','%q' );"

#define VSCPDB_LOCATION_UPDATE   "UPDATE location SET link_to_zone=%d,"\
                                 "link_to_subzone=%d,link_to_guid=%d,"\
                                 "name='%q',description='%q' "\
                                 "WHERE idx_location=%ld;"

#define VSCPDB_ORDINAL_LOCATION_ID                  0   //
#define VSCPDB_ORDINAL_LOCATION_LINK_TO_ZONE        1   //
#define VSCPDB_ORDINAL_LOCATION_LINK_TO_SUBZONE     2   //
#define VSCPDB_ORDINAL_LOCATION_LINK_TO_GUID        3   //
#define VSCPDB_ORDINAL_LOCATION_NAME                4   //
#define VSCPDB_ORDINAL_LOCATION_DESCRIPTION         5   //





//*****************************************************************************
//                               MDF_CACHE
//*****************************************************************************

/*
 * Loaded MDF's are cached. This record points to the loaded MDF
 *
 * name     - (first) device name from mdf
 * mdf      - The MDF file content.
 * picture  - Picture of device
 * date     - When the MDF was fetched.
 * guid     - GUID for the device.
 */
#define VSCPDB_MDF_CREATE "CREATE TABLE IF NOT EXISTS 'mdf' ("\
	"`idx_mdf`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
        "`name`         TEXT,"\
        "`date`         TEXT,"\
	"`url`          TEXT,"\
	"`mdf`          TEXT,"\
	"`picture`	BLOB,"\
        "`guid`         TEXT DEFAULT '00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00'"\
        ");"

#define VSCPDB_MDF_CREATE_INDEX "CREATE INDEX `idx_mdf` "\
                                "ON mdf_cache ('url'):"
#define VSCPDB_MDF_SELECT_ALL   "SELECT * FROM mdf;"
#define VSCPDB_MDF_SELECT_ALL_DATE   "SELECT * FROM mdf ORDER BY name;"

#define VSCPDB_ORDINAL_MDF_ID                 0   //
#define VSCPDB_ORDINAL_MDF_NAME               1   //
#define VSCPDB_ORDINAL_MDF_DATE               2   //
#define VSCPDB_ORDINAL_MDF_URL                3   //
#define VSCPDB_ORDINAL_MDF_FILE_PATH          4   //
#define VSCPDB_ORDINAL_MDF_PICTURE_PATH       5   //
#define VSCPDB_ORDINAL_MDF_GUID               6   //

//*****************************************************************************
//                                SIMPLEUI
//*****************************************************************************

/*
 * Defines a simple UI
 */

#define VSCPDB_SIMPLE_UI_CREATE "CREATE TABLE IF NOT EXISTS 'simpleui' ("\
	"`idx_simpleui`     INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`name`             TEXT NOT NULL UNIQUE,"\
	"`link_to_ower`     INTEGER NOT NULL,"\
	"`permission`       INTEGER DEFAULT 777,"\
        "`comment`          TEXT"\
        ");";

#define VSCPDB_SIMPLE_UI_CREATE_INDEX "CREATE INDEX `idxsimpleui` "\
                "ON simpleui ('name'):"

#define VSCPDB_ORDINAL_SIMPLE_UI_NAME               1   //
#define VSCPDB_ORDINAL_SIMPLE_UI_LINK_TO_OWNER      2   //
#define VSCPDB_ORDINAL_SIMPLE_UI_PERMISSION         3   //
#define VSCPDB_ORDINAL_SIMPLE_UI_COMMENT            4   //


//*****************************************************************************
//                              SIMPLEUI_ITEM
//*****************************************************************************

/*
 * Defines a simple UI item
 * param_... defines contents for the row type
 */

#define VSCPDB_SIMPLE_UI_ITEM_CREATE "CREATE TABLE IF NOT EXISTS `simpleui_item` ("\
	"`idx_simpleui_item`	INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
	"`link_to_simpleui`	INTEGER NOT NULL,"\
        "`param_left`           TEXT NOT NULL,"\
        "`param_middle`         TEXT NOT NULL,"\
        "`param_right`          TEXT NOT NULL,"\
	"`page`                 INTEGER,"\
	"`sortorder`            INTEGER,"\
	"`rowtype`              INTEGER DEFAULT 0"\
        ");"

#define VSCPDB_SIMPLE_UI_ITEM_CREATE_INDEX "CREATE INDEX `idxsimpleui_item` "\
                "ON simpleui_item ('link_to_simpleui'):"

#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_ID                0   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_LINK_TO_SIMPLEUI  1   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_PARAM_LEFT        2   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_PARAM_MIDDLE      3   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_PARAM_RIGHT       4   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_PAGE              5   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_SORT_ORDER        6   //
#define VSCPDB_ORDINAL_SIMPLE_UI_ITEM_ROW_TYPE          7   //






//*****************************************************************************
//                                 TABLE
//*****************************************************************************

/*
 * User defined tables with diagram hints
 * Databases are always created in the 'table' sub folder of the server root.
 *
 * bmem - Is true of the table is a in-memory database.
 * name - Unique name of table.
 * xname - Text on xaxis.
 * yname - Text on yaxis.
 * title - Text for diagram title.
 * note - Text to display as note on diagram..
 * size - A specific size for a table with a defined size (round robin). Normally
 *          zero for a ever growing table-.
 * sql_create - SQL expression to use to create table
 * sql_insert - SQL expression to insert value. The value should be set as
 *  The SQL expression can contain VSCP decision matrix escapes which are
 *  filled in before the SQL expression is evaluated.
 * sql_delete - SQL expression to delete value.
 * description - User description for table
 */

#define VSCPDB_TABLE_CREATE "CREATE TABLE IF NOT EXISTS 'table' ("\
	"`idx_table`        INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
        "`bEnable`          INTEGER NOT NULL,"\
        "`bmem`             INTEGER NOT NULL,"\
	"`name`             TEXT NOT NULL,"\
	"`link_to_user`     TEXT NOT NULL DEFAULT 0,"\
	"`permission`       INTEGER NOT NULL DEFAULT 1729,"\
        "`type`             INTEGER NOT NULL DEFAULT 0,"\
        "`size`             INTEGER NOT NULL DEFAULT 0,"\
	"`xname`            TEXT NOT NULL,"\
	"`yname`            TEXT NOT NULL,"\
	"`title`            TEXT NOT NULL,"\
	"`note`             TEXT NOT NULL,"\
	"`sql_create`       TEXT NOT NULL,"\
	"`sql_insert`       TEXT NOT NULL,"\
        "`sql_delete`       TEXT NOT NULL,"\
        "`description`      TEXT NOT NULL,"\
        "`vscpclass`        INTEGER NOT NULL DEFAULT 10,"\
        "`vscptype`         INTEGER NOT NULL DEFAULT 0,"\
        "`vscpsensoridx`    INTEGER NOT NULL DEFAULT 0,"\
        "`vscpunit`         INTEGER NOT NULL DEFAULT 0,"\
        "`vscpzone`         INTEGER NOT NULL DEFAULT 255,"\
        "`vscpsubzone`      INTEGER NOT NULL DEFAULT 255"\
        ");"

#define VSCPDB_TABLE_INSERT "INSERT INTO 'table' "\
                "(bEnable,bmem,name,link_to_user,permission,type,size,"\
                "xname,yname,title,note,sql_create,sql_insert,sql_delete,description,"\
                "vscpclass,vscptype,vscpsensoridx,vscpunit,vscpzone,vscpsubzone "\
                " ) VALUES ('%d','%d','%s','%d','%d','%d','%lu',"\
                "'%q','%q','%q','%q','%q','%q','%q','%q',"\
                "'%d','%d','%d','%d','%d','%d' );"

#define VSCPDB_TABLE_UPDATE "UPDATE 'table' "\
                "SET benable='%d',"\
                "bmem='%d',"\
                "permission='%d',"\
                "type='%d',"\
                "size='%lu',"\
                "xman='%s',"\
                "yname='%s' "\
                "title='%s',"\
                "note='%s',"\
                "sql_create='%s',"\
                "sql_insert='%s',"\
                "sql_delete='%s',"\
                "description='%s',"\
                "vscpclass='%d',"\
                "vscptype='%d',"\
                "vscpsensorindex='%d',"\
                "vscpunit='%d',"\
                "vscpzone='%d',"\
                "vscpsubzone='%d'"\
                " WHERE link_to_user='%ld';"

// Get the columns of a user table (table name inserted in %s)
#define VSCPDB_TABLE_GET_COLUMNS "pragma table_info ('vscptable');"

// If the table is a static table we may need to delete the oldest records here
// to keep the table size constant.
// Keep last n
#define VSCPDB_TABLE_DELETE_STATIC "DELETE FROM 'vscptable' WHERE ROWID IN (SELECT ROWID FROM 'vscptable' ORDER BY ROWID DESC LIMIT -1 OFFSET %lu);"

// Delete last n
#define VSCPDB_TABLE_DELETE_LAST "DELETE FROM 'vscptable' WHERE ROWID IN (SELECT ROWID FROM 'vscptable' ORDER BY ROWID ASC LIMIT %lu);"

// Default table create SQL expression. Use if sqlcreate is empty
#define VSCPDB_TABLE_DEFAULT_CREATE "CREATE TABLE 'vscptable' ( `idx` INTEGER NOT NULL PRIMARY KEY UNIQUE, `datetime` TEXT, `value` REAL DEFAULT 0 );"

// Default table insert SQL expression. Use if sqlinsert is empty
#define VSCPDB_TABLE_DEFAULT_INSERT "INSERT INTO 'vscptable' (datetime,value) VALUES ('%%s','%%lf');"

// Default table delete SQL expression. Delete all records in vscptable
#define VSCPDB_TABLE_DEFAULT_DELETE "DELETE FROM vscptable;"

// Default table delete range SQL expression. Delete records in date range in vscptable
#define VSCPDB_TABLE_DELETE_RANGE "DELETE FROM vscptable;"

// Count number of records in database.
#define VSCPDB_TABLE_COUNT "SELECT COUNT(*) FROM vscptable;"

// Get count number of records in database + data.
#define VSCPDB_TABLE_COUNT_AND_DATA "SELECT (SELECT COUNT() FROM 'vscptable') AS 'count', * FROM 'vscptable';"

// Get record count over range
#define VSCPDB_TABLE_COUNT_RANGE "SELECT COUNT(*) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get first date from database.
#define VSCPDB_TABLE_DATE_FIRST "SELECT datetime FROM vscptable ORDER BY datetime ASC LIMIT 1;"

// Get last date from database.
#define VSCPDB_TABLE_DATE_LAST "SELECT datetime FROM vscptable ORDER BY datetime DESC LIMIT 1;"

// Select date, time and the rest of the columns in a range
#define VSCPDB_TABLE_SELECT_STANDARD_RANGE "SELECT datetime,value,* FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Select date, time and the rest of the columns in a range
#define VSCPDB_TABLE_SELECT_CUSTOM_RANGE "SELECT datetime,value FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get sum for a range
#define VSCPDB_TABLE_GET_SUM "SELECT SUM(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get min for a range
#define VSCPDB_TABLE_GET_MIN "SELECT MIN(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get max for a range
#define VSCPDB_TABLE_GET_MAX "SELECT MAX(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get average for a range
#define VSCPDB_TABLE_GET_AVG "SELECT AVG(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get median for a range
#define VSCPDB_TABLE_GET_MEDIAN "SELECT MEDIAN(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get standard deviation
#define VSCPDB_TABLE_GET_STDDEV "SELECT STDEV(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get variance
#define VSCPDB_TABLE_GET_VARIANCE "SELECT VARIANCE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get mode
#define VSCPDB_TABLE_GET_MODE "SELECT MODE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get lower quartile
#define VSCPDB_TABLE_GET_LOWER_QUARTILE "SELECT LOWER_QUARTILE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

// Get upper quartile
#define VSCPDB_TABLE_GET_UPPER_QUARTILE "SELECT UPPER_QUARTILE(value) FROM 'vscptable' WHERE datetime(datetime) between '%s' AND '%s';"

#define VSCPDB_ORDINAL_TABLE_ID                 0   //
#define VSCPDB_ORDINAL_TABLE_ENABLE             1   //
#define VSCPDB_ORDINAL_TABLE_BMEM               2   //
#define VSCPDB_ORDINAL_TABLE_NAME               3   //
#define VSCPDB_ORDINAL_TABLE_LINK_TO_USER       4   //
#define VSCPDB_ORDINAL_TABLE_PERMISSION         5   //
#define VSCPDB_ORDINAL_TABLE_TYPE               6   //
#define VSCPDB_ORDINAL_TABLE_SIZE               7   //
#define VSCPDB_ORDINAL_TABLE_XNAME              8   //
#define VSCPDB_ORDINAL_TABLE_YNAME              9   //
#define VSCPDB_ORDINAL_TABLE_TITLE              10  //
#define VSCPDB_ORDINAL_TABLE_NOTE               11  //
#define VSCPDB_ORDINAL_TABLE_SQL_CREATE         12  //
#define VSCPDB_ORDINAL_TABLE_SQL_INSERT         13  //
#define VSCPDB_ORDINAL_TABLE_SQL_DELETE         14  //
#define VSCPDB_ORDINAL_TABLE_DESCRIPTION        15  //
#define VSCPDB_ORDINAL_TABLE_VSCP_CLASS         16  //
#define VSCPDB_ORDINAL_TABLE_VSCP_TYPE          17  //
#define VSCPDB_ORDINAL_TABLE_VSCP_SENSOR_INDEX  18  //
#define VSCPDB_ORDINAL_TABLE_VSCP_UNIT          19  //
#define VSCPDB_ORDINAL_TABLE_VSCP_ZONE          20  //
#define VSCPDB_ORDINAL_TABLE_VSCP_SUBZONE       21  //


//*****************************************************************************
//                               UDP nodes
//*****************************************************************************

#define VSCPDB_UDPNODE_CREATE "CREATE TABLE IF NOT EXISTS 'udpnode' ("\
	"`idx_udpnode`      INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
        "`bEnable`          INTEGER DEFAULT 0,"\
        "`interface`        TEXT NOT NULL,"\
	"`filter`           TEXT NOT NULL,"\
        "`mask`             TEXT NOT NULL,"\
        "`encryption`       TEXT NOT NULL,"\
        "`bSetBroadcast`    TEXT DEFAULT 0"\
        ");";\

#define VSCPDB_UDPNODE_UPDATE "UPDATE 'udpnode' "\
                "SET benable='%d',"\
                "interface='%s',"\
                "filter='%s',"\
                "mask='%s' "\
                "encryption='%s',"\
                "bSetBroadcast='%d',"\
                " WHERE idx_udpnode='%ld';"

#define VSCPDB_UDPNODE_INSERT "INSERT INTO 'udpnode' "\
                "(bEnable,interface,filter,mask,encryption,bSetBroadcast)"\
                " VALUES ('%d',%q','%q','%d','%d','%d');"

#define VSCPDB_ORDINAL_UDPNODE_IDX              0   //
#define VSCPDB_ORDINAL_UDPNODE_ENABLE           1   //
#define VSCPDB_ORDINAL_UDPNODE_INTERFACE        2   //
#define VSCPDB_ORDINAL_UDPNODE_FILTER           3   //
#define VSCPDB_ORDINAL_UDPNODE_MASK             4   //
#define VSCPDB_ORDINAL_UDPNODE_ENCRYPTION       5   //
#define VSCPDB_ORDINAL_UDPNODE_SET_BROADCAST    6   //


//*****************************************************************************
//                               MULTICAST
//*****************************************************************************

#define VSCPDB_MULTICAST_CREATE "CREATE TABLE IF NOT EXISTS 'multicast' ("\
	"`idx_multicast`    INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
        "`bEnable`          INTEGER DEFAULT 0,"\
	"`group`            TEXT DEFAULT 'udp://224.0.23.158:44444',"\
        "`public`           TEXT DEFAULT '192.168.1.55',"\
        "`port`             INTEGER DEFAULT 44444,"\
	"`ttl`              INTEGER DEFAULT 1,"\
        "`guid`             TEXT,"\
	"`txfilter`         TEXT,"\
        "`txmask`           TEXT,"\
        "`rxfilter`         TEXT,"\
        "`rxmask`           TEXT,"\
        "`encryption`       TEXT DEFAULT 'none',"\
        "`bSendAck`         INTEGER DEFAULT 0,"\
        "`bAllowUnsecure`   INTEGER DEFAULT 1"\
        ");";\

#define VSCPDB_MULTICAST_UPDATE "UPDATE 'multicast' "\
                "SET benable='%d',"\
                "group='%s',"\
                "public='%s',"\
                "port='%d',"\
                "ttl='%d',"\
                "guid='%s',"\
                "txfilter='%s',"\
                "txmask='%s' "\
                "rxfilter='%s',"\
                "rxmask='%s',"\
                "encryption='%s',"\
                "bsendack='%d',"\
                "ballowunsecure='%d',"\
                " WHERE idx_multicast='%ld';"

#define VSCPDB_MULTICAST_INSERT "INSERT INTO 'multicast' "\
                "(bEnable,group,public,port,ttl,guid,txfilter,txmask,rxfilter,rxmask,encryption,bsendack,allowunsecure)"\
                " VALUES ('%d',%q','%q','%q','%d','%d','%q','%q','%q','%q','%q','%d','%d');"

#define VSCPDB_ORDINAL_MULTICAST_IDX                0   //
#define VSCPDB_ORDINAL_MULTICAST_ENABLE             1   //
#define VSCPDB_ORDINAL_MULTICAST_GROUP              2   //
#define VSCPDB_ORDINAL_MULTICAST_PUBLIC             3   //
#define VSCPDB_ORDINAL_MULTICAST_PORT               4   //
#define VSCPDB_ORDINAL_MULTICAST_TTL                5   //
#define VSCPDB_ORDINAL_MULTICAST_GUID               6   //
#define VSCPDB_ORDINAL_MULTICAST_TXFILTER           7   //
#define VSCPDB_ORDINAL_MULTICAST_TXMASK             8   //
#define VSCPDB_ORDINAL_MULTICAST_RXFILTER           9   //
#define VSCPDB_ORDINAL_MULTICAST_RXMASK             10  //
#define VSCPDB_ORDINAL_MULTICAST_ENCRYPTION         11  //
#define VSCPDB_ORDINAL_MULTICAST_SENDACK            12  //
#define VSCPDB_ORDINAL_MULTICAST_ALLOW_UNSECURE     13  //








// ----------------------------------------------------------------------------




//*****************************************************************************
//                                VARIABLE
//*****************************************************************************

//
// bstock - True for a stock variable. A stock variable is virtual but is
//          in the database for sorting, searching and listing of variables.
//              lastchange is always set to program start
//              value is always  empty
//              name set to correct name
//              type set to correct type
//              bpersistent set to false
//              link_to-user = 0 (admin)
//              permission depends on the actual variable
//          ONLY!!!!  PERSISTENT  !!!!   no use for persistent variables
//

#define VSCPDB_VARIABLE_CREATE  "CREATE TABLE IF NOT EXISTS 'variable' ("\
                        "'idx_variable'     INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
                        "'bstock'           INTEGER NOT NULL DEFAULT 0,"\
                        "'lastchange'       TEXT NOT NULL,"\
                        "'name'             TEXT NOT NULL UNIQUE,"\
                        "'type'             INTEGER NOT NULL DEFAULT 0,"\
                        "'value'            TEXT NOT NULL,"\
                        "'bPersistent'      INTEGER NOT NULL DEFAULT 0,"\
                        "'link_to_user'     INTEGER NOT NULL,"\
                        "'permission'       INTEGER NOT NULL DEFAULT 777,"\
                        "'note'	TEXT"\
                        ");"

#define VSCPDB_VARIABLE_UPDATE "UPDATE 'variable' "\
                                        "SET lastchange='%s', "\
                                        "name='%q', "\
                                        "type='%d', "\
                                        "value='%q', "\
                                        "bPersistent='%d', "\
                                        "link_to_user='%d', "\
                                        "permission='%d', "\
                                        "note='%q' "\
                                        "WHERE idx_variable='%ld';"

#define VSCPDB_VARIABLE_INSERT "INSERT INTO 'variable' "\
                        "(lastchange,name,type,value,bPersistent,link_to_user,permission,note) "\
                        "VALUES ('%s','%s', '%d','%q','%d','%d','%d','%q');"

#define VSCPDB_VARIABLE_FIND_ALL "SELECT * FROM 'variable'"

#define VSCPDB_VARIABLE_FIND_FROM_NAME "SELECT * FROM 'variable' WHERE name='%s'"

#define VSCPDB_VARIABLE_WITH_ID  "DELETE FROM 'variable' WHERE idx_variable='%ld';"

#define VSCPDB_ORDINAL_VARIABLE_ID              0   //
#define VSCPDB_ORDINAL_VARIABLE_BSTOCK          1   //
#define VSCPDB_ORDINAL_VARIABLE_LASTCHANGE      2   //
#define VSCPDB_ORDINAL_VARIABLE_NAME            3   //
#define VSCPDB_ORDINAL_VARIABLE_TYPE            4   //
#define VSCPDB_ORDINAL_VARIABLE_VALUE           5   //
#define VSCPDB_ORDINAL_VARIABLE_PERSISTENT      6   //
#define VSCPDB_ORDINAL_VARIABLE_LINK_TO_USER    7   //
#define VSCPDB_ORDINAL_VARIABLE_PERMISSION      8   //
#define VSCPDB_ORDINAL_VARIABLE_NOTE            9   //





// ----------------------------------------------------------------------------





//*****************************************************************************
//                                     DM
//*****************************************************************************



#define VSCPDB_DM_CREATE  "CREATE TABLE IF NOT EXISTS 'dm' ("\
	"`idx_dm`                       INTEGER NOT NULL PRIMARY KEY UNIQUE,"\
        "`GroupID`                      TEXT NOT NULL,"\
	"`bEnable`                      INTEGER NOT NULL DEFAULT 0,"\
	"`maskPriority`                 INTEGER NOT NULL DEFAULT 0,"\
	"`maskClass`                    NUMERIC NOT NULL DEFAULT 0,"\
	"`maskType`                     INTEGER NOT NULL DEFAULT 0,"\
	"`maskGUID`                     TEXT NOT NULL DEFAULT '00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00',"\
	"`filterPriority`               INTEGER NOT NULL DEFAULT 0,"\
	"`filterClass`                  INTEGER NOT NULL DEFAULT 0,"\
	"`filterType`                   INTEGER NOT NULL DEFAULT 0,"\
	"`filterGUID`                   TEXT NOT NULL,"\
	"`allowedFrom`                  TEXT NOT NULL,"\
	"`allowedTo`                    TEXT NOT NULL,"\
	"`allowedMonday`                INTEGER NOT NULL,"\
	"`allowedTuesday`               INTEGER NOT NULL,"\
	"`allowsWednesday`              INTEGER NOT NULL,"\
	"`allowedThursday`              INTEGER NOT NULL,"\
	"`allowedFriday`                INTEGER NOT NULL,"\
	"`allowedSaturday`              NUMERIC NOT NULL,"\
	"`allowedSunday`                INTEGER NOT NULL,"\
	"`allowedTime`                  TEXT NOT NULL,"\
	"`bCheckIndex`                  INTEGER NOT NULL DEFAULT 0,"\
	"`index`                        INTEGER NOT NULL DEFAULT 0,"\
	"`bCheckZone`                   INTEGER NOT NULL DEFAULT 0,"\
	"`zone`                         INTEGER NOT NULL DEFAULT 0,"\
	"`bCheckSubZone`                INTEGER NOT NULL DEFAULT 0,"\
	"`subzone`                      INTEGER NOT NULL DEFAULT 0,"\
	"`bCheckMeasurementIndex`	INTEGER NOT NULL DEFAULT 0,"\
	"`actionCode`                   INTEGER NOT NULL,"\
	"`actionParameter`              TEXT NOT NULL,"\
        "`bCheckMeasurementValue`	INTEGER NOT NULL DEFAULT 0,"\
	"`measurementValue`             REAL,"\
	"`measurementUnit`              INTEGER,"\
	"`measurementCompare`           INTEGER,"\
	"`comment`                      TEXT"\
    ");"

#define VSCPDB_DM_INSERT "INSERT INTO 'dm' "\
                "(GroupID,bEnable,maskPriority,maskClass,maskType,maskGUID,filterPriority,filterClass,filterType,filterGUID,"\
                "allowedFrom,allowedTo,allowedMonday,allowedTuesday,allowsWednesday,allowedThursday,allowedFriday,allowedSaturday,"\
                "allowedSunday,allowedTime,bCheckIndex,'index',bCheckZone,zone,bCheckSubZone,subzone,bCheckMeasurementIndex,"\
                "actionCode,actionParameter,bCheckMeasurementValue,measurementValue,measurementUnit,measurementCompare,comment"\
                " )VALUES ('%q','%d','%d','%d','%d','%q','%d','%d','%d','%q',"\
                "'%q','%q','%d','%d','%d','%d','%d','%d',"\
                "'%d','%q','%d','%d','%d','%d','%d','%d','%d',"\
                "'%d','%q','%d','%lf','%d','%d','%q'"\
                ");"

#define VSCPDB_DM_UPDATE "UPDATE 'dm' "\
                "SET GroupID='%q',bEnable='%d',maskPriority='%d',maskClass='%d',maskType='%d',maskGUID='%q',filterPriority='%d',filterClass='%d',filterType='%d',filterGUID='%q',"\
                "allowedFrom='%q',allowedTo='%q',allowedMonday='%d',allowedTuesday='%d',allowsWednesday='%d',allowedThursday='%d',allowedFriday='%d',allowedSaturday='%d',"\
                "allowedSunday='%d',allowedTime='%q',bCheckIndex='%d','index'='%d',bCheckZone='%d',zone='%d',bCheckSubZone='%d',subzone='%d',bCheckMeasurementIndex='%d',"\
                "actionCode='%d',actionParameter='%q',bCheckMeasurementValue='%d',measurementValue='%lf',measurementUnit='%d',measurementCompare='%d',comment='%q'"\
                " WHERE idx_dm='%d';"

#define VSCPDB_DM_UPDATE_ITEM "UPDATE 'dm' SET ( %s='%s' ) WHERE id='%d' ;"

#define VSCPDB_ORDINAL_DM_ID                        0   //
#define VSCPDB_ORDINAL_DM_GROUPID                   1   //
#define VSCPDB_ORDINAL_DM_ENABLE                    2   //
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
#define VSCPDB_ORDINAL_DM_ACTIONCODE                28   //
#define VSCPDB_ORDINAL_DM_ACTIONPARAMETER           29   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_CHECK_VALUE   30   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_VALUE         31   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_UNIT          32   //
#define VSCPDB_ORDINAL_DM_MEASUREMENT_COMPARE       33   //
#define VSCPDB_ORDINAL_DM_COMMENT                   34   //




#endif  // compile

