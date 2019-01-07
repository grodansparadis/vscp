///////////////////////////////////////////////////////////////////////////////
// CanalSuperWrapper.h: interface for the CCanalSuperWrapper class
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
//
//

/*!
\file canalsuperwrapper.h
\brief Wrapping class for VSCP interfaces.
\details This class ecapsulates both the CANAL dll and the VSCP TCP/IP interface
and makes it possible to use the same code to read/write both CANAL messages and
VSCP events. \author Ake Hedman <akhe@grodansparadis.com>, Grodans Paradis AB,
Sweden
*/

#include "devitem.h"
#include "dllwrapper.h"
#include "guid.h"
#include "mdf.h"
#include "vscpremotetcpif.h"
#include <dllist.h>
#include <string.h>

#if !defined(CANALSUPERWRAPPER_H__INCLUDED_)
#define CANALSUPERWRAPPER_H__INCLUDED_

#include "canal.h"
#include "devitem.h"

#if wxUSE_GUI
#include <wx/progdlg.h>
#endif

// Default values for read/write register functions
// used in device config and scan.
#define SW_REGISTER_READ_RESEND_TIMEOUT 5000
#define SW_REGISTER_READ_ERROR_TIMEOUT 4000
#define SW_REGISTER_READ_MAX_TRIES 3

/*!@{
Constants for possible interfaces
@}*/
/// Cnstant that defines the CANAL interface
#define USE_DLL_INTERFACE 0
/// Constat that defines the TCP/IP interface
#define USE_TCPIP_INTERFACE 1

/// Maximum path length (if nit defined by the OS if any.
#ifndef MAX_PATH
#define MAX_PATH 255
#endif

/*!
\class CCanalSuperWrapper
\brief Encapsulates the CANAL interface and the VSCP tcp/ip interface
*/
class CCanalSuperWrapper
{

  public:
    /*!
    Default Constructor
    Use one of the SetInterface methods to specify
    device/host data.
    */
    CCanalSuperWrapper(void);

    /*!
    Set Interface CANAL
    Use this method if the default constructor is used to construct the object
    pItem->name is the user name for the driver.
    pItem->path is the location of the driver on the disc. Path can be set to
    "TCPIP" in which case the config string holds connect information.
    pItem->config is the configuration string for the driver. If path = "TCPIP"
    the this string is searched for host connect information on the form
    "username;password;host;port"
    pItem->flags is the configuration flags for the driver. Not used for TCP/IP
    interface.
    */
    CCanalSuperWrapper(devItem *pItem);

    /// Destructor
    virtual ~CCanalSuperWrapper();

    /*!
    Init common paramaters
    */
    void init(void);

    /*!
    Set Interface TCP/IP
    Use this method if the default constructor is used to construct the object
    @param host to connect to (defaults to localhost if not given).
    @param username Username to login to service.
    @param password to login to service.
    */
    void setInterface(const std::string &host,
                      const std::string &username,
                      const std::string &password);

    /*!
    Set Interface CANAL
    Use this method if the default constructor is used to construct the object
    @param name is the user name for the driver.
    @param path is the location of the driver on the disc.
    @param parameters is the configuration string for the driver.
    @param flags is the configuration flags for the driver.
    @param filter Filer to set.
    @param mask Mask to set.
    */
    void setInterface(const std::string &name,
                      const std::string &path,
                      const std::string &parameters,
                      const unsigned long flags,
                      const unsigned long filter,
                      const unsigned long mask);

    /*!
    Open communication channel.

    Data can be supplied to override the pdev structure information
    that was given on construction.

    @param strInterface is name of channel.
    @param flags Interface flags
    @return true if channel is open or false if error or the channel is
    already opened.
    */
    long doCmdOpen(const std::string &strInterface = (_T("")),
                   unsigned long flags             = 0L);

    /*!
    Close communication channel

    @return true if the close was successful
    */
    int doCmdClose(void);

    /*!
    Do a no operation test command.

    @return true if success false if not.
    */
    int doCmdNOOP(void);

    /*!
    Clear input queue.

    @return true if success false if not.
    */
    int doCmdClear(void);

    /*!
    Get the CANAL protocol level

    @return true on success
    */
    unsigned long doCmdGetLevel(void);

    /*!
    Send a CANAL message.

    @return true if success false if not.
    */
    int doCmdSend(canalMsg *pMsg);

