// VscpCanalDeviceIf.cpp : Defines the initialization routines for the DLL.
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

#if !defined(VSCPCANALDEVICEIF_H__INCLUDED_)
#define VSCPCANALDEVICEIF_H__INCLUDED_

#include <canaldlldef.h>

// Format code for object data
#define CANAL_FORMAT_CAN_XML        0 // XML formated CAN message
#define CANAL_FORMAT_CAN_JSON       1 // JSON formated CAN message
#define CANAL_FORMAT_VSCP_XML       2 // XML formated VSCP event
#define CANAL_FORMAT_VSCP_JSON      3 // JSON formatted VSCP event

// USed by send XML parser
typedef struct comboMsg
{
    bool bCan; // True if CAN message
    canalMsg msg;
    vscpEvent ev;
} comboMsg;

class VscpCanalDeviceIf
{

  public:
    /// Constructor
    VscpCanalDeviceIf();

    /// Destructor
    virtual ~VscpCanalDeviceIf();

    // Setters
    void setPath(std::string path) { m_strPath = path; };
    void setParameter(std::string param) { m_strParameter = param; };
    void setFlags(uint32_t flags) { m_deviceFlags = flags; };
    void setbAsync(bool bAsync) { m_bAsync = bAsync; };

    /*!
        Construct CANAL message from XML or JSON format message/event
        @param pmsg Pointer to CANAL message that will get data.
        @param strObj XML or JSON data object
        @param nFormat Format for the data object
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    bool
    constructCanalMsg( canalMsg *pmsg,
                        std::string& strObj,
                        uint8_t nFormat = CANAL_FORMAT_CAN_XML );

    /*!
        Make formated data from CANAL message

        @param str String that will get formatted data
        @param pmsg CANAL message wor work on
        @param nFormat Wanted foremat. 0=XML, 1=JSON
        @return true on success, false on failure.
    */
    bool CanalToFormatedEvent( std::string& str,
                                canalMsg *pmsg,
                                uint8_t nFormat = CANAL_FORMAT_CAN_XML );

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
    int CanalOpen();

    /*!
        CanalClose
    */
    int CanalClose();

    /*!
        Send CAN message

        @param pCanMsg Pointer to CANAL message to send
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSend(const PCANALMSG pCanMsg);

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
    int CanalBlockingSend(const PCANALMSG pCanMsg,
                          uint32_t timeout = CANAL_BLOCK_FOREVER);

    /*!
        Send CAN message, block

        @param msg CAN msg to send on XML/JSON format
        @param timeout timeout - Timeout in milliseconds. 0 (default) to wait
       forever.
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalBlockingSend(std::string& msg,
                          uint32_t timeout = CANAL_BLOCK_FOREVER,
                          uint8_t nFormat  = CANAL_FORMAT_CAN_XML);

    /*!
        Receive CANAL message

        @param pCanMsg Pointer to struct that receive CAN message
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalReceive(PCANALMSG pCanMsg);

    /*!
        Receive CANAL message on XML/JSON format

        @param msg std string that will receive CAN message in XML/JSON format, See
        CanalSend for XML/JSON format.
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalReceive(std::string &msg,
                     uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        CanalBlockingReceive

        @param pCanMsg Pointer to CANAL message structure that will get received
        CAN message.
        @param timeout timeout - Timeout in milliseconds. 0 (default) to wait
       forever.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalBlockingReceive(PCANALMSG pCanMsg,
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
    int CanalBlockingReceive(std::string &msg,
                             uint32_t timeout = CANAL_BLOCK_FOREVER,
                             uint8_t nFormat  = CANAL_FORMAT_CAN_XML);

    /*!
        CanalDataAvailable
        @return number of messages in input queue
    */
    int CanalDataAvailable(void);

    /*!
        Get a structure that gives some information about the state
        of the channel.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalGetStatus(PCANALSTATUS pCanStatus);

    /*!
        CanalGetStatus

        @param objStatus Channel status as XML/JSON object

        {
            "status" : 1234,
            "lasterror" : 1234,
            "lastsuberror" : 1234,
            "lasterrorstr" : "Some error string"
        }
        @param nFormat Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */

