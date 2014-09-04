// webserver.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2014 
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


#define WEBSRV_MAX_SESSIONS						1000	// Max web server active sessions
#define WEBSRV_NAL_USERNAMELEN					128		// Max length for userdname


/**
 * Invalid method page.
 */
#define WEBSERVER_METHOD_ERROR "<html><head><title>Illegal request</title></head><body>Invalid method.</body></html>"

/**
 * Invalid URL page.
 */
#define WEBSERVER_NOT_FOUND_ERROR "<html><head><title>Not found</title></head><body>The page you are looking for is not available on this server.</body></html>"

/**
 * Start page
 */
#define WEBSERVER_PAGE_MAIN "<html><head><title>VSCP Daemon</title></head><body>Welcome to the VSCP Daemon.</body></html>"

/**
 * Start page
 */
#define WEBSERVER_PAGE "<html><head><title>VSCP Daemon</title></head><body>VSCP Daemon.</body></html>"

/**
 * Available in next version
 */
#define NEXTVERSION_PAGE "<html><head><title>VSCP Daemon</title></head><body>Sorry this functionality is not available until next version of VSCP & Friends.</body></html>"

/**
 * Invalid password
 */
#define WEBSERVER_DENIED "<html><head><title>Access denied</title></head><body>Access denied</body></html>"

/**
 * Name of our cookie.
 */
#define WEBSERVER_COOKIE_NAME "____vscp_session____"


static const char *html_form =
  "<html><body>POST example."
  "<form method=\"POST\" action=\"/handle_post_request\">"
  "Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
  "Input 2: <input type=\"text\" name=\"input_2\" /> <br/>"
  "<input type=\"submit\" />"
  "</form></body></html>";


/**
 * State we keep for each user/session/browser.
 */
struct websrv_Session
{
  // We keep all sessions in a linked list.
  struct websrv_Session *m_next;

  // Unique ID for this session. 
  char m_sid[33];

  /**
   * Reference counter giving the number of connections
   * currently using this session.
   */
  unsigned int m_referenceCount;

  // Time when this session was last active.
  time_t lastActiveTime;

  // Client item for this session
  CClientItem *m_pClientItem;

  // User
  CUserItem *m_pUserItem;
    
  // String submitted via form.
  char value_1[64];

  // Another value submitted via form.
  char value_2[64];
  
  // Keypairs
  HashString m_keys;
  
};





/* Test Certificate */
const char cert_pem[] =
  "-----BEGIN CERTIFICATE-----\n"
  "MIICpjCCAZCgAwIBAgIESEPtjjALBgkqhkiG9w0BAQUwADAeFw0wODA2MDIxMjU0\n"
  "MzhaFw0wOTA2MDIxMjU0NDZaMAAwggEfMAsGCSqGSIb3DQEBAQOCAQ4AMIIBCQKC\n"
  "AQC03TyUvK5HmUAirRp067taIEO4bibh5nqolUoUdo/LeblMQV+qnrv/RNAMTx5X\n"
  "fNLZ45/kbM9geF8qY0vsPyQvP4jumzK0LOJYuIwmHaUm9vbXnYieILiwCuTgjaud\n"
  "3VkZDoQ9fteIo+6we9UTpVqZpxpbLulBMh/VsvX0cPJ1VFC7rT59o9hAUlFf9jX/\n"
  "GmKdYI79MtgVx0OPBjmmSD6kicBBfmfgkO7bIGwlRtsIyMznxbHu6VuoX/eVxrTv\n"
  "rmCwgEXLWRZ6ru8MQl5YfqeGXXRVwMeXU961KefbuvmEPccgCxm8FZ1C1cnDHFXh\n"
  "siSgAzMBjC/b6KVhNQ4KnUdZAgMBAAGjLzAtMAwGA1UdEwEB/wQCMAAwHQYDVR0O\n"
  "BBYEFJcUvpjvE5fF/yzUshkWDpdYiQh/MAsGCSqGSIb3DQEBBQOCAQEARP7eKSB2\n"
  "RNd6XjEjK0SrxtoTnxS3nw9sfcS7/qD1+XHdObtDFqGNSjGYFB3Gpx8fpQhCXdoN\n"
  "8QUs3/5ZVa5yjZMQewWBgz8kNbnbH40F2y81MHITxxCe1Y+qqHWwVaYLsiOTqj2/\n"
  "0S3QjEJ9tvklmg7JX09HC4m5QRYfWBeQLD1u8ZjA1Sf1xJriomFVyRLI2VPO2bNe\n"
  "JDMXWuP+8kMC7gEvUnJ7A92Y2yrhu3QI3bjPk8uSpHea19Q77tul1UVBJ5g+zpH3\n"
  "OsF5p0MyaVf09GTzcLds5nE/osTdXGUyHJapWReVmPm3Zn6gqYlnzD99z+DPIgIV\n"
  "RhZvQx74NQnS6g==\n" "-----END CERTIFICATE-----\n";