    /*!
    Send a VSCP event.

    @return true if success false if not.
    */
    int doCmdSend(const vscpEvent *pEvent);

    /*!
    Send a VSCP Ex event.

    @return true if success false if not.
    */
    int doCmdSend(const vscpEventEx *pEventEx);

    /*!
    Receive a CANAL message.

    @return true if success false if not.
    */
    int doCmdReceive(canalMsg *pMsg);

    /*!
    Receive a VSCP event.

    @return true if success false if not.
    */
    int doCmdReceive(vscpEvent *pEvent);

    /*!
    Receive a VSCP Ex event.

    @return true if success false if not.
    */
    int doCmdReceive(vscpEventEx *pEventEx);

    /*!
    Get the number of messages in the input queue

    @return the number of messages available or if negative
    an error code.
    */
    int doCmdDataAvailable(void);

    /*!
    Receive CANAL status.

    @return true if success false if not.
    */
    int doCmdStatus(canalStatus *pStatus);

    /*!
    Receive CANAL statistics through the pipe.

    @return true if success false if not.
    */
    int doCmdStatistics(canalStatistics *pStatistics);

    /*!
    Set/Reset a filter through the pipe.

    @return true if success false if not.
    */
    int doCmdFilter(unsigned long filter);

    /*!
    Set/Reset a mask through the pipe.

    @return true if success false if not.
    */
    int doCmdMask(unsigned long mask);

    /*!
    Set/Reset the VSCP filter for a channel.

    @return true if success false if not.
    */
    int doCmdVscpFilter(const vscpEventFilter *pFilter);

    /*!
    Set baudrate.

    @return true if success false if not.
    */
    int doCmdBaudrate(unsigned long baudrate);

    /*!
    Get i/f version.

    @return true if success false if not.
    */
    unsigned long doCmdVersion(void);

    /*!
    Get dll version.

    @return true if success false if not.
    */
    unsigned long doCmdDLLVersion(void);

    /*!
    Get vendorstring.

    @return Pointer to vendor string.
    */
    const char *doCmdVendorString(void);

    /*!
    Get device information string.

    @return pointer to driver information string.
    */
    const char *doCmdGetDriverInfo(void);

    /*!
    Get the type of interface that is active.
    @return USE_DLL_INTERFACE if direct DLL interface active.
    @return USE_TCPIP_INTERFACE	if TCP/IP interface active.
    */
    int getDeviceType(void) { return m_itemDevice.id; };

    /*!
    Check if interface is open
    @return Return true if if open, false if closed.
    */
    bool isOpen(void) { return (m_devid ? true : false); };

    /*!
    Shutdown the VSCP daemon

    @return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC on falure.
    */
    int doCmdShutDown(void);

    /*!
    Get a pointer to the TCP/IP interface
    @return A pointer to the TCP/IP interface
    */
    VscpRemoteTcpIf *getTcpIpInterface(void) { return &m_vscptcpif; };

    /*!
        Get a pointer to the DLL interface
        @return A pointer to the DLL interface
    */
    CDllWrapper *getDllInterface(void) { return &m_canalDll; };

    /*!
    Get variable value from string variable
    \param name of variable
    \param strValue pointer to string that get the value of the string variable.
    \return true if the variable is of type string.
    */
    int getVariableString(std::string &name, std::string &strValue)
    {
        return m_vscptcpif.getRemoteVariableAsString(name, strValue);
    };

    /*!
    Set variable value from string variable
    \param name of variable
    \param strValue to string that get the value of the string variable.
    \return true if the variable is of type string.
    */
    int setVariableString(std::string &name, std::string &strValue)
    {
        return m_vscptcpif.getRemoteVariableValue(name, strValue);
    };

    /*!
    Get variable value from boolean variable
    \param name of variable
    \param bValue pointer to boolean variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int getVariableBool(std::string &name, bool *bValue)
    {
        return m_vscptcpif.getRemoteVariableBool(name, bValue);
    };

    /*!
    Set variable value from boolean variable
    \param name of variable
    \param bValue boolean variable that get the value of the string variable.
    \return true if the variable is of type string.
    */
    int setVariableBool(std::string &name, bool bValue)
    {
        return m_vscptcpif.setRemoteVariableBool(name, bValue);
    };

