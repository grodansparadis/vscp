// websrv.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#if !defined(WEBSRV_H__INCLUDED_)
#define WEBSRV_H__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"

#include "userlist.h"
#include "websocket.h"

WX_DECLARE_STRING_HASH_MAP( wxString, HashString );

/*!
 * Init the webserver sub system
 */
int start_webserver( void );

/*!
 * Init the webserver sub system
 */
int stop_webserver( void );


/*!
 * Send header
 */
void websrv_sendheader( struct web_connection *conn, 
                            int returncode, 
                            const char *pcontent );

/*!
 * Send header set 'sessionid' cookie
 */
void websrv_sendSetCookieHeader( struct web_connection *conn, 
                                    int returncode, 
                                    const char *pcontent,
                                    const char *psid );


////////////////////////////////////////////////////////////////////////////////
//                           ws1  Websocket handlers
////////////////////////////////////////////////////////////////////////////////

int ws1_connectHandler( const struct web_connection *conn, void *cbdata );
void ws1_readyHandler(struct web_connection *conn, void *cbdata);
int ws1_dataHandler( struct web_connection *conn,
                        int bits,
                        char *data,
                        size_t len,
                        void *cbdata );
void ws1_closeHandler(const struct web_connection *conn, void *cbdata );



#define WEBSRV_MAX_SESSIONS                     1000    // Max web server active sessions
#define WEBSRV_NAL_USERNAMELEN                  128	    // Max length for userdname


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


/*static const char *html_form =
  "<html><body>POST example."
  "<form method=\"POST\" action=\"/handle_post_request\">"
  "Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
  "Input 2: <input type=\"text\" name=\"input_2\" /> <br/>"
  "<input type=\"submit\" />"
  "</form></body></html>";
*/

/**
 * State we keep for each user/session/browser.
 */
struct websrv_session
{
  // Unique ID for this session. 
  char m_sid[33];

  // Time when this session was last active.
  time_t lastActiveTime;

  // Client item for this session
  CClientItem *m_pClientItem;

  // User
  CUserItem *m_pUserItem;
    
  // String submitted via form.
  //char value_1[64];

  // Another value submitted via form.
  //char value_2[64];
  
  // Keypairs
  //HashString m_keys; 
  
};


WX_DECLARE_LIST(struct websrv_session, WEBSRVSESSIONLIST);


// Test Certificate 

