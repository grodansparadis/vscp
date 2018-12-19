/////////////////////////////////////////////////////////////////////////////
// Name:        vscpworks.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 28 Jun 2007 10:05:16 CEST
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// 
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
// 
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
// 
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
// 
//  Alternative licenses for VSCP & Friends may be arranged by contacting 
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
// 

#ifndef _VSCPWORKS_H_
#define _VSCPWORKS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "vscpworks.h"
#endif

/*!
 * Includes
 */

#include "wx/image.h"
#include "frmmain.h"

#include <vscp.h>
#include <canalsuperwrapper.h>
#include <mdf.h>

/*!
 * Forward declarations
 */


/*!
 * Control identifiers
 */

#define VSCPWORKS_CONFIG_FILE_NAME   _("vscpworks.conf")

#define VSCPWORKS_MAIN_WINDOW_DEFAULT_WIDTH                 700
#define VSCPWORKS_MAIN_WINDOW_DEFAULT_HEGHT                 400

#define MAX_NUMBER_OF_NODES                                 256

#define VSCPWORKS_LOGMSG_DEBUG                              0
#define VSCPWORKS_LOGMSG_INFO                               1
#define VSCPWORKS_LOGMSG_NOTICE                             2
#define VSCPWORKS_LOGMSG_WARNING                            3
#define VSCPWORKS_LOGMSG_ERROR                              4
#define VSCPWORKS_LOGMSG_CRITICAL                           5
#define VSCPWORKS_LOGMSG_ALERT                              6
#define VSCPWORKS_LOGMSG_EMERGENCY                          7

#define VCSP_TRMIT_FIELD_COUNT                              6

#define VCSP_TRMIT_FIELD_WIDTH_0                            30
#define VCSP_TRMIT_FIELD_WIDTH_1                            200
#define VCSP_TRMIT_FIELD_WIDTH_2                            60
#define VCSP_TRMIT_FIELD_WIDTH_3                            60
#define VCSP_TRMIT_FIELD_WIDTH_4                            120
#define VCSP_TRMIT_FIELD_WIDTH_5                            900

#define VCSP_TRMIT_FIELD_TEXT_0                             _("x")
#define VCSP_TRMIT_FIELD_TEXT_1                             _("Name")
#define VCSP_TRMIT_FIELD_TEXT_2                             _("Period")
#define VCSP_TRMIT_FIELD_TEXT_3                             _("Count")
#define VCSP_TRMIT_FIELD_TEXT_4                             _("Trigger")
#define VCSP_TRMIT_FIELD_TEXT_5                             _("Event")

#define VCSP_RSCV_FIELD_COUNT                               4

#ifdef WIN32
#define VCSP_RSCV_FIELD_DEFAULT_HEIGHT                      20
#else   
#define VCSP_RSCV_FIELD_DEFAULT_HEIGHT                      20
#endif

#define VCSP_RSCV_FIELD_WIDTH_0                             30
#define VCSP_RSCV_FIELD_WIDTH_1                             170
#define VCSP_RSCV_FIELD_WIDTH_2                             170
#define VCSP_RSCV_FIELD_WIDTH_3                             700

#define VCSP_RSCV_FIELD_TEXT_0                              _("Dir")
#define VCSP_RSCV_FIELD_TEXT_1                              _("Class")
#define VCSP_RSCV_FIELD_TEXT_2                              _("Type")
#define VCSP_RSCV_FIELD_TEXT_3                              _("Note")

// Default values for read/write register functions
// used in device config and scan.
#define VSCP_CANAL_RESEND_TIMEOUT                           1000
#define VSCP_CANAL_ERROR_TIMEOUT                            5000
#define VSCP_CANAL_MAX_TRIES                                3

#define VSCPWORKS_TCPIP_DEFAULT_RESPONSE_TIMEOUT            6000
#define VSCPWORKS_TCPIP_REGISTER_READ_RESEND_TIMEOUT        2000
#define VSCPWORKS_TCPIP_REGISTER_READ_ERROR_TIMEOUT         30000
#define VSCPWORKS_TCPIP_REGISTER_READ_MAX_TRIES             3