    /*!
    Get variable value from integer variable
    \param name of variable
    \param value pointer to integer variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int getVariableInt(std::string &name, int *value)
    {
        return m_vscptcpif.getRemoteVariableInt(name, value);
    };

    /*!
    set variable value from integer variable
    \param name of variable
    \param value integer variable that get the value of the string variable.
    \return true if the variable is of type string.
    */
    int setVariableInt(std::string &name, int value)
    {
        return m_vscptcpif.setRemoteVariableInt(name, value);
    };

    /*!
    Get variable value from long variable
    \param name of variable
    \param value pointer to long variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int getVariableLong(std::string &name, long *value)
    {
        return m_vscptcpif.getRemoteVariableLong(name, value);
    };

    /*!
    Set variable value from long variable
    \param name of variable
    \param value long variable that get the value of the string variable.
    \return true if the variable is of type string.
    */
    int setVariableLong(std::string &name, long value)
    {
        return m_vscptcpif.setRemoteVariableLong(name, value);
    };

    /*!
    Get variable value from double variable
    \param name of variable
    \param value pointer to double variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int getVariableDouble(std::string &name, double *value)
    {
        return m_vscptcpif.getRemoteVariableDouble(name, value);
    };

    /*!
    Set variable value from double variable
    \param name of variable
    \param value pointer to double variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int setVariableDouble(std::string &name, double value)
    {
        return m_vscptcpif.setRemoteVariableDouble(name, value);
    };

    /*!
    Get variable value from measurement variable
    \param name of variable
    \param strValue String that get that get the
    value of the measurement.
    \return true if the variable is of type string.
    */
    int getVariableMeasurement(std::string &name,
                               std::string &strValue) // TODO Fix
    {
        return m_vscptcpif.getRemoteVariableValue(name, strValue);
    };

    /*!
    Set variable value from measurement variable
    \param name of variable
    \param strValue String that get that get the
    value of the measurement.
    \return true if the variable is of type string.
    */
    int setVariableMeasurement(std::string &name,
                               std::string &strValue) // TODO Fix
    {
        return m_vscptcpif.setRemoteVariableValue(name, strValue);
    };

    /*!
    Get variable value from event variable
    \param name of variable
    \param pEvent pointer to event variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int getVariableEvent(std::string &name, vscpEvent *pEvent)
    {
        return m_vscptcpif.getRemoteVariableEvent(name, pEvent);
    };

    /*!
    Set variable value from event variable
    \param name of variable
    \param pEvent pointer to event variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int setVariableEvent(std::string &name, vscpEvent *pEvent)
    {
        return m_vscptcpif.setRemoteVariableEvent(name, pEvent);
    };

    /*!
    Get variable value from event variable
    \param name of variable
    \param pEvent pointer to event variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int getVariableEventEx(std::string &name, vscpEventEx *pEvent)
    {
        return m_vscptcpif.getRemoteVariableEventEx(name, pEvent);
    };

    /*!
    Set variable value from event variable
    \param name of variable
    \param pEvent pointer to event variable that get the value of the string
    variable. \return true if the variable is of type string.
    */
    int setVariableEventEx(std::string &name, vscpEventEx *pEvent)
    {
        return m_vscptcpif.setRemoteVariableEventEx(name, pEvent);
    };

    /*!
    Get variable value from GUID variable
    \param name of variable
    \param GUID variable that get the value of the GUID variable.
    \return true if the variable is of type string.
    */
    int getVariableGUID(std::string &name, cguid &GUID)
    {
        return m_vscptcpif.getRemoteVariableGUID(name, GUID);
    };

    /*!
    Set variable value from GUID variable
    \param name of variable
    \param GUID variable that get the value of the GUID variable.
    \return true if the variable is of type string.
    */
    int setVariableGUID(std::string &name, cguid &GUID)
    {
        return m_vscptcpif.setRemoteVariableGUID(name, GUID);
    };

    /*!
    Get variable value from VSCP data variable
    \param name of variable
    \param psizeData Pointer to variable that will hold the size of the data
    array \param pData pointer to VSCP data array variable (unsigned char [8] )
    that get the value of the string variable. \return true if the variable is
    of type string.
    */
    int getVariableVSCPdata(std::string &name,
                            uint8_t *pData,
                            uint16_t *psizeData)
    {
        return m_vscptcpif.getRemoteVariableVSCPdata(name, pData, psizeData);
    };

