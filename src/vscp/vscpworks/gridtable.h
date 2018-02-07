/////////////////////////////////////////////////////////////////////////////
// Name:        gridtable.h
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
/////////////////////////////////////////////////////////////////////////////

#ifndef _GRIDTABLE_H_
#define _GRIDTABLE_H_

#include <wx/grid.h>
#include <wx/ffile.h>
#include <vscp.h>
#include <vscpremotetcpif.h>
#include <vscphelper.h>


/// The event hash table
WX_DECLARE_HASH_MAP( unsigned long, vscpEventEx*, wxIntegerHash, wxIntegerEqual, VSCPEventHash );

/// The event count hash table (key = class+type, value=count)
WX_DECLARE_STRING_HASH_MAP( long, VSCPEventCountHash );



enum {
    VSCP_GRID_VIEW_MODE_LIST,
    VSCP_GRID_VIEW_MODE_COUNT
};


enum {
    VSCP_RCVGRID_COLUMN_DIR = 0,
    VSCP_RCVGRID_COLUMN_CLASS,
    VSCP_RCVGRID_COLUMN_TYPE,
    VSCP_RCVGRID_COLUMN_NOTE
};

enum {
    VSCP_TXGRID_COLUMN_ACTIVE = 0,
    VSCP_TXGRID_COLUMN_NAME,
    VSCP_TXGRID_COLUMN_PERIOD,
    VSCP_TXGRID_COLUMN_COUNT,
    VSCP_TXGRID_COLUMN_TRIGGER,
    VSCP_TXGRID_COLUMN_MSG
};

#define VSCP_EVENT_DIRECTION_RX   0
#define VSCP_EVENT_DIRECTION_TX   1

class VscpRXObj
{
public:

  /// Constructor
  VscpRXObj() { m_pEvent = NULL;
                  m_nDir=VSCP_EVENT_DIRECTION_RX; 
                  m_time = wxDateTime::Now(); };

public:

   unsigned char m_nDir;

  /// VSCP event pointer
  vscpEvent *m_pEvent;

  /// Date/Time received
  wxDateTime m_time;
  
  /// Grid line note
  wxString m_wxStrNote;
  
};



/// Grid event list
WX_DECLARE_LIST( VscpRXObj, RXLIST );



class BigGridTable : public wxGridTableBase
{

public:
  
    /// Constructor
    BigGridTable();
    BigGridTable( int numRows, int numCols );
    
    /// Destructor
    virtual ~BigGridTable();

    /*!
      Get total number of rows in the grid
    */
    int GetNumberRows();
    

    /*!
      Get the number of columns for the grid
      @return Number of columns.
    */
    int GetNumberCols() {return 4; };  
    

    wxString GetValue( int row, int col );
    void SetValue( int row, int col, const wxString& strNewVal );
    
    wxString GetRowLabelValue(int row);
    void SetRowLabelValue( int row, const wxString &value );

    wxString GetColLabelValue(int col);
    void SetColLabelValue( int col, const wxString &value );
    
    
    void ClearGrid( void );
    bool IsEmptyCell( int row, int col );

    bool AppendRows( size_t numRows = 1 );
    bool DeleteRows( size_t pos = 0, size_t numRows = 1 );

    bool AppendCols(int numCols = 1, bool updateLabels = true);  
    bool DeleteCols(int pos = 0, int numCols = 1, bool updateLabels = true);

    bool InsertCols(int pos = 0, int numCols = 1, bool updateLabels = true);
    bool InsertRows(int pos = 0, int numRows = 1, bool updateLabels = true);
    
    wxGridCellAttr* GetAttr(int row, int col);


    /*!
      Initialize the grid
      @return true on success. false on failure. 
    */
    bool init( void );
    
    /*!
      Write one VSCP event to the current pos
      @param pEvent Pointer to VSCP event.
      @return true on success. false on failure. 
    */
    bool writeEvent( VscpRXObj *pRecord, bool bUpdate = true );
    
    /*!
      Read one VSCP event from a specific pos in the temp file
      @param pos Position to read from (offset zero).
      @param pRecord Pointer to VSCP event in record form.
      @return true on success. false on failure. 
    */
    VscpRXObj * readEvent( unsigned long pos );
    
    /*!
      Clear the event list
    */
    void clearEventList( void );


    /// Mode 
    unsigned char m_mode;
    
    /// Path of temporary file
    wxString m_tempFilePath;
    
    /// Temp buffer file
    wxFFile m_temFile;

    /// List with events for grid
    RXLIST m_rxList;
    
    /// Hash table with class/type counts
    VSCPEventHash VSCPEventCountHash;
    
    wxArrayString  m_rowLabels;
    wxArrayString  m_colLabels;
    
};

#endif
