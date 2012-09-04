// SelectIfDlg.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// canald.cpp 
//
// This file is part of the CANAL (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: selectifdlg.h,v $                                       
// $Date: 2005/09/15 19:34:50 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#if !defined(AFX_SELECTIFDIALOG_H__2A4A7FC3_C572_4C69_994D_9750357A3527__INCLUDED_)
#define AFX_SELECTIFDIALOG_H__2A4A7FC3_C572_4C69_994D_9750357A3527__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wx/spinctrl.h>
#include <wx/listctrl.h>

#include "../../common/canal.h"

// Maximum drivers that can be loaded
#define MAX_DRIVERS	256


class CSelectIfDlg  : public wxDialog
{

 public:

  /*!
    Constructor
  */
  CSelectIfDlg( wxWindow *parent );


  /*!
    Destructor
  */
  virtual ~CSelectIfDlg();


  /*!
    Save settings
  */
  void OnOK( wxCommandEvent& event );


  /*!
    Cancel 
  */
  void OnCancel( wxCommandEvent& event );


  /*!
    Add a new interface
  */
  void OnAddIf( wxCommandEvent& event );


  /*!
    Edit an interface
  */
  void OnEditIf( wxCommandEvent& event );


  /*!
    Remove ab interface
  */
  void OnRemoveIf( wxCommandEvent& event );



  /*!
    Fill the device listbox with availabe devices
  */
  void fillIfListBox( void );


  /*!
    Load device data from the configuration file
  */
  void loadDeviceConfigurationData( void );

  /*!
    Get selected device or NULL if no device
    is delected or the CANAL daemon interface should be used,

    @return Pointer to selcted device Item
  */
  devItem *getSelectedDevice( void );

  /*!
    Get device profile

    @param pDev Device item structure
    @return true on success.
  */
  bool getDeviceProfile( devItem *pDev );


  /*!
    Get a numerical data value (hex or decimal) from a string
    @param szData Strng containing value in string form
    @return The converted number
  */
  uint32_t getDataValue( const char *szData );


  /*!
	Replaces backslahes in a string with standard slashes
	@param wxstr wxString with data to work on
	@return Clean string
  */
  wxString replaceBackslash( wxString& wxstr );

 
 public:

  /*!
    Interface Listbox
  */
  wxListBox *m_pinterfaceList;
 
  /*!
    Selected device or NULL
    if shared memory
  */
  devItem *m_sel_pdev;

 private:

  // Standard buttons
  wxButton *m_pOKButton;
  wxButton *m_pCancelButton;

  // Buttons
  wxButton *m_pButtonAddIf;
  wxButton *m_pButtonEditIf;
  wxButton *m_pButtonRemoveIf;


  /*!
    List with device profiles from the registry
  */
  devItem *m_deviceList[ 256 ];


  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()

    };


    enum 
      {
	BtnSave = 200,
	BtnCancel,
	IfList,
	ButtonAddIf,
	ButtonEditIf,
	ButtonRemoveIf
      };

#endif // !defined(AFX_SELECTIFDIALOG_H__2A4A7FC3_C572_4C69_994D_9750357A3527__INCLUDED_)