    /*!
    Set variable value from VSCP data variable
    \param name of variable
    \param sizeData to variable that will hold the size of the data array
    \param pData pointer to VSCP data array variable (unsigned char [8] ) that
    get the value of the string variable. \return true if the variable is of
    type string.
    */
    int setVariableVSCPdata(std::string &name,
                            uint16_t sizeData,
                            uint8_t *pData)
    {
        return m_vscptcpif.setRemoteVariableVSCPdata(name, pData, sizeData);
    };

    /*!
    Get variable value from class variable
    \param name of variable
    \param vscp_class pointer to int that get the value of the class variable.
    \return true if the variable is of type string.
    */
    int getVariableVSCPclass(std::string &name, uint16_t *vscp_class)
    {
        return m_vscptcpif.getRemoteVariableVSCPclass(name, vscp_class);
    };

    /*!
    Set variable value from class variable
    \param name of variable
    \param vscp_class int that get the value of the class variable.
    \return true if the variable is of type string.
    */
    int setVariableVSCPclass(std::string &name, uint16_t vscp_class)
    {
        return m_vscptcpif.setRemoteVariableVSCPclass(name, vscp_class);
    };

    /*!
    Get variable value from type variable
    \param name of variable
    \param vscp_type pointer to int that get the value of the type variable.
    \return true if the variable is of type string.
    */
    int getVariableVSCPtype(std::string &name, uint16_t *vscp_type)
    {
        return m_vscptcpif.getRemoteVariableVSCPtype(name, vscp_type);
    };

    /*!
    Set variable value from type variable
    \param name of variable
    \param vscp_type to int that get the value of the type variable.
    \return true if the variable is of type string.
    */
    int setVariableVSCPtype(std::string &name, uint16_t vscp_type)
    {
        return m_vscptcpif.setRemoteVariableVSCPtype(name, vscp_type);
    };

    /*!
    Set read timeout
    @param to Read timeout in milliseconds
    */
    void setReadTimeout(uint32_t to) { m_registerReadErrorTimeout = to; };

    /*!
    Set read resend timeout
    @param to Read resend timeout in milliseconds
    */
    void setReadResendTimeout(uint32_t to)
    {
        m_registerReadResendTimeout = to;
    };

    /*!
    Set max read retries
    @param n Number of retries before giving up
    */
    void setMaxRetries(uint8_t n) { m_registerReadMaxRetries = n; };

// We don't want the graphcal UI on apps that don't use it
#if (wxUSE_GUI != 0)

    /*!
    Read level I register
    @param nodeid Nickname for node to read register on
    @param reg Register to read.
    @param pval Pointer to value read
    @return True on success false on failure.
    */
    bool _readLevel1Register(uint8_t nodeid,
                             uint8_t reg,
                             uint8_t *pval,
                             wxProgressDialog *pdlg = NULL);

    /*!
    Write level I register
    @param nodeid Nickname for node to write register on
    @param reg Register to write to.
    @param val Register value to write
    @return True on success false on failure.
    */
    bool _writeLevel1Register(uint8_t nodeid,
                              uint8_t reg,
                              uint8_t *pval,
                              wxProgressDialog *pdlg = NULL);

    /*!
    Read a level 2 register
    @param interfaceGUID GUID for interface where devices sits whos register
    should be read with byte 0 set to nickname id for the device.
    @param reg Register to read.
    @param pcontent Pointer to read value.
    @return True on success. False otherwise.
    */
    bool readLevel2Register(cguid &ifGUID,
                            uint32_t reg,
                            uint8_t *pcontent,
                            cguid *pdestGUID       = NULL,
                            wxProgressDialog *pdlg = NULL,
                            bool bLevel2           = false);

    /*!
    Write a level 2 register

    @param interfaceGUID GUID for interface where devices sits whos register
    should be read with byte 0 set to nickname id for the device.
    @param reg Register to write.
    @param pcontent Pointer to data to write. Return read data.
    @return True on success. False otherwise.
    */
    bool _writeLevel2Register(cguid &ifGUID,
                              uint32_t reg,
                              uint8_t *pcontent,
                              cguid &destGUID,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false);

    /*!
    Load level I register content into an array
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param pregisters Pointer to an array of 256 8-bit registers.
    @param nodeid nodeid The node whos registers should be read.
    @param bQuite No progress information if sett to true. (default is false)
    @param startreg First register to read. Default is 0.
    @param count Number of registers to read. Default is 256.
    @return true on success, false on failure.
    */

    bool readLevel1Registers(wxWindow *pwnd,
                             uint8_t *pregisters,
                             uint8_t nodeid,
                             uint8_t startreg       = 0,
                             uint16_t count         = 256,
                             wxProgressDialog *pdlg = NULL);