#define VSCP_DEVCONFIG_NUMBERBASE_HEX                       0
#define VSCP_DEVCONFIG_NUMBERBASE_DECIMAL                   1

WX_DECLARE_STRING_HASH_MAP( wxString, MdfProxyHash );

// Structure for CANAL nodes
typedef struct {
    unsigned char m_GUID[16];
    wxString m_strRealname;
    wxString m_strHostname;
    unsigned char m_zone;
} canal_nodeinfo;

// Structure for VSCP nodes
typedef struct {
    unsigned char m_GUID[16];
    wxString m_strRealname;
    wxString m_strHostname;
    unsigned char m_zone;
} vscp_nodeinfo;

// Structure for CANAL drivers
typedef struct {
    wxString m_strDescription;      // Decription of driver
    wxString m_strPath;             // Path to driver
    wxString m_strConfig;           // Driver configuration string
    unsigned long m_flags;          // Driver flags
} canal_interface;

// Structure for VSCP drivers
typedef struct {
    wxString m_strDescription;      // Description of VSCP interface
    wxString m_strHost;             // Host where server lives
    wxString m_strUser;             // Username
    wxString m_strPassword;         // Password
    bool m_bLevel2;                 // Full Level II communication
    //unsigned long m_port;         // Port to use on server
    wxString m_strInterfaceName;    // Name for remote interface
    unsigned char m_GUID[16];       // GUID for interface
    vscpEventFilter m_vscpfilter;   // Filter to apply
} vscp_interface;

#define INTERFACE_CANAL   0
#define INTERFACE_VSCP    1

// Structure for VSCP drivers
typedef struct {
    int m_type;
    canal_interface *m_pcanalif;
    vscp_interface *m_pvscpif;
} both_interface;

// Lists for interfaces
WX_DECLARE_LIST(canal_interface, LIST_CANAL_IF );
WX_DECLARE_LIST(vscp_interface, LIST_VSCP_IF );

