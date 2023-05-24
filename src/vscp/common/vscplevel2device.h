// vscplevel2device.cpp : Defines the initialization routines for the 
// Level II driver DL/DLL.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C)2000-2023 Ake Hedman, the VSCP project
// <info@vscp.org>
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

#if !defined(vscpl2deviceif_H__INCLUDED_)
#define vscpl2deviceif_H__INCLUDED_

#include <string>

#include <dlfcn.h>

#include <level2drvdef.h>

// Format code for object data
#define CANAL_FORMAT_CAN_XML        0 // XML formated CAN message
#define CANAL_FORMAT_CAN_JSON       1 // JSON formated CAN message
#define CANAL_FORMAT_VSCP_XML       2 // XML formated VSCP event
#define CANAL_FORMAT_VSCP_JSON      3 // JSON formatted VSCP event

// Used by send XML parser
typedef struct comboMsg
{
    bool bCan; // True if CAN message
    canalMsg msg;
    vscpEvent ev;
} comboMsg;

class vscpl2deviceif
{

  public:
    /// Constructor
    vscpl2deviceif();

    /// Destructor
    virtual ~vscpl2deviceif();

    // Setters
    void setPath(std::string path) { m_strPath = path; };
    void setParameter(std::string param) { m_strParameter = param; };
    void setFlags(uint32_t flags) { m_deviceFlags = flags; };
    void setbAsync(bool bAsync) { m_bAsync = bAsync; };


    /*!
        Initialize driver
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int init(void);

    /*!
        Initialize driver

        @param strpath CANAL driver path string
        @param strparam CANAL configuration string
        @param flags CANAL flags
        @param bAsync True to enable driver asynchronous functionally
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */

    int init(std::string strpath,
             std::string strparam,
             uint32_t flags,
             bool bAsync = false);

    /*!
        Initialize driver with JSON data

        @param jsonInit Init data as a JSON object
        XML format
        ----------
        <init path="path"
            config="config"
            flags="flags"
            basync= "true|false" />
        JSON format
        -----------
        {
            "path"   : "path to CANAL driver",
            "config" : "CANAL configuration string",
            "flags"  : 12345
            "basync" : true|false
        }
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int init(std::string objInit, uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        CanalOpen
    */
    int vscpOpen();

    /*!
        CanalClose
    */
    int vscpClose();

    /*!
        Send CAN message

        @param pCanMsg Pointer to CANAL message to send
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpWrite(const PCANALMSG pCanMsg);

    /*!
        Send CAN message

        @param msg CAN msg to send on XML/JSON format

        XML format
        ----------
        <msg>
            <can flags="1234"
                    obid="1234"
                    id="1234"
                    data="1,2,3,4.."
            />
            <vscp head=""
                vscpclass=""
                vscptype=""
                data="1,2,3..."
            />
        </msg>

        JSON format
        ------------
        {
            "can_flags"     : 1234
            "can_obid"      : 1234
            "can_id"        : 1234
            "can_data"      : [1,2,3]

            “vscp_head”     : 0,
            "vscp_obid"     : 0,
            "vscp_guid"     : "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
            “vscp_class”    : 10,
            “vscp_type”     : 6,
            "vscp_data"     : [1,2,3,4],
        }
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSend(std::string& msg, uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        Send CAN message

        @param pCanMsg Pointer to CANAL message to send
        @param timeout timeout - Timeout in milliseconds. 0 (default) to wait
       forever.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpBlockingSend(const PCANALMSG pCanMsg,
                          uint32_t timeout = CANAL_BLOCK_FOREVER);

    /*!
        Send CAN message, block

        @param msg CAN msg to send on XML/JSON format
        @param timeout timeout - Timeout in milliseconds. 0 (default) to wait
       forever.
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpBlockingSend(std::string& msg,
                          uint32_t timeout = CANAL_BLOCK_FOREVER,
                          uint8_t nFormat  = CANAL_FORMAT_CAN_XML);

    /*!
        Receive CANAL message

        @param pCanMsg Pointer to struct that receive CAN message
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpReceive(PCANALMSG pCanMsg);

    /*!
        Receive CANAL message on XML/JSON format

        @param msg std string that will receive CAN message in XML/JSON format, See
        CanalSend for XML/JSON format.
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpReceive(std::string &msg,
                     uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        CanalBlockingReceive

        @param pCanMsg Pointer to CANAL message structure that will get received
        CAN message.
        @param timeout timeout - Timeout in milliseconds. 0 (default) to wait
       forever.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpBlockingReceive(PCANALMSG pCanMsg,
                             uint32_t timeout = CANAL_BLOCK_FOREVER);

    /*!
        CanalBlockingReceive

        @param msg Pointer to CANAL message structure that will get received
        CAN message on XML/JSON format. See CanalSend for XML/JSON format.
        @param timeout timeout - Timeout in milliseconds. 0 (default) to wait
       forever.
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int vscpBlockingReceive(std::string &msg,
                             uint32_t timeout = CANAL_BLOCK_FOREVER,
                             uint8_t nFormat  = CANAL_FORMAT_CAN_XML);

    /*!
        CanalGetVersion

        Get version
        @return Version packed in unsigned int.
    */
    uint32_t vscpGetVersion(void);

    /*!
        Getter for path
    */
    std::string getPath(void) { return m_strPath; };

    /*!
        Getter for parameter
    */
    std::string getParameter(void) { return m_strParameter; };

    /*!
        Getter for flags
    */
    uint32_t getDeviceFlags(void) { return m_deviceFlags; };

    /*!
        Release the driver
    */
    void releaseDriver(void) { if (NULL != m_hdll )  { dlclose(m_hdll); m_hdll = NULL; } };

    /*!
        Return true if the driver is a genaration 1 driver
        @return True of generation 1 driver
    */
    bool isGenerationOne(void) { return m_bGenerationOne; };

  private:
    // Driver DLL/DL path
    std::string m_strPathDriver;

    // Device configuration string
    std::string m_strPathConfig;

    // GUID
    cguid m_guid;

    // True if asynchronous operation
    bool m_bAsync;

    // DLL handle
    void *m_hdll;

    // Handle for dll/dl driver interface
    long m_openHandle;


    // ------------------------------------------------------------------------
    //                     End of driver worker thread data
    // ------------------------------------------------------------------------

    // Level I (CANAL) driver methods
    LPFNDLL_VSCPOPEN m_proc_vscpOpen;
    LPFNDLL_VSCPCLOSE m_proc_vscpClose;
    LPFNDLL_VSCPWRITE m_proc_vscpWrite;
    LPFNDLL_VSCPREAD m_proc_vscpRead;
    LPFNDLL_VSCPGETVERSION m_proc_vscpGetVersion;
    LPFNDLL_VSCPGETVENDORSTRING m_proc_vscpGetVendorString;
};

#endif // include protection