    /*!
    Load level II register content into an array
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param pregisters Pointer to an array of 256 8-bit registers.
    @param pinterfaceGUID GUID for interface to do read on.
    @param pdestGUID GUID for remote node.
    @param startreg First register to read. Default is 0.
    @param count Number of registers to read. Default is 256.
    @return true on success, false on failure.
    */

    bool readLevel2Registers(wxWindow *pwnd,
                             uint8_t *pregisters,
                             cguid &ifGUID,
                             uint32_t startreg      = 0,
                             uint32_t count         = 256,
                             cguid *pdestGUID       = NULL,
                             wxProgressDialog *pdlg = NULL,
                             bool bLevel2           = false);

    /*!
    Get MDf file from device registers

    @param pid Pointer to id. Either a one byte nickname if bLevel = false
    or a 16 byte GUID if bLevel2 = true.
    @return true on success, false on failure.
    */
    std::string getMDFfromLevel1Device(uint8_t id, bool bSilent = false);

    /*!
    Get MDf file from device registers

    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return true on success, false on failure.
    */
    std::string getMDFfromLevel2Device(wxProgressDialog &progressDlg,
                                       cguid &ifGUID,
                                       cguid &destGUID,
                                       bool bLevel2 = false,
                                       bool bSilent = false);

    /*!
     Get MDF from server or device
     @param pwnd Pointer to a window for GUI. If a windows pointer
                is supplied a progress dialog will be shown.
     @param preg_url Pointer to a register array of 33 bytes that
                        is a copy of the registers for the MDF url in
                        the device terminated with zeros. If a NULL pointer
                        is used here the URL to the MDF file must be supplied
                        in string form.
     @oaram url	This is an alternative way to give the URL to the MDF file and
                    is used if preg_url is a NULL pointer. The form should be
                    http://www.grodansparadis.com/smart2_001.mdf
     */
    bool loadMDF(wxWindow *pwnd,
                 uint8_t *preg_url,
                 std::string &url,
                 CMDF *pmdf);
    /*!
    Get Decision Matrix info for a Level I Node

    @param nodeid id for node whos info should be fetched.
    @param pdata Pointer to returned data. Array of eight bytes.
    @return true on success, false on failure.
    */
    bool getLevel1DmInfo(const uint8_t nodeid, uint8_t *pdata);

    /*!
    Get Decision Matrix info for a Level II Node

    @param interfaceGUID GUID + nodeid for node whos info should be
    fetched.
    @param pdata Pointer to returned data. Array of eight bytes.
    @return true on success, false on failure.
    */
    bool getLevel2DmInfo(cguid &ifGUID, uint8_t *pdata, bool bLevel2 = false);

    /*!
    Set register page for level 1 node
    @param nodeid Nickname for node to set register page for
    @param page Page to set.
    @Param Pointer to interface GUID if the interface should be set
    over the daemon interface.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @return True on success, false on failure.
    */

    bool setRegisterPage(uint8_t nodeid,
                         uint16_t page,
                         cguid *pifGUID   = NULL,
                         cguid *pdestGUID = NULL,
                         bool bLevel2     = false);

    /*!
    Get current register page
    @param nodeid Nickname for node to set register page for
    @Param Pointer to interface GUID if the interface should be set
    over the daemon interface.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @return Current register page.
    */
    uint32_t getRegisterPage(wxWindow *pwnd,
                             uint8_t nodeid,
                             cguid *pifGUID   = NULL,
                             cguid *pdestGUID = NULL,
                             bool bLevel2     = false);

    /*!
    Get a decision matrix row

    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param pdm Pointer to MDF decsion matrix info.
    @param row Row to fetch.
    @param pRow Pointer t array which must hold eight bytes and
    will receive the row.
    @param bSilent Set to true to not show error messages.
    */
    bool getDMRow(wxWindow *pwnd,
                  uint8_t nodeid,
                  CMDF_DecisionMatrix *pdm,
                  uint32_t row,
                  uint8_t *pRow,
                  cguid *pifGUID   = NULL,
                  cguid *pdestGUID = NULL,
                  bool bLevel2     = false,
                  bool bSilent     = false);

    //          * * * * Abstraction handlers * * * *

