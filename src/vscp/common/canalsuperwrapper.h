///////////////////////////////////////////////////////////////////////////////
// CanalSuperWrapper.h: interface for the CCanalSuperWrapper class
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: canalsuperwrapper.h,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
///////////////////////////////////////////////////////////////////////////////

/*!
    \file canalsuperwrapper.h
    \brief Wrapping class for VSCP interfaces. 
    \details This class ecapsulates both the CANAL dll and the VSCP TCP/IP interface and
        makes it possible to use the same code to read/write both CANAL messages and VSCP 
        events.
    \author Ake Hedman <akhe@grodansparadis.com>, Grodans Paradis AB, Sweden
*/

#include <string.h>
#include "devitem.h"
#include "dllwrapper.h"
#include "vscptcpif.h"
#include "guid.h"
#include "../../common/dllist.h"

#if !defined(AFX_CANALSUPERWRAPPER_H__A908F21A_317D_4E74_9308_18D7DD6B7D49__INCLUDED_)
#define AFX_CANALSUPERWRAPPER_H__A908F21A_317D_4E74_9308_18D7DD6B7D49__INCLUDED_

#include "canal.h"
#include "devitem.h"

/// Build swig code if defined
#define BUILD_SWIG              // Must be here for SWIG builds, Can be commented out for C/C++

/*!@{
    Constants for possible interfaces
@}*/
/// Cnstant that defines the CANAL interface
#define USE_DLL_INTERFACE       0
/// Constat that defines the TCP/IP interface
#define USE_TCPIP_INTERFACE     1