// Structure for configuration
typedef struct {

    // General
    wxString m_strPathTemp;         // Path to temporary storage
    
    // Logfile
    bool m_bEnableLog;              // True for writes to logfile
    wxString m_strPathLogFile;      // Path to logfile
    uint8_t m_logLevel;             // Log level
    
    // Mainframe
    int m_sizeMainFrameWidth;       // Initial main frame width
    int m_sizeMainFrameHeight;      // Initial main frame height
    int m_xposMainFrame;            // Initial main frame x position
    int m_yposMainFrame;            // Initial main frame y position
    
    // Session window
    int m_sizeSessionFrameWidth;    // Initial session frame width
    int m_sizeSessionFrameHeight;   // Initial session frame height
    
    // Configuration window
    int m_sizeConfigurationFrameWidth;    // Initial configuration frame width
    int m_sizeConfigurationFrameHeight;   // Initial configuration frame height
    
/*
    // CANAL Frames
    wxColour m_CanalRcvFrameTextColour;        // Background colour for CANAL grid
    wxColour m_CanalRcvFrameBgColour;          // Background colour for CANAL grid
    wxColour m_CanalRcvFrameLineColour;        // Foreground colour for CANAL grid
    wxFont m_CanalRcvFrameFont;                // Font for CANAL grid
    
    bool m_CanalRcvShowField[8];               // Flag to show field.
    wxString m_CanalRcvFieldText[8];           // Header text for field
    unsigned char m_CanalRcvFieldOrder[8];     // Order for fields
    unsigned short m_CanalRcvFieldWidth[8];     // Width for fields
    
    wxColour m_CanalTrmitFrameTextColour;      // Background colour for CANAL transmit grid
    wxColour m_CanalTrmitFrameBgColour;        // Background colour for CANAL transmit grid
    wxColour m_CanalTrmitFrameLineColour;      // Foreground colour for CANAL transmit grid
    wxFont m_CanalTrmitFrameFont;              // Font for VSCP grid
    
    bool m_CanalTrmitShowField[5];              // Flag to show field.
    wxString m_CanalTrmitFieldText[5];          // Header text for field
    unsigned char m_CanalTrmitFieldOrder[5];    // Order for fields
    unsigned short m_CanalTrmitFieldWidth[5];   // Order for fields
*/

    
    // VSCP Frames
    wxColour m_VscpRcvFrameRxTextColour;      // Text colour for VSCP grid rx Line
    wxColour m_VscpRcvFrameRxBgColour;        // Background colour for VSCP grid rx line
    wxColour m_VscpRcvFrameTxTextColour;      // Text colour for VSCP grid tx Line
    wxColour m_VscpRcvFrameTxBgColour;        // Background colour for VSCP grid tx line		
    wxColour m_VscpRcvFrameLineColour;        // Line colour for VSCP grid
    wxFont m_VscpRcvFrameFont;                // Font for VSCP grid
    int m_VscpRcvFrameRowLineHeight;          // Line height for rows

    bool m_VscpRcvShowField[ VCSP_RSCV_FIELD_COUNT ];            // Flag to show field.
    wxString m_VscpRcvFieldText[ VCSP_RSCV_FIELD_COUNT ];        // Header text for field
    unsigned char m_VscpRcvFieldOrder[ VCSP_RSCV_FIELD_COUNT ];  // Order for fields
    unsigned short m_VscpRcvFieldWidth[ VCSP_RSCV_FIELD_COUNT ]; // Width for fields

    wxColour m_VscpTrmitFrameTextColour;      // Default Text colour for VSCP grid
    wxColour m_VscpTrmitFrameBgColour;        // Background colour for VSCP grid
    wxColour m_VscpTrmitFrameLineColour;      // Foreground colour for VSCP grid
    wxFont m_VscpTrmitFrameFont;              // Font for VSCP grid

    bool m_VscpTrmitShowField[ VCSP_TRMIT_FIELD_COUNT ];             // Flag to show field.
    wxString m_VscpTrmitFieldText[ VCSP_TRMIT_FIELD_COUNT ];         // Header text for field
    unsigned char m_VscpTrmitFieldOrder[ VCSP_TRMIT_FIELD_COUNT ];   // Order for fields
    unsigned short m_VscpTrmitFieldWidth[ VCSP_TRMIT_FIELD_COUNT ];  // Width for fields

    bool m_bAutoscollRcv;                     // True to scroll upp so last added row
                                              // always is shown
    bool m_VscpRcvFrameRxbPyamas;             // every second row grey if true
    
    bool m_UseSymbolicNames;                  // Use symbolic names for class and type
    
    // Interfaces
    LIST_CANAL_IF m_canalIfList;
    LIST_VSCP_IF m_vscpIfList;

    // DLL communication settings
    uint8_t  m_CANALRegMaxRetries;              // Max number of retries to read a register.
    uint32_t m_CANALRegResendTimeout;           // Timeout before register read retries    
    uint32_t m_CANALRegErrorTimeout;            // Timeout before register read is considered and error

    // TCP/IP communication settings
    uint32_t m_TCPIP_ResponseTimeout;           // General response time in seconds (for all communication)
    uint32_t m_TCPIP_SleepAfterCommand;         // Wait after a command has been sent
    uint8_t	m_TCPIPRegMaxRetries;               // Max number of retries to read a register.
    uint32_t m_TCPIPRegResendTimeout;           // Timeout before register read retries
    uint32_t m_TCPIPRegErrorTimeout;            // Timeout before register read is considered and error
    
    // device configuration
    uint8_t m_Numberbase;                       // Number base for register values

    // Confirm switch 
    bool m_bConfirmDelete;                      // Must confirm session rx/tx etc list delete

    MdfProxyHash m_mfProxyHashTable;            // Translate devices stored URL to whatever is read from
                                                // configuration file
    // Save data from Manufacturer dialog.
    bool bGuidWritable;                         // Should be trye for manufacturing information to be writable
    cguid m_manufacturerGuid;                   // Last used manufacturer GUID
    uint32_t m_manufacturerId;
    uint32_t m_manufacturerSubId;

} appConfiguration;	