    /*!
    Read abstraction string
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @retstr String that will reveive abstraction string.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstractionString(wxWindow *pwnd,
                              uint8_t nodeid,
                              CMDF_Abstraction *abstraction,
                              std::string &retstr,
                              cguid *pifGUID         = NULL,
                              cguid *pdestGUID       = NULL,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false,
                              bool bSilent           = false);

    /*!
    Write abstraction string
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @retstr Abstraction string that will get written.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstractionString(wxWindow *pwnd,
                                uint8_t nodeid,
                                CMDF_Abstraction *abstraction,
                                std::string &strvalue,
                                cguid *pifGUID         = NULL,
                                cguid *pdestGUID       = NULL,
                                wxProgressDialog *pdlg = NULL,
                                bool bLevel2           = false,
                                bool bSilent           = false);

    /*!
    Read abstraction bitfield
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @retstr String that will reveive abstraction string.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstractionBitField(wxWindow *pwnd,
                                uint8_t nodeid,
                                CMDF_Abstraction *abstraction,
                                std::string &strBitField,
                                cguid *pifGUID         = NULL,
                                cguid *pdestGUID       = NULL,
                                wxProgressDialog *pdlg = NULL,
                                bool bLevel2           = false,
                                bool bSilent           = false);

    /*!
    Write abstraction bitfield
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @retstr Abstraction string that will get written.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstractionBitField(wxWindow *pwnd,
                                  uint8_t nodeid,
                                  CMDF_Abstraction *abstraction,
                                  std::string &strBitField,
                                  cguid *pifGUID         = NULL,
                                  cguid *pdestGUID       = NULL,
                                  wxProgressDialog *pdlg = NULL,
                                  bool bLevel2           = false,
                                  bool bSilent           = false);
    /*!
    Read abstraction bool
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to boolean that will reveive abstraction boolean.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstractionBool(wxWindow *pwnd,
                            uint8_t nodeid,
                            CMDF_Abstraction *abstraction,
                            bool *bval,
                            cguid *pifGUID         = NULL,
                            cguid *pdestGUID       = NULL,
                            wxProgressDialog *pdlg = NULL,
                            bool bLevel2           = false,
                            bool bSilent           = false);

    /*!
    Write abstraction bool
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to boolean that will be written as abstraction boolean.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstractionBool(wxWindow *pwnd,
                              uint8_t nodeid,
                              CMDF_Abstraction *abstraction,
                              bool &bval,
                              cguid *pifGUID         = NULL,
                              cguid *pdestGUID       = NULL,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false,
                              bool bSilent           = false);

    /*!
    Read abstraction 8-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to 8-bit integer.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstraction8bitinteger(wxWindow *pwnd,
                                   uint8_t nodeid,
                                   CMDF_Abstraction *abstraction,
                                   uint8_t *pval,
                                   cguid *pifGUID         = NULL,
                                   cguid *pdestGUID       = NULL,
                                   wxProgressDialog *pdlg = NULL,
                                   bool bLevel2           = false,
                                   bool bSilent           = false);

    /*!
    Write abstraction 8-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval 8-bit integer to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstraction8bitinteger(wxWindow *pwnd,
                                     uint8_t nodeid,
                                     CMDF_Abstraction *abstraction,
                                     uint8_t &val,
                                     cguid *pifGUID         = NULL,
                                     cguid *pdestGUID       = NULL,
                                     wxProgressDialog *pdlg = NULL,
                                     bool bLevel2           = false,
                                     bool bSilent           = false);

    /*!
    Read abstraction 16-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to 16-bit iteger.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstraction16bitinteger(wxWindow *pwnd,
                                    uint8_t nodeid,
                                    CMDF_Abstraction *abstraction,
                                    uint16_t *pval,
                                    cguid *pifGUID         = NULL,
                                    cguid *pdestGUID       = NULL,
                                    wxProgressDialog *pdlg = NULL,
                                    bool bLevel2           = false,
                                    bool bSilent           = false);

    /*!
    Write abstraction 16-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param val16 16-bit integer to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstraction16bitinteger(wxWindow *pwnd,
                                      uint8_t nodeid,
                                      CMDF_Abstraction *abstraction,
                                      uint16_t &val16,
                                      cguid *pifGUID         = NULL,
                                      cguid *pdestGUID       = NULL,
                                      wxProgressDialog *pdlg = NULL,
                                      bool bLevel2           = false,
                                      bool bSilent           = false);

    /*!
    Read abstraction 32-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to 32-bit integer.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstraction32bitinteger(wxWindow *pwnd,
                                    uint8_t nodeid,
                                    CMDF_Abstraction *abstraction,
                                    uint32_t *pval,
                                    cguid *pifGUID         = NULL,
                                    cguid *pdestGUID       = NULL,
                                    wxProgressDialog *pdlg = NULL,
                                    bool bLevel2           = false,
                                    bool bSilent           = false);

    /*!
    Write abstraction 32-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param val32 32-bit integer to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstraction32bitinteger(wxWindow *pwnd,
                                      uint8_t nodeid,
                                      CMDF_Abstraction *abstraction,
                                      uint32_t &val32,
                                      cguid *pifGUID         = NULL,
                                      cguid *pdestGUID       = NULL,
                                      wxProgressDialog *pdlg = NULL,
                                      bool bLevel2           = false,
                                      bool bSilent           = false);

    /*!
    Read abstraction 64-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to 64-bit integer.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstraction64bitinteger(wxWindow *pwnd,
                                    uint8_t nodeid,
                                    CMDF_Abstraction *abstraction,
                                    uint64_t *pval,
                                    cguid *pifGUID         = NULL,
                                    cguid *pdestGUID       = NULL,
                                    wxProgressDialog *pdlg = NULL,
                                    bool bLevel2           = false,
                                    bool bSilent           = false);

    /*!
    Write abstraction 64-bit integer
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param val64 64-bit integer to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstraction64bitinteger(wxWindow *pwnd,
                                      uint8_t nodeid,
                                      CMDF_Abstraction *abstraction,
                                      uint64_t &val64,
                                      cguid *pifGUID         = NULL,
                                      cguid *pdestGUID       = NULL,
                                      wxProgressDialog *pdlg = NULL,
                                      bool bLevel2           = false,
                                      bool bSilent           = false);

    /*!
    Read abstraction float 32-bit ( IEEE standard 754 1985 )
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to 64-bit IEEE standard 754 floating point.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    http://en.wikipedia.org/wiki/IEEE_754-1985
    */
    bool getAbstractionFloat(wxWindow *pwnd,
                             uint8_t nodeid,
                             CMDF_Abstraction *abstraction,
                             float *pval,
                             cguid *pifGUID         = NULL,
                             cguid *pdestGUID       = NULL,
                             wxProgressDialog *pdlg = NULL,
                             bool bLevel2           = false,
                             bool bSilent           = false);