/// Maximum path length (if nit defined by the OS if any.
#ifndef MAX_PATH
#define MAX_PATH   255
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
    CCanalSuperWrapper( void );

    /*!
        Set Interface CANAL
        Use this method if the default constructor is used to construct the object
        pItem->name is the user name for the driver.
        pItem->path is the location of the driver on the disc. Path can be set to "TCPIP"
            in which case the config string holds connect information.
        pItem->config is the configuration string for the driver. If path = "TCPIP" the
            this string is searched for host connect information on the form
            "username;password;host;port"
        pItem->flags is the configuration flags for the driver. Not used for TCP/IP
            interface.
    */
    CCanalSuperWrapper( devItem *pItem );

    /// Destructor
    virtual ~CCanalSuperWrapper();

    /*!
        Set Interface TCP/IP
        Use this method if the default constructor is used to construct the object
        @param host to connect to (defaults to localhost if not given).
        @param port on host to connect to. Default to 9598 if not given.
        @param username Username to login to service.
        @param password to login to service.
    */
    void setInterface( const wxString& host, 
                          const short port,
                          const wxString& username,
                          const wxString& password );
                      
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
    void setInterface( const wxString& name,
                          const wxString& path,
                          const wxString& parameters,
                          const unsigned long flags,
                          const unsigned long filter,
                          const unsigned long mask );
 
    /*!
        Open communication channel.

        Data can be supplied to override the pdev structure information
        that was given on construction.

        @param strInterface is name of channel.
        @param flags Interface flags
        @return true if channel is open or false if error or the channel is
        already opened.
    */
    long doCmdOpen( const wxString& strInterface = (_T("")), unsigned long flags = 0L );

    /*!
        Close communication channel

        @return true if the close was successfull
    */
    int doCmdClose( void );

    /*!
        Do a no operation test command. 

        @return true if success false if not.
    */
    int doCmdNOOP( void );

 
    /*!
        Get the CANAL protocol level

        @return true on success
    */
    unsigned long doCmdGetLevel( void );

    /*!
        Send a CANAL message. 

        @return true if success false if not.
    */
    int doCmdSend( canalMsg *pMsg );

	/*!
        Send a VSCP event. 

        @return true if success false if not.
    */
    int doCmdSend( const vscpEvent *pEvent );
  
	/*!
        Send a VSCP Ex event. 

        @return true if success false if not.
    */
    int doCmdSend( const vscpEventEx *pEventEx );

    /*!
        Receive a CANAL message. 

        @return true if success false if not.
    */
    int doCmdReceive( canalMsg *pMsg );

	/*!
        Receive a VSCP event. 

        @return true if success false if not.
    */
    int doCmdReceive( vscpEvent *pEvent );
  
	/*!
        Receive a VSCP Ex event. 

        @return true if success false if not.
    */
    int doCmdReceive( vscpEventEx *pEventEx );

    /*!
        Get the number of messages in the input queue

        @return the number of messages available or if negative
        an error code.
    */
    int doCmdDataAvailable( void );

    /*!
        Receive CANAL status. 

        @return true if success false if not.
    */
    int doCmdStatus( canalStatus *pStatus );

    /*!
        Receive CANAL statistics through the pipe. 

        @return true if success false if not.
    */
    int doCmdStatistics( canalStatistics *pStatistics );

    /*!
        Set/Reset a filter through the pipe. 

        @return true if success false if not.
    */
    int doCmdFilter( unsigned long filter );

    /*!
        Set/Reset a mask through the pipe. 

        @return true if success false if not.
    */
    int doCmdMask( unsigned long mask );


	/*!
        Set/Reset the VSCP filter for a channel. 

        @return true if success false if not.
    */
    int doCmdVscpFilter( const vscpEventFilter *pFilter );


    /*!
        Set baudrate. 

        @return true if success false if not.
    */
    int doCmdBaudrate( unsigned long baudrate );

    /*!
        Get i/f version. 

        @return true if success false if not.
    */
    unsigned long doCmdVersion( void );


    /*!
        Get dll version. 

        @return true if success false if not.
    */
    unsigned long doCmdDLLVersion( void );

    /*!
        Get vendorstring. 

        @return Pointer to vendor string.
    */
    const char *doCmdVendorString( void );

    /*!
        Get device information string. 

        @return pointer to driver information string.
    */
    const char *doCmdGetDriverInfo( void );

    /*!
        Get the type of interface that is active.
        @return USE_DLL_INTERFACE if direct DLL interface active.
        @return USE_TCPIP_INTERFACE	if TCP/IP interface active.
    */
    int getDeviceType( void ) { return m_itemDevice.id; };


    /*!
        Check if interface is open
        @return Return tru if if open, false if closed.
    */
    bool isOpen( void ) { return ( m_devid ? true : false ); };  

    /*!
        Shutdown the VSCP daemon

        @return CANAL_ERROR_SUCCESS if success CANAL_ERROR_GENERIC on falure.
    */
    int doCmdShutDown( void );
  
    /*!
        Get a pointer to the TCP/IP interface
        @return A pointer to the TCP/IP interface
    */
    VscpTcpIf *getTcpIpInterface( void ) { return &m_vscptcpif; };

    /*!
        Get variable value from string variable
        \param name of variable
        \param strValue pointer to string that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableString( wxString& name, wxString *strValue )
            { return m_vscptcpif.getVariableString( name, strValue ); };

    /*!
        Set variable value from string variable
        \param name of variable
        \param strValue to string that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableString( wxString& name, wxString& strValue )
            { return m_vscptcpif.setVariableString( name, strValue ); };

    /*!
        Get variable value from boolean variable
        \param name of variable
        \param bValue pointer to boolean variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableBool( wxString& name, bool *bValue )
            { return m_vscptcpif.getVariableBool( name, bValue ); };

    /*!
        Set variable value from boolean variable
        \param name of variable
        \param bValue boolean variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableBool( wxString& name, bool bValue )
            { return m_vscptcpif.setVariableBool( name, bValue ); };

    /*!
        Get variable value from integer variable
        \param name of variable
        \param value pointer to integer variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableInt( wxString& name, int *value )
            { return m_vscptcpif.getVariableInt( name, value ); };

    /*!
        set variable value from integer variable
        \param name of variable
        \param value integer variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableInt( wxString& name, int value )
            { return m_vscptcpif.setVariableInt( name, value ); };

    /*!
        Get variable value from long variable
        \param name of variable
        \param value pointer to long variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableLong( wxString& name, long *value )
            { return m_vscptcpif.getVariableLong( name, value );  };

    /*!
        Set variable value from long variable
        \param name of variable
        \param value long variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableLong( wxString& name, long value )
            { return m_vscptcpif.setVariableLong( name, value );  };

    /*!
        Get variable value from double variable
        \param name of variable
        \param value pointer to double variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableDouble( wxString& name, double *value )
            { return m_vscptcpif.getVariableDouble( name, value ); };

    /*!
        Set variable value from double variable
        \param name of variable
        \param value pointer to double variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableDouble( wxString& name, double value )
            { return m_vscptcpif.setVariableDouble( name, value ); };

    /*!
        Get variable value from measurement variable
        \param name of variable
        \param strValue String that get that get the 
        value of the measurement.
        \return true if the variable is of type string.
    */
    bool getVariableMeasurement( wxString& name, wxString& strValue )
            { return m_vscptcpif.getVariableMeasurement( name, strValue ); };

    /*!
        Set variable value from measurement variable
        \param name of variable
        \param strValue String that get that get the 
        value of the measurement.
        \return true if the variable is of type string.
    */
    bool setVariableMeasurement( wxString& name, wxString& strValue )
            { return m_vscptcpif.setVariableMeasurement( name, strValue ); };

    /*!
        Get variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableEvent( wxString& name, vscpEvent *pEvent )
            { return m_vscptcpif.getVariableEvent( name, pEvent ); };

    /*!
        Set variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableEvent( wxString& name, vscpEvent *pEvent )
            { return m_vscptcpif.setVariableEvent( name, pEvent ); };

    /*!
        Get variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableEventEx( wxString& name, vscpEventEx *pEvent )
            { return m_vscptcpif.getVariableEventEx( name, pEvent ); };

    /*!
        Set variable value from event variable
        \param name of variable
        \param pEvent pointer to event variable that get the value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableEventEx( wxString& name, vscpEventEx *pEvent )
            { return m_vscptcpif.setVariableEventEx( name, pEvent ); };

    /*!
        Get variable value from GUID variable
        \param name of variable
        \param GUID variable that get the value of the GUID variable.
        \return true if the variable is of type string.
    */
    bool getVariableGUID( wxString& name, cguid& GUID )
            { return m_vscptcpif.getVariableGUID( name, GUID ); };

    /*!
        Set variable value from GUID variable
        \param name of variable
        \param GUID variable that get the value of the GUID variable.
        \return true if the variable is of type string.
    */
    bool setVariableGUID( wxString& name, cguid& GUID )
            { return m_vscptcpif.setVariableGUID( name, GUID ); };

    /*!
        Get variable value from VSCP data variable
        \param name of variable
        \param psizeData Pointer to variable that will hold the size of the data array
        \param pData pointer to VSCP data array variable (unsigned char [8] ) that get the 
        value of the string variable.
        \return true if the variable is of type string.
    */
    bool getVariableVSCPdata( wxString& name, uint16_t *psizeData, uint8_t *pData )
            { return m_vscptcpif.getVariableVSCPdata( name, psizeData, pData ); };

    /*!
        Set variable value from VSCP data variable
        \param name of variable
        \param sizeData to variable that will hold the size of the data array
        \param pData pointer to VSCP data array variable (unsigned char [8] ) that get the 
        value of the string variable.
        \return true if the variable is of type string.
    */
    bool setVariableVSCPdata( wxString& name, uint16_t sizeData, uint8_t *pData )
            { return m_vscptcpif.setVariableVSCPdata( name, sizeData, pData ); };

    /*!
        Get variable value from class variable
        \param name of variable
        \param vscp_class pointer to int that get the value of the class variable.
        \return true if the variable is of type string.
    */
    bool getVariableVSCPclass( wxString& name, uint16_t *vscp_class )
            { return m_vscptcpif.getVariableVSCPclass( name, vscp_class ); };

    /*!
        Set variable value from class variable
        \param name of variable
        \param vscp_class int that get the value of the class variable.
        \return true if the variable is of type string.
    */
    bool setVariableVSCPclass( wxString& name, uint16_t vscp_class )
            { return m_vscptcpif.setVariableVSCPclass( name, vscp_class ); };

    /*!
        Get variable value from type variable
        \param name of variable
        \param vscp_type pointer to int that get the value of the type variable.
        \return true if the variable is of type string.
    */
    bool getVariableVSCPtype( wxString& name, uint8_t *vscp_type )
            { return m_vscptcpif.getVariableVSCPtype( name, vscp_type ); };


    /*!
        Set variable value from type variable
        \param name of variable
        \param vscp_type to int that get the value of the type variable.
        \return true if the variable is of type string.
    */
    bool setVariableVSCPtype( wxString& name, uint8_t vscp_type )
            { return m_vscptcpif.setVariableVSCPtype( name, vscp_type ); };


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
        CANAL TCP/IP interface
    */
    VscpTcpIf m_vscptcpif;

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

};

#endif // !defined(AFX_CANALSUPERWRAPPER_H__A908F21A_317D_4E74_9308_18D7DD6B7D49__INCLUDED_)