const char key_pem[] =
  "-----BEGIN RSA PRIVATE KEY-----\n"
  "MIIEowIBAAKCAQEAtN08lLyuR5lAIq0adOu7WiBDuG4m4eZ6qJVKFHaPy3m5TEFf\n"
  "qp67/0TQDE8eV3zS2eOf5GzPYHhfKmNL7D8kLz+I7psytCziWLiMJh2lJvb2152I\n"
  "niC4sArk4I2rnd1ZGQ6EPX7XiKPusHvVE6VamacaWy7pQTIf1bL19HDydVRQu60+\n"
  "faPYQFJRX/Y1/xpinWCO/TLYFcdDjwY5pkg+pInAQX5n4JDu2yBsJUbbCMjM58Wx\n"
  "7ulbqF/3lca0765gsIBFy1kWeq7vDEJeWH6nhl10VcDHl1PetSnn27r5hD3HIAsZ\n"
  "vBWdQtXJwxxV4bIkoAMzAYwv2+ilYTUOCp1HWQIDAQABAoIBAArOQv3R7gmqDspj\n"
  "lDaTFOz0C4e70QfjGMX0sWnakYnDGn6DU19iv3GnX1S072ejtgc9kcJ4e8VUO79R\n"
  "EmqpdRR7k8dJr3RTUCyjzf/C+qiCzcmhCFYGN3KRHA6MeEnkvRuBogX4i5EG1k5l\n"
  "/5t+YBTZBnqXKWlzQLKoUAiMLPg0eRWh+6q7H4N7kdWWBmTpako7TEqpIwuEnPGx\n"
  "u3EPuTR+LN6lF55WBePbCHccUHUQaXuav18NuDkcJmCiMArK9SKb+h0RqLD6oMI/\n"
  "dKD6n8cZXeMBkK+C8U/K0sN2hFHACsu30b9XfdnljgP9v+BP8GhnB0nCB6tNBCPo\n"
  "32srOwECgYEAxWh3iBT4lWqL6bZavVbnhmvtif4nHv2t2/hOs/CAq8iLAw0oWGZc\n"
  "+JEZTUDMvFRlulr0kcaWra+4fN3OmJnjeuFXZq52lfMgXBIKBmoSaZpIh2aDY1Rd\n"
  "RbEse7nQl9hTEPmYspiXLGtnAXW7HuWqVfFFP3ya8rUS3t4d07Hig8ECgYEA6ou6\n"
  "OHiBRTbtDqLIv8NghARc/AqwNWgEc9PelCPe5bdCOLBEyFjqKiT2MttnSSUc2Zob\n"
  "XhYkHC6zN1Mlq30N0e3Q61YK9LxMdU1vsluXxNq2rfK1Scb1oOlOOtlbV3zA3VRF\n"
  "hV3t1nOA9tFmUrwZi0CUMWJE/zbPAyhwWotKyZkCgYEAh0kFicPdbABdrCglXVae\n"
  "SnfSjVwYkVuGd5Ze0WADvjYsVkYBHTvhgRNnRJMg+/vWz3Sf4Ps4rgUbqK8Vc20b\n"
  "AU5G6H6tlCvPRGm0ZxrwTWDHTcuKRVs+pJE8C/qWoklE/AAhjluWVoGwUMbPGuiH\n"
  "6Gf1bgHF6oj/Sq7rv/VLZ8ECgYBeq7ml05YyLuJutuwa4yzQ/MXfghzv4aVyb0F3\n"
  "QCdXR6o2IYgR6jnSewrZKlA9aPqFJrwHNR6sNXlnSmt5Fcf/RWO/qgJQGLUv3+rG\n"
  "7kuLTNDR05azSdiZc7J89ID3Bkb+z2YkV+6JUiPq/Ei1+nDBEXb/m+/HqALU/nyj\n"
  "P3gXeQKBgBusb8Rbd+KgxSA0hwY6aoRTPRt8LNvXdsB9vRcKKHUFQvxUWiUSS+L9\n"
  "/Qu1sJbrUquKOHqksV5wCnWnAKyJNJlhHuBToqQTgKXjuNmVdYSe631saiI7PHyC\n"
  "eRJ6DxULPxABytJrYCRrNqmXi5TCiqR2mtfalEMOPxz8rUU8dYyx\n"
  "-----END RSA PRIVATE KEY-----\n";