// Structure for list fields
typedef struct {
    unsigned short m_pos;
    wxString m_Name;
    unsigned short m_Width;
    bool m_bVisible;
    wxColour m_ForegroundColour;
    wxColour m_BackgroundColour;
} listfield;


// Forward class definitions
class worksMulticastThread;



/*!
 * VscpworksApp class declaration
 */

class VscpworksApp: public wxApp
{    
  DECLARE_CLASS( VscpworksApp )
  DECLARE_EVENT_TABLE()

public:
  /// Constructor
  VscpworksApp();

  void Init();

  //void OnAssertFailure(const wxChar *file, int line, const wxChar *func, const wxChar *cond, const wxChar *msg) { ;};

  /// Initialises the application
  virtual bool OnInit();

  /// Called on exit
  virtual int OnExit();
  
  /*!
    Log message
  */
  void logMsg( const wxString& wxstr, unsigned char level = VSCPWORKS_LOGMSG_INFO );

  /*!
    Handler for asserts
  */
#if (wxMAJOR_VERSION >= 3)
  static void AssertHandler( const wxString &file,
                                    int line,
                                    const wxString &func,
                                    const wxString &cond,
                                    const wxString &msg );
#endif

  /*!
    Read XML configuration from file
    @return true on success.
  */
  bool readConfiguration( void );
  
  /*!
    Write XML configuration to file
    @return true on success.
  */
  bool writeConfiguration( void );
  
 /*!
    Read a level i register
    @param pcsw pointer to superwrapper class.
    @param nodeid for the device which register should be read.
    @param reg Register to read.
    @param pcontent Pointer to read value.
    @return True on success. False otherwise.
  */
  bool readLevel1Register( CCanalSuperWrapper *pcsw,
                            unsigned char nodeid, 
                            unsigned char reg = 0xd0, 
                            unsigned char *pcontent = NULL );
    
  /*!
    Write a level i register
    @param pcsw pointer to superwrapper class.
    @param nodeid for the device which register should be read.
    @param reg Register to write.
    @param pcontent Pointer to data to write. Returns data read.
    @return True on success. False otherwise.
  */
  bool writeLevel1Register( CCanalSuperWrapper *pcsw,
                              unsigned char nodeid, 
                              unsigned char reg, 
                              unsigned char *pcontent );
      
  /*!
    Read a level 2 register
    @param pcsw pointer to superwrapper class.
    @param interfaceGUID GUID for interface where devices sits whos register
          should be read with byte 0 set to nickname id for the device.
    @param reg Register to read.
    @param pcontent Pointer to read value.
    @return True on success. False otherwise.
  */    
  bool readLevel2Register( CCanalSuperWrapper *pcsw, 
                            uint8_t *interfaceGUID, 
                            uint32_t reg = 0xd0, 
                            uint8_t *pcontent = NULL,
                            uint8_t *pdestGUID = NULL,
                            bool bLevel2 = false );  
                            
  /*!
    Write a level 2 register
    @param pcsw pointer to superwrapper class.
    @param interfaceGUID GUID for interface where devices sits whos register
          should be read with byte 0 set to nickname id for the device.
    @param reg Register to write.
    @param pcontent Pointer to data to write. Return read data.
    @return True on success. False otherwise.
  */    
  bool writeLevel2Register( CCanalSuperWrapper *pcsw,
                              uint8_t *interfaceGUID, 
                              uint32_t reg, 
                              uint8_t *pcontent,
                              uint8_t *pdestGUID = NULL,
                              bool bLevel2 = false );   

    /*!
        Make a string look nicer with linebreaks etc
        @param str String to make look nicer
        @param width Linewidth to maintain.
        @return Fromated string.
    */
    wxString formatString( const wxString& str, const unsigned int width = 80 );


    
  
