/////////////////////////////////////////////////////////////////////////////
// Name:        gridtable.cpp
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
//  You should have received a copy of the GNU General Public License
//  along with this file see the file COPYING.  If not, write to
//  the Free Software Foundation, 59 Temple Place - Suite 330,
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "gridtable.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP 
#include "wx/wx.h"
#endif

////@begin includes ŗ
////@end includes 

// Configuration stuff
#include <wx/config.h> 
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include "wx/wfstream.h"
#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/listimpl.cpp>

#ifdef __WXMSW__
#include <wx/msw/regconf.h>
#endif

#include "vscpworks.h"
#include <vscpeventhelper.h>
#include "gridtable.h"   

/*!
    Use this macro for unused function parameters.
 */
#define GRIDTABLE_UNUSED(__par) (void)(__par)

extern appConfiguration g_Config;

extern VSCPInformation g_vscpinfo; // VSCP class type information

WX_DEFINE_LIST(RXLIST); // Receive event list

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors
//

BigGridTable::BigGridTable() : wxGridTableBase()
{
    // List is default view mode
    m_mode = VSCP_GRID_VIEW_MODE_LIST;
    init();
}

BigGridTable::BigGridTable(int numRows, int numCols) : wxGridTableBase()
{
    GRIDTABLE_UNUSED(numRows);
    GRIDTABLE_UNUSED(numCols);

    // List is default view mode
    m_mode = VSCP_GRID_VIEW_MODE_LIST;
    init();
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//

BigGridTable::~BigGridTable()
{
    // Close the file
    //m_temFile.Close();

    // remove the temporary file
    //::wxRemoveFile(m_tempFilePath);

    // Clear grid data
    clearEventList();
}


//////////////////////////////////////////////////////////////////////////////////////////////
// init
//

bool BigGridTable::init()
{
    // Create the temporary file
    //m_tempFilePath = wxFileName::CreateTempFileName(_("vscp"));

    //if (m_temFile.Open(m_tempFilePath, _("w+b"))) {
    //	return false;
    //}

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// GetNumberRows
//

int BigGridTable::GetNumberRows(void)
{
    if (VSCP_GRID_VIEW_MODE_LIST == m_mode) {
        unsigned long len = m_rxList.GetCount();
        return len;
    }
    else {
        return VSCPEventCountHash.size();
    }

}


//////////////////////////////////////////////////////////////////////////////////////////////
// GetValue
//

wxString BigGridTable::GetValue(int row, int col)
{
    static int last_row = 0;
    static VscpRXObj *pRecord = NULL;
    static wxString str;

    if ((0 == last_row) || (row != last_row)) {
        if (NULL == (pRecord = readEvent(row))) return wxString(_(""));
    }

    if (NULL == pRecord) return wxString(_(""));

    // Save the row
    last_row = row;

    switch (col) {

    case VSCP_RCVGRID_COLUMN_DIR:
        if (VSCP_EVENT_DIRECTION_RX == pRecord->m_nDir) {
            return wxString(_("RX"));
        }
        else {
            return wxString(_("TX"));
        }
        break;

    case VSCP_RCVGRID_COLUMN_CLASS:
        if (g_Config.m_UseSymbolicNames) {
            wxString strClass =
                g_vscpinfo.getClassDescription(pRecord->m_pEvent->vscp_class);
            if (0 == strClass.Length()) strClass = _("Unknown class");
            return str.Format(_("%s \n0x%04X, %d"),
                strClass.c_str(),
                pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_class);
        }
        else {
            return str.Format(_("0x%04X, %d"),
                pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_class);
        }

    case VSCP_RCVGRID_COLUMN_TYPE:
        if (g_Config.m_UseSymbolicNames) {
            wxString strType =
                g_vscpinfo.getTypeDescription(pRecord->m_pEvent->vscp_class,
                pRecord->m_pEvent->vscp_type);
            if (0 == strType.Length()) strType = _("Unknown type");
            return str.Format(_("%s \n0x%04X, %d "),
                strType.c_str(),
                pRecord->m_pEvent->vscp_type,
                pRecord->m_pEvent->vscp_type);
        }
        else {
            return str.Format(_("0x%04X, %d"),
                pRecord->m_pEvent->vscp_type,
                pRecord->m_pEvent->vscp_type);
        }

    case VSCP_RCVGRID_COLUMN_NOTE:
        if ( pRecord->m_wxStrNote.Length() ) {
            return pRecord->m_wxStrNote;
        }
        else {
            // Use field for info
            wxString strGUID;
            vscp_writeGuidArrayToString( pRecord->m_pEvent->GUID, strGUID );
            return str.Format(_("Nodeid=%d GUID=%s"), ((pRecord->m_pEvent->GUID[ 14 ]<<8) + pRecord->m_pEvent->GUID[15]), (const char *)strGUID.mbc_str() );
        }

    default:
        str = _("Invalid column");
    }

    return str;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// SetValue
//

void BigGridTable::SetValue(int row, int col, const wxString& strNewVal)
{
    VscpRXObj *pRecord;
    wxString str;

    if (NULL == (pRecord = readEvent(row))) return;

    switch (col) {

    case VSCP_RCVGRID_COLUMN_DIR:
        return; // Can not change direction
        break;

    case VSCP_RCVGRID_COLUMN_CLASS:
        return; // Can not change class
        break;

    case VSCP_RCVGRID_COLUMN_TYPE:
        return; // Can not change type
        break;

    case VSCP_RCVGRID_COLUMN_NOTE:
        str = strNewVal;
        break;

    }

}


//////////////////////////////////////////////////////////////////////////////////////////////
// IsEmptyCell
//

bool BigGridTable::IsEmptyCell(int row, int col)
{
    if (row > (int) (m_rxList.GetCount() - 1)) return true;
    if (col > 3) return true;
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// SetRowLabelValue
//

void BigGridTable::SetRowLabelValue(int row, const wxString &value)
{
    GRIDTABLE_UNUSED(row);
    wxString str = value;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// SetColLabelValue
//

void BigGridTable::SetColLabelValue(int col, const wxString &value)
{
    if (col > (int) (m_colLabels.GetCount()) - 1) {
        int n = m_colLabels.GetCount();

        for (int i = n; i <= col; i++) {
            m_colLabels.Add(wxGridTableBase::GetColLabelValue(i));
        }
    }
    m_colLabels[col] = value;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// GetRowLabelValue
//

wxString BigGridTable::GetRowLabelValue(int row)
{
    return wxString::Format(_("%d"), row);
}


//////////////////////////////////////////////////////////////////////////////////////////////
// GetColLabelValue
//

wxString BigGridTable::GetColLabelValue(int col)
{
    if (col > 3) return wxString(_("Undefined"));
    return g_Config.m_VscpRcvFieldText[ col ];
}


//////////////////////////////////////////////////////////////////////////////////////////////
// ClearGrid
//

void BigGridTable::ClearGrid(void)
{
    ;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// AppendRows
//

bool BigGridTable::AppendRows(size_t numRows)
{
    if (GetView()) {
        wxGridTableMessage msg(this,
            wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
            numRows);

        GetView()->ProcessTableMessage(msg);
    }

    return true;

}


//////////////////////////////////////////////////////////////////////////////////////////////
// AppendCols
//

bool BigGridTable::AppendCols(int numCols, bool updateLabels)
{
    GRIDTABLE_UNUSED(updateLabels);

    if (GetView()) {
        wxGridTableMessage msg(this,
            wxGRIDTABLE_NOTIFY_COLS_APPENDED,
            numCols);

        GetView()->ProcessTableMessage(msg);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// DeleteRows
//

bool BigGridTable::DeleteRows(size_t pos, size_t numRows)
{
    if (GetView()) {
        wxGridTableMessage msg(this,
            wxGRIDTABLE_NOTIFY_ROWS_DELETED,
            pos, numRows);

        GetView()->ProcessTableMessage(msg);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// DeleteCols
//

bool BigGridTable::DeleteCols(int pos, int numCols, bool updateLabels)
{
    GRIDTABLE_UNUSED(updateLabels);

    if (GetView()) {
        wxGridTableMessage msg(this,
            wxGRIDTABLE_NOTIFY_COLS_DELETED,
            pos, numCols);

        GetView()->ProcessTableMessage(msg);

    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// clearEventList
//

void BigGridTable::clearEventList(void)
{
    RXLIST::iterator iterEvent;
    for (iterEvent = m_rxList.begin();
        iterEvent != m_rxList.end();
        ++iterEvent) {
        VscpRXObj *pRecord = *iterEvent;
        if (NULL != pRecord) {
            if (NULL != pRecord->m_pEvent->pdata) delete [] pRecord->m_pEvent->pdata;
            if (NULL != pRecord->m_pEvent) delete pRecord->m_pEvent;
            delete pRecord;
        }
    }

    m_rxList.clear();

}


//////////////////////////////////////////////////////////////////////////////////////////////
// readEvent
//

VscpRXObj *BigGridTable::readEvent(unsigned long pos)
{
    if (pos >= m_rxList.GetCount()) return NULL;

    return m_rxList.Item(pos)->GetData();


    //convertVSCPtoEx( pEvent, &pRecord->m_event ); 

    /*
      // File should be open
      if ( !m_temFile.IsOpened() ) return false;
  
      // Calculate # objects
      wxFileOffset size = m_temFile.Length()/sizeof(VscpRecord);
  
      // Must be within current range
      if ( pos >= size ) return false;
  
      wxFileOffset newpos = pos * sizeof(VscpRecord);
      if ( newpos != m_temFile.Tell() ) {
        m_temFile.Seek( newpos );
      }
  
      // Write record
      if ( sizeof(VscpRecord) != 
            m_temFile.Read( &pRecord->m_event, sizeof(VscpRecord) ) ) {
        return false;
      }
     */

}



//////////////////////////////////////////////////////////////////////////////////////////////
// writeEvent
//

bool BigGridTable::writeEvent(VscpRXObj *pRecord, bool bUpdate)
{
    // Validate pointer
    //if ( NULL == pRecord ) return false;

    // File should be open
    //if ( !m_temFile.IsOpened() ) return false;

    // Go to end
    //m_temFile.SeekEnd( 0 );

    // Write record
    //if ( sizeof(VscpRecord) != m_temFile.Write( &pRecord->m_event, sizeof(VscpRecord) ) ) {
    //  return false;
    //}

    m_rxList.Append(pRecord);

    if (bUpdate) {

        // Add one row to the grid
        AppendRows(1);

        //wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, 1);
        //GetView()->ProcessTableMessage(msg);
        //if ( GetView() )
        //{
        //  wxGridTableMessage msg( this,
        //                          wxGRIDTABLE_NOTIFY_ROWS_APPENDED,
        //                          1 );

        //  GetView()->ProcessTableMessage( msg );
        //}

        // If selected mow the added row into seight
        if (g_Config.m_bAutoscollRcv) {
            GetView()->MakeCellVisible(m_rxList.GetCount() - 1, 0);
        }
        /*	
              if ( VSCP_EVENT_DIRECTION_RX == pRecord->m_nDir ) {
                  for ( int i=0; i<10; i++ ) {
                      GetView()->SetCellTextColour( m_rxList.GetCount()-1, i, g_Config.m_VscpRcvFrameRxTextColour  );
                      GetView()->SetCellBackgroundColour( m_rxList.GetCount()-1, i, g_Config.m_VscpRcvFrameRxBgColour );
                  }
              }
              else {
                  for ( int i=0; i<10; i++ ) {
                      GetView()->SetCellTextColour( m_rxList.GetCount()-1, i, g_Config.m_VscpRcvFrameTxTextColour );
                      GetView()->SetCellBackgroundColour( m_rxList.GetCount()-1, i, g_Config.m_VscpRcvFrameTxBgColour );	
                  }		
              }
    
              // RX/TX
              GetView()->SetCellAlignment( m_rxList.GetCount()-1, 0, wxALIGN_CENTRE, wxALIGN_CENTRE );
              // Class
            GetView()->SetCellAlignment( m_rxList.GetCount()-1, 3, wxALIGN_CENTRE, wxALIGN_CENTRE );
              // Type
              GetView()->SetCellAlignment( m_rxList.GetCount()-1, 4, wxALIGN_CENTRE, wxALIGN_CENTRE );
              // Head
              GetView()->SetCellAlignment( m_rxList.GetCount()-1, 5, wxALIGN_CENTRE, wxALIGN_CENTRE );
              // Count
              GetView()->SetCellAlignment( m_rxList.GetCount()-1, 6, wxALIGN_CENTRE, wxALIGN_CENTRE );
         */
        GetView()->ForceRefresh();

    }

    return true;
}