    int CanalGetStatus(std::string &objStatus,
                       uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        Get CANAL channel statistics

        @param pCanalStatistics Pointer to CANAL statistic structure
        that will receive the channel statistic data.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalGetStatistics(PCANALSTATISTICS pCanalStatistics);

    /*!
        Get CANAL channel statistics

        @param jsonStatistics Statistics as a XML/JSON object
        {
            "cntReceiveFrames"   : 1234,
            "cntTransmittFrames" : 1234,
            "cntReceiveData"     : 1234,
            "cntTransmittData"   : 1234,
            "cntOverruns"        : 1234,
            "cntBusWarnings"     : 1234,
            "cntBusOff"          : 1234
        }
        @param format Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalGetStatistics(std::string& objStatistics,
                           uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        CanalSetFilter

        @param filter Filter as a 32-bit number
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSetFilter(uint32_t filter);

    /*!
        CanalSetMask

        @param mask Mask as a 32-bit number
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSetMask(uint32_t mask);

    /*!
        Set CANAL filter and mask in one operation with VSCP input data.

        @param vscpfilterJson XML/JSON object with filter data
        {
            "filter" : {
                "priority"  : 123,
                "vscpclass" : 1234,
                "vscptype"  : 1234,
                "nodeid"   : 12
            },
            "mask"   : {
                "priority"  : 123,
                "vscpclass" : 1234,
                "vscptype"  : 1234,
                "nodeid"   : 12
            }
        }
        @param format Format for the data. XML=0 (default), JSON=1
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int VscpSetFilter(std::string &vscpfilterJson,
                      uint8_t nFormat = CANAL_FORMAT_CAN_XML);

    /*!
        Set a new baudrate

        @param baudrate Baudrate as a re.but number with meaning specific to
        device.
        @return CANAL_ERROR_SUCCESS on success, CANAL error code on failure
    */
    int CanalSetBaudrate(uint32_t baudrate);

    /*!
        CanalGetVersion

        Get version
        @return Version packed in unsigned int.
    */
    uint32_t CanalGetVersion(void);

    /*!
        CanalGetDllVersion

        Get DLL version
        @return DLL version packed in unsigned int.
    */
    uint32_t CanalGetDllVersion(void);

    /*!
        CanalGetVendorString

        @return Pointer to zero terminated vendor string (UTF8)
    */
    const char *CanalGetVendorString(void);

    /*!
        Get CANAL driver info on XML format.
        @return Pointer to CANAL driver info on null terminated UTF8 form
        or nULL if no data is available.
    */
    const char *CanalGetDriverInfo(void);

  private:
    // Driver DLL/DL path
    std::string m_strPath;

    // Device configuration string
    std::string m_strParameter;

    // Device flags for CANAL DLL open
    uint32_t m_deviceFlags;

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
    LPFNDLL_CANALOPEN m_proc_CanalOpen;
    LPFNDLL_CANALCLOSE m_proc_CanalClose;
    LPFNDLL_CANALGETLEVEL m_proc_CanalGetLevel;
    LPFNDLL_CANALSEND m_proc_CanalSend;
    LPFNDLL_CANALRECEIVE m_proc_CanalReceive;
    LPFNDLL_CANALDATAAVAILABLE m_proc_CanalDataAvailable;
    LPFNDLL_CANALGETSTATUS m_proc_CanalGetStatus;
    LPFNDLL_CANALGETSTATISTICS m_proc_CanalGetStatistics;
    LPFNDLL_CANALSETFILTER m_proc_CanalSetFilter;
    LPFNDLL_CANALSETMASK m_proc_CanalSetMask;
    LPFNDLL_CANALSETBAUDRATE m_proc_CanalSetBaudrate;
    LPFNDLL_CANALGETVERSION m_proc_CanalGetVersion;
    LPFNDLL_CANALGETDLLVERSION m_proc_CanalGetDllVersion;
    LPFNDLL_CANALGETVENDORSTRING m_proc_CanalGetVendorString;

    // Generation 2
    LPFNDLL_CANALBLOCKINGSEND m_proc_CanalBlockingSend;
    LPFNDLL_CANALBLOCKINGRECEIVE m_proc_CanalBlockingReceive;
    LPFNDLL_CANALGETDRIVERINFO m_proc_CanalGetdriverInfo;
};


#define CANAL_XML_FILTER_TEMPLATE                                              \
    "<filter mask_priority=\"%d\" \n"                                          \
    "mask_class=\"%d\" \n"                                                     \
    "mask_type=\"%d\" \n"                                                      \
    "mask_id=\"%d\" \n"                                                        \
    "filter_priority=\"%d\" \n"                                                \
    "filter_class=\"%d\" \n"                                                   \
    "filter_type=\"%d\" \n"                                                    \
    "filter_id=\"%d\" />"

#define CANAL_JSON_FILTER_TEMPLATE                                             \
    "{\n"                                                                      \
    "\"mask_priority\": %d,\n"                                                 \
    "\"mask_class\": %d,\n"                                                    \
    "\"mask_type\": %d,\n"                                                     \
    "\"mask_id\": %d,\n"                                                       \
    "\"filter_priority\": %d,\n"                                               \
    "\"filter_class\": %d,\n"                                                  \
    "\"filter_type\": %d,\n"                                                   \
    "\"filter_id\": %d,\n"                                                     \
    "}"

#endif // include protection