    /*!
    Write abstraction float 32-bit ( IEEE standard 754 1985 )
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param valfloat 64-bit IEEE standard 754 floating point value to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    http://en.wikipedia.org/wiki/IEEE_754-1985
    */
    bool writeAbstractionFloat(wxWindow *pwnd,
                               uint8_t nodeid,
                               CMDF_Abstraction *abstraction,
                               float &valfloat,
                               cguid *pifGUID         = NULL,
                               cguid *pdestGUID       = NULL,
                               wxProgressDialog *pdlg = NULL,
                               bool bLevel2           = false,
                               bool bSilent           = false);

    /*!
    Read abstraction double 64-bit ( IEEE standard 754)
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to 64-bit IEEE standard 754 floating point.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    http://docs.wxwidgets.org/trunk/group__group__funcmacro__math.html
    */
    bool getAbstractionDouble(wxWindow *pwnd,
                              uint8_t nodeid,
                              CMDF_Abstraction *abstraction,
                              double *pval,
                              cguid *pifGUID         = NULL,
                              cguid *pdestGUID       = NULL,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false,
                              bool bSilent           = false);

    /*!
    Write abstraction double 64-bit ( IEEE standard 754)
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param valdouble 64-bit IEEE standard 754 floating point to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    http://docs.wxwidgets.org/trunk/group__group__funcmacro__math.html
    */
    bool writetAbstractionDouble(wxWindow *pwnd,
                                 uint8_t nodeid,
                                 CMDF_Abstraction *abstraction,
                                 double &valdouble,
                                 cguid *pifGUID         = NULL,
                                 cguid *pdestGUID       = NULL,
                                 wxProgressDialog *pdlg = NULL,
                                 bool bLevel2           = false,
                                 bool bSilent           = false);

    /*!
    Read abstraction date
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pinter to Date
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure

    Dates are stored as YYYY-MM-DD
    byte 0 - MSB of year
    byte 1 - LSB of year
    byte 2 - Month (1-12)
    byte 3 - Date (0-31)
    */
    bool getAbstractionDate(wxWindow *pwnd,
                            uint8_t nodeid,
                            CMDF_Abstraction *abstraction,
                            wxDateTime *pval,
                            cguid *pifGUID         = NULL,
                            cguid *pdestGUID       = NULL,
                            wxProgressDialog *pdlg = NULL,
                            bool bLevel2           = false,
                            bool bSilent           = false);