  /*!
    Get Decision Matrix info for a Level I Node
    @param pcsw Pointer to CANAL super wrapper
    @param nodeid id for node whos info should be fetched.
    @param pdata Pointer to returned data. Array of eight bytes.
  */
  bool getLevel1DmInfo( CCanalSuperWrapper *pcsw, 
                          const unsigned char nodeid, 
                          unsigned char *pdata );
             
  /*!
    Get Decision Matrix info for a Level II Node
    @param pcsw Pointer to CANAL super wrapper
    @param interfaceGUID GUID + nodeid for node whos info should be 
            fetched.
    @param pdata Pointer to returned data. Array of eight bytes.
  */
  bool getLevel2DmInfo( CCanalSuperWrapper *pcsw,
                          unsigned char *interfaceGUID, 
                          unsigned char *pdata,
                          bool bLevel2 = false );
  
  /*!
    Get a HTML string with clear text register information
    @param pcsw Pointer to CANAL super wrapper
    @param Pointer registers as a 256 byte register array.
    @return HTML string for wxHtml window or simular.
  */
  wxString getHtmlStatusInfo( CCanalSuperWrapper *pcsw,
                                const uint8_t *registers );

  /*!
    Load and parse mdf from device
    @param pwnd Pointer to window (ownder usually this) that called this method.
    @param pcsw Pointer to CANAL super wrapper
    @param pmdf Pointer to mdf class that will receive result
    @param url Pointer to url that have the mdf file. If this file
    contains "://" as in "http://" it is expected to be a remote file. If
    empty registers will be read.
    if not a local path.
    @param pid A pointer to an interface GUID for Level II node or node id for Level I node.
    @return true on success.
  */
  bool loadMDF( wxWindow *pwnd, CCanalSuperWrapper *pcsw, CMDF *pmdf, wxString& url, uint8_t *pid );

  /*!
    Load level I register content into an array
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param pcsw Pointer to CANAL super wrapper
    @param pregisters Pointer to an array of 256 8-bit registers.
    @param nodeid nodeid The node whos registers should be read.
    @param bQuite No progress information if sett to true. (default is false)
  */
  bool readAllLevel1Registers( wxWindow *pwnd,
                                    CCanalSuperWrapper *pcsw,
                                    uint8_t *pregisters,
                                    uint8_t nodeid );

  /*!
    Load level II register content into an array
    @param pwnd Pointer to window (owner usually this) that called this method.
    @param pcsw Pointer to CANAL super wrapper
    @param pregisters Pointer to an array of 256 8-bit registers.
    @param nodeid nodeid The node whos registers should be read.
    @param bQuite No progress information if sett to true. (default is false)
    return true on success.
  */
  bool readAllLevel2Registers( wxWindow *pwnd,
                                    CCanalSuperWrapper *pcsw,
                                    uint8_t *pregisters,
                                    uint8_t *pinterfaceGUID );

  /*!
        Get MDf file from device registers
        @param pcsw Pointer to CANAL super wrapper
        @param pid Pointer to id. Either a one byte nickname if bLevel = false
                or a 16 byte GUID if bLevel2 = true.
        @param bLevel2 True if pid points to a full  GUID false if pid points to one 
                byte nickname.
  */
  wxString getMDFfromDevice( CCanalSuperWrapper *pcsw, 
                                uint8_t *pid, 
                                bool bLevel2 = false,
                                bool bSilent = false );

  /*!
    Get MDF info.
    @param pmdf Pointer to MDF object holding information
    @return With MDF information in HTML format.

  */
  wxString addMDFInfo( CMDF *pmdf );

  /*!
    This variable is true if the configuration is fetched from
    the global paces /progdata instead of user data location. 
    Typically this happesn when a windows machine has no password.
  */
  bool m_bUseGlobalConfig;

  /*!
    Pointer to the multicast worker thread
  */
  worksMulticastThread *m_pmulticastWorkerThread;
  pthread_mutex_t m_mutexmulticastWorkerThread;

};

/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(VscpworksApp)
////@end declare app












#endif
  // _VSCPWORKS_H_
