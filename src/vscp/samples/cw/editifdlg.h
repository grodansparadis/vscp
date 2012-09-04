// EditIfDlg.cpp
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
// $RCSfile: editifdlg.h,v $                                       
// $Date: 2005/03/10 23:01:29 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#if !defined(AFX_EDITIFDIALOG_H__2A4A7FC3_C572_4C69_994D_9750357A3527__INCLUDED_)
#define AFX_EDITIFDIALOG_H__2A4A7FC3_C572_4C69_994D_9750357A3527__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wx/spinctrl.h>

class CEditIfDlg  : public wxDialog
{

 public:

  /*!
    Constructor
  */
  CEditIfDlg( wxWindow *parent );

  /*!
    Destructor
  */
  virtual ~CEditIfDlg();

  /*!
    Save settings
  */
  void OnOK( wxCommandEvent& event );

  /*!
    Cancel 
  */
  void OnCancel( wxCommandEvent& event );


  /*!
    Driver path
  */
  void OnButtonDriverPath( wxCommandEvent& event );


 public:

  /*!
    Driver name edit field
  */
  wxString m_wxStrDriverName;

  /*!
    Driver path edit field
  */
  wxString m_wxStrDriverPath;

  /*!
    Device string
  */
  wxString m_wxStrDriverDeviceString;

  /*!
    Device Flags
  */
  wxString m_wxStrDriverFlags;

  /*!
    Device In Buffer Size
  */
  wxString m_wxStrDriverInBufSize;

  /*!
    Device Out Buffer Size
  */
  wxString m_wxStrDriverOutBufSize;

  /*!
    Device Filter
  */
  wxString m_wxStrDriverFilter;

  /*!
    Device Mask
  */
  wxString m_wxStrDriverMask;
 

 private:

  // Standard buttons
  wxButton *m_pOKButton;
  wxButton *m_pCancelButton;

  // Buttons
  wxButton *m_pButtonBrowsePath;

  // Edit Controls
  wxTextCtrl *m_pEditCtrlName;
  wxTextCtrl *m_pEditCtrlPath;
  wxTextCtrl *m_pEditCtrlDeviceString;
  wxTextCtrl *m_pEditCtrlFlags;
  wxTextCtrl *m_pEditCtrlInBufSize;
  wxTextCtrl *m_pEditCtrlOutBufSize;
  wxTextCtrl *m_pEditCtrlFilter;
  wxTextCtrl *m_pEditCtrlMask;


  // any class wishing to process wxWidgets events must use this macro
  DECLARE_EVENT_TABLE()

    };


    enum 
      {
	EditIfBtnSave = 300,
	EditIfBtnCancel,
	EditIfBtnBrowsePath,
	EditIfDeviceName,
	EditIfDevicePath,
	EditIfDeviceString,
	EditIfDeviceFlags,
	EditIfDeviceFilter,
	EditIfDeviceMask,
	EditIfDeviceInBufSize,
	EditIfDeviceOutBufSize
      };

#endif // !defined(AFX_EDITIFDIALOG_H__2A4A7FC3_C572_4C69_994D_9750357A3527__INCLUDED_)