    /*!
    Write abstraction date
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param valdate Date to write
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure

    Dates are stored as YYYY-MM-DD
    byte 0 - MSB of year
    byte 1 - LSB of year
    byte 2 - Month (1-12)
    byte 3 - Date (0-31)
    */
    bool writeAbstractionDate(wxWindow *pwnd,
                              uint8_t nodeid,
                              CMDF_Abstraction *abstraction,
                              wxDateTime &valdate,
                              cguid *pifGUID         = NULL,
                              cguid *pdestGUID       = NULL,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false,
                              bool bSilent           = false);

    /*!
    Read abstraction time
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to three byte array for time hhmmss.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstractionTime(wxWindow *pwnd,
                            uint8_t nodeid,
                            CMDF_Abstraction *abstraction,
                            wxDateTime *pval,
                            cguid *pifGUID         = NULL,
                            cguid *pdestGUID       = NULL,
                            wxProgressDialog *pdlg = NULL,
                            bool bLevel2           = false,
                            bool bSilent           = false);

    /*!
    Write abstraction time
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param valtime Time to write
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstractionTime(wxWindow *pwnd,
                              uint8_t nodeid,
                              CMDF_Abstraction *abstraction,
                              wxDateTime &valtime,
                              cguid *pifGUID         = NULL,
                              cguid *pdestGUID       = NULL,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false,
                              bool bSilent           = false);

    /*!
    Read abstraction GUID
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param bval Pointer to GUID class.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool getAbstractionGUID(wxWindow *pwnd,
                            uint8_t nodeid,
                            CMDF_Abstraction *abstraction,
                            cguid *pval,
                            cguid *pifGUID         = NULL,
                            cguid *pdestGUID       = NULL,
                            wxProgressDialog *pdlg = NULL,
                            bool bLevel2           = false,
                            bool bSilent           = false);

    /*!
    write abstraction GUID
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param valguid GUID to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return True on success, false on failure
    */
    bool writeAbstractionGUID(wxWindow *pwnd,
                              uint8_t nodeid,
                              CMDF_Abstraction *abstraction,
                              cguid &valguid,
                              cguid *pifGUID         = NULL,
                              cguid *pdestGUID       = NULL,
                              wxProgressDialog *pdlg = NULL,
                              bool bLevel2           = false,
                              bool bSilent           = false);

    /*!
    Get string representation of abstraction value
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param nodeid nodeid The node whos registers should be read.
    @param abstraction Pointer to MDF abstraction info
    @param valguid GUID to write.
    @param interfaceGUID Interface to work on.
    @param pdestGUID Pointer to guid of node.
    @param bLevel2 Set to true if this is a level II devive
    @param bSilent Set to true to not show error messages.
    @return String representation
    */
    std::string getAbstractionValueAsString(wxWindow *pwnd,
                                            uint8_t nodeid,
                                            CMDF_Abstraction *abstraction,
                                            cguid *pifGUID         = NULL,
                                            cguid *pdestGUID       = NULL,
                                            wxProgressDialog *pdlg = NULL,
                                            bool bLevel2           = false,
                                            bool bSilent           = false);

#endif

  protected:
    /*!
    ID for open device
    */
    long m_devid;

    /*!
    Timer for canal interaction
    */
    unsigned int m_uTimerID;

    /*!
    Data for the selected device
    id holds type of i/f (DLL, TCP/IP
    */
    devItem m_itemDevice;

    /*!
        TCP/IP interface
    */
    VscpRemoteTcpIf m_vscptcpif;

    /*!
    GUID
    */
    cguid m_GUID;

    /*!
    The CANAL dll wrapper
    */
    CDllWrapper m_canalDll;

    /*!
    Flag for open device
    */
    bool m_bOpen;

    /*!
    Register read timeout in milliseconds
    */
    uint32_t m_registerReadErrorTimeout;

    /*!
    Register read resend timout.
    */
    uint32_t m_registerReadResendTimeout;

    /*!
    Number of retries before giving up
    */
    uint8_t m_registerReadMaxRetries;
};

#endif // !defined(CANALSUPERWRAPPER_H__INCLUDED_)