/**
 * How many bytes of a file do we give to libmagic to determine the mime type?
 * 16k might be a bit excessive, but ought not hurt performance much anyway,
 * and should definitively be on the safe side.
 */
#define WEBSERVER_MAGIC_HEADER_SIZE (16 * 1024)


//******************************************************************************
//                                     WEBSOCKETS
//******************************************************************************

#define MAX_VSCPWS_MESSAGE_QUEUE 512

// Authentication states
enum {
	WEBSOCK_AUTH_STAGE_START = 0,
	WEBSOCK_AUTH_STAGE_SERVER_HASH,
	WEBSOCK_AUTH_STAGE_CLIENT_HASH
};

enum {
	WEBSOCK_ERROR_NO_ERROR = 0,			// Everything is OK.			
	WEBSOCK_ERROR_SYNTAX_ERROR,			// Syntax error.
	WEBSOCK_ERROR_UNKNOWN_COMMAND,		// Unknown command.
	WEBSOCK_ERROR_TX_BUFFER_FULL,		// Transmit buffer full.
	WEBSOCK_ERROR_VARIABLE_DEFINED,		// Variable is already defined.
	WEBSOCK_ERROR_VARIABLE_UNKNOWN,		// Cant find variable
	WEBSOCK_ERROR_NOT_AUTHORIZED,		// Not authorized
};

#define	WEBSOCK_STR_ERROR_NO_ERROR				"Everything is OK"			
#define WEBSOCK_STR_ERROR_SYNTAX_ERROR			"Syntax error"
#define WEBSOCK_STR_ERROR_UNKNOWN_COMMAND		"Unknown command"
#define WEBSOCK_STR_ERROR_TX_BUFFER_FULL		"Transmit buffer full"
#define WEBSOCK_STR_ERROR_VARIABLE_DEFINED		"Variable is already defined"
#define WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN		"Unable to find variable"
#define WEBSOCK_STR_ERROR_NOT_AUTHORIZED		"Not authorized"

struct websock_session {
	// We keep all sessions in a linked list.
	struct websock_session *m_next;

	// Unique ID for this session. 
	char m_key[33];		// Sec-WebSocket-Key

	// Generated hash for this session
	char m_sid[33];

	// Prorocol version
	int m_version;		// Sec-WebSocket-Key

	// Reference counter giving the number of connections
	// currently using this session.
	unsigned int m_referenceCount;

	// This variable is true when the logon process
	// is valid and the usr is logged in.
	bool bAuthenticated;

	// Time when this session was last active.
	time_t lastActiveTime;
    
	//wxArrayString *pMessageList;	// Messages (not events) to client.

	// Client structure for websocket
    CClientItem *pClientItem;		

	// User structure for websocket
	CUserItem *m_pUserItem;

    bool bTrigger;					// True to activate trigger functionality.
    uint32_t triggerTimeout;		// Time out before trigg (or error) must occur.
    TRIGGERLIST listTriggerOK;		// List with positive triggers.
    TRIGGERLIST listTriggerERR;		// List with negative triggers.
};