const char server_cert_pem[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDBjCCAe4CCQDDIH/hK1C0BjANBgkqhkiG9w0BAQsFADBFMQswCQYDVQQGEwJB\n"
"VTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0\n"
"cyBQdHkgTHRkMB4XDTE3MDkwMzE5MjIyOVoXDTI3MDkwMTE5MjIyOVowRTELMAkG\n"
"A1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0\n"
"IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"ALUmHEoJcebkUOyqEAhH2OdEuTTk8AxjjVvq9B1dXjlf/dvxGnZX2InScGCJA9Uy\n"
"kO1XI8nLXKAGl6OL9jDt/0K3/oFLedDLtZf1qE+kEBuaqAgL+VVAPqwtQZcyCoI9\n"
"zx777I1tPUOl1Q1ass3T7lYsTN8QADmW5zjJn4MJPMQ55qoQUL7HVQR4VJ/ELAXu\n"
"xGkQlJFBY5q0Qq6buN102D2upNKXKpDYYPc0OgyJ73fR2+rzQapc52QD4Oh6cbD8\n"
"Fh5Vh/qGNMckh1cQsVm6fRtlkoUqxANZk58rqkEwOuk04p7vlnVvZTidOng7G2nW\n"
"1n7YQXCycI+JhofCqOqT9x8CAwEAATANBgkqhkiG9w0BAQsFAAOCAQEATx5GZCxU\n"
"KKQCDsafzAwoodbjRlpsJhvdCBGpgMrFTPyQo7BNF/E2XyVCDXbCmbxTRlhFafJG\n"
"Loj/73toGkU8+1qUIy/Fffsmeh9YCyMlA2bE+85ccMCVKgCIEx0+fa6Au6/Ref7/\n"
"n7vN/9deJzxWUaNbP26LNq3prbuIbKN6WFNT5mR8HLTmP3O45sqy1jwOZgSwvbgH\n"
"bhugE4tSsKghMV5rUgiMhGIrEakFH+1LCZjQh+ojcWWEWyVk3QTQMmSd6tAZf4pb\n"
"/Y1GuN6DAiLfzbabUQZCeQ1iZcgrwIOGHWJUPAf+BTPcFLlR3k/kYA9lrqvra7ln\n"
"dFIuUv3YzfenfA==\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAtSYcSglx5uRQ7KoQCEfY50S5NOTwDGONW+r0HV1eOV/92/Ea\n"
"dlfYidJwYIkD1TKQ7VcjyctcoAaXo4v2MO3/Qrf+gUt50Mu1l/WoT6QQG5qoCAv5\n"
"VUA+rC1BlzIKgj3PHvvsjW09Q6XVDVqyzdPuVixM3xAAOZbnOMmfgwk8xDnmqhBQ\n"
"vsdVBHhUn8QsBe7EaRCUkUFjmrRCrpu43XTYPa6k0pcqkNhg9zQ6DInvd9Hb6vNB\n"
"qlznZAPg6HpxsPwWHlWH+oY0xySHVxCxWbp9G2WShSrEA1mTnyuqQTA66TTinu+W\n"
"dW9lOJ06eDsbadbWfthBcLJwj4mGh8Ko6pP3HwIDAQABAoIBAGgaacGGogW+Cl+n\n"
"8CTCHX3y+bjTJL0J7S/426eQg9jXOI3QhpOiMlgqLtjbhO9d6vnqzS9oBmgUwcqE\n"
"YcyGyd5u3P0zAeOjXk3hKIP0Vil2/L/7GaQLkrjiHUKlyHJG0SQORUiVkdKxl7nf\n"
"+Mfe1qaBOQAsMuTluyXggSIOCfT+FdHoi6nr/+Nugyx7e/UrZ3GWHVbh8KXOlvHh\n"
"kETfcI6KUkWKtE+YJx9w89Bjh8TBvU0nkOntR11T2SMNllyIS9nND8pqa7QPz3N0\n"
"Ag/iN4Wh8S5f4Nn4GccAOtIORuYuw9Pmt1E9dFWEna1fGztBHlClFQPOLUhZ+/zR\n"
"MfQV5bkCgYEA3pQTLZ5ldX1Kvg5sYw63wwewr147R0pav6AoJ8HTnWGqi5y485CX\n"
"uKE/IcJseidG9FmkO7rfexQaBtW9eW0GCVru416VSP9g2r1iUu0ViaqctYt7ZacE\n"
"UEI+g4FmaXHyn1CKTjJXgUAdoDbtlyHwLmLmNt+B3zKGa1lPIb5MwdMCgYEA0Fl7\n"
"VCTnmdyFH8m/bK76uW7SgkYmKYd5AvDr2QFCSqY3tdZh2VIukoUPmheCFq0kpDc0\n"
"+eT680rF/m6CCu+00nM6v/3TNARRANeQ2G73kTPpyiphE+ttKCBQ/tke3TcHQA85\n"
"7cI6bfkMonyKi0JRdLs4QEWf86Avr6p6JKdQWgUCgYEA3oAT8+SF9D89umRcwWFz\n"
"HcnQPF7sz0VrFmiZ+7RtQMTjYhFXalQ+91hp7euX2TzuV1JNNVCIG1dq9S4x7PKp\n"
"uCxo5m4kugZg4gm0AsXyY95kLa+zuViOnVS7fWab5Aj+y3gN6kG07AYWF5URSaWp\n"
"nhVLocso3uB5M1LiIg9EV/UCgYBNrN6Wyz9xFE6pQDzWlxGwakme+eomV3RdDVbQ\n"
"S3DchcWFTEykicgFJghgCV2deKWNd2uPsreAVqMkLSzcSOuf/gesJkREQ0uzxaoh\n"
"lpVDlBgYH96bX40NhabMrEOec3KHhmWxZ1UDRPNZ7JZ2Pp5Bp77b71knqdO9aRAq\n"
"dBo3xQKBgQCnxheQbozuzPO/6nixAng1GP1GuuB2bVb4e5Z0+0dt2RfI8+MSqnSL\n"
"q9Yr2+p/fJFkVthrOUYwJkMf7ujhK2uNCJ7aKmwHPSIRztNV3UDGFd9wgpj3Pebx\n"
"36ahCvDzidTEG+EEra6zPJ1An3KEbPsfXwcy1NVEZ/kFQyzczL0AOw==\n"
"-----END RSA PRIVATE KEY-----\n";


const char client_cert_pem[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDBjCCAe4CCQCFpskbTEyGpTANBgkqhkiG9w0BAQsFADBFMQswCQYDVQQGEwJB\n"
"VTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0\n"
"cyBQdHkgTHRkMB4XDTE3MDkwMzE5MjIwNVoXDTI3MDkwMTE5MjIwNVowRTELMAkG\n"
"A1UEBhMCQVUxEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoMGEludGVybmV0\n"
"IFdpZGdpdHMgUHR5IEx0ZDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"ANyFUuYxv/uexSr/K9aSmcnEcylNH4S3NdlvMwFvW3XFqAV05tV6HnPnSELEk6t3\n"
"8aMDUGKDBrrjwsVK6+S7OyrkioXeB9dWldHbqD7o3MkIM3sUxUtaR6x0RMZ+sIX4\n"
"XpE0xULcip1bG0etP4Z2frEP2IOOValQcm4SCnKYZJyTr/oR31NmlIPU/47s74U6\n"
"rqwwUE92bzvf1jGeUHEn7IAgSJNIUBNsOIdRQAMBuTJIAmG2qawXaetjLi/NBwNS\n"
"d0OX2v3o9SrA+ZhQYpPG5xp3B3ncHgVvmhmp7hUdlYbiemcUHn18hZjxPVZLbtY8\n"
"gQldrWyMZkVabSZjuIH3IKcCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAUZsxxYVK\n"
"l0tH8E0FCnRJTvG6gjOeiqJRIk7Mmg+hfFZK/ewqBixxg1OBM/xmPXfnI/ULRz74\n"
"UMXnyDIsGakzrFDqWqPt3xots35yHHo2ZkVao6gV4qx0Reu86qeN5iRvG0EjoGMD\n"
"7XRaw56E0XhvMBJW1CiUg944HSw4ptJli0dJCYa+P9s1Fop3lA0d9+dwKMKUyCDr\n"
"yBz4XjyO9jXSQC/t0fkxC4gHhdH/ZaAq0Lem6Xxc40ZwoVc1+dHWFxn8d6L/RYvb\n"
"16gOuw6s2Xt9h2K8OFKzehOgNZAkI2oUELRFUx9Wc8/Bcl6uEkBmPHRqeX5l35jo\n"
"ztBrpAEsCy0cGg==\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEA3IVS5jG/+57FKv8r1pKZycRzKU0fhLc12W8zAW9bdcWoBXTm\n"
"1Xoec+dIQsSTq3fxowNQYoMGuuPCxUrr5Ls7KuSKhd4H11aV0duoPujcyQgzexTF\n"
"S1pHrHRExn6whfhekTTFQtyKnVsbR60/hnZ+sQ/Yg45VqVBybhIKcphknJOv+hHf\n"
"U2aUg9T/juzvhTqurDBQT3ZvO9/WMZ5QcSfsgCBIk0hQE2w4h1FAAwG5MkgCYbap\n"
"rBdp62MuL80HA1J3Q5fa/ej1KsD5mFBik8bnGncHedweBW+aGanuFR2VhuJ6ZxQe\n"
"fXyFmPE9Vktu1jyBCV2tbIxmRVptJmO4gfcgpwIDAQABAoIBAEpiBlZzTYi4Q1V/\n"
"gO/9vzYZt6akxw7jJZzUL2Y6g6U0KLq+deZoLMF3sB4lZJIgATe1NHYmMCz2Coq1\n"
"/N/Ib+rF8Bu7ivWN1TdWWmft8Bs3UvYfSXVjXG3FQjWaIjzuTCe6nxcwgOkXBBqn\n"
"S5g1fAKJj8TATBCyfAa4uyFwWe+eGRs0W9pOMP8eU0EtvTer34rSU4L/LG3d7UcI\n"
"upm/0T5QeLqv6Htv8UbHNQto701vJQVdWLavALMXGfGO112yTSz7OpitKpBEYDrV\n"
"3+781zYm8AKkFIsRMXVK2HiBEF43zIrnNuoozsKpps/tZdlv9VqCSJ4hIaHm9mxJ\n"
"3zMN3OECgYEA8dr5w68jTLrthDZ2qOG/6tZw9fMfXoF7hSUXplgxMN5Sohfr23Xm\n"
"/IHVm7oiqhDNNZzplGyux7jB00x2/1ltOzay5mx4PMMLlsDBgiURgUwqS8C8dPVh\n"
"0sN2RytdKGDmFP6lnKS7c15CEw1ChvdL4RwtqzjTKE0ZOK3zUY5/MykCgYEA6Wru\n"
"Dusip4p4PA1K6eiCoC6SaqCuQCB7ZR5WPR5szAFkgoW63rNtC8S4Bl1qXXUb/v/V\n"
"ptaVsGrqBc8/CxvCac1KCREbcyjuVWUAfw2VwdwgDbfrEieWrZNvsDs86EgB+Bo4\n"
"Jm/cUjrFqSTJAbtvp4SYl1reax86XmCsHhNNf08CgYEApAhxd9/0IBlz+ET8K8SY\n"
"5sy0ZouTjgRh40bqCF8uVcej4d45kGoh1Ma2Ot1+nzuwApm+7nTcAgd0JjxpRPzB\n"
"EfUiVxfgYM2ksYVgeUVs3vXqheBdsTGwPENnmBN4Jme6BSlE573uiOu4ArXulh1p\n"
"sG7tJoDu7hmEbqXELl9oNCkCgYEA51zWGnN3JhpakyuZ1cBhueRvvMEH9wg7Rz+K\n"
"u4oszQmUVsu3Locqzz9uKODvTTOHTHrJi1WnifZvgNKr6pbZXYXenJ4YV01676nt\n"
"lAIjLsTCANcMajJTaDl7u3L8LEEzsnhKr86w09Dtm3qawtzHD4Seu2eWjxelA2dP\n"
"M4BukIECgYAn5n+HhCi5JD3I1VCX70uE5nj8alYyQ85qE57Lopmau1RyVfP4oeCt\n"
"gMsy0o7vIF+xW1Z2yDxm+mJghOY/myDsbTGX9G8rY7PC7tWE8okjsQT5UoayFzKp\n"
"mmvrTV8TQBVcTQqn0Jyj7T5MBnuwfioXYN9pKPQlvc4pPmHbqPi7CA==\n"
"-----END RSA PRIVATE KEY-----\n";


/**
 * How many bytes of a file do we give to libmagic to determine the mime type?
 * 16k might be a bit excessive, but ought not hurt performance much anyway,
 * and should definitively be on the safe side.
 */
#define WEBSERVER_MAGIC_HEADER_SIZE (16 * 1024)


bool websrv_parseHeader( wxArrayString &valarray, wxString &header );

bool websrv_getHeaderElement( wxArrayString &valarray, 
                                const wxString &name,
                                wxString &value );

/*!
    This class implement the VSCP Webserver thread
*/







#endif
