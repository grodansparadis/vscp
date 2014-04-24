/////////////////////////////////////////////////////////////////////////////
// Name:        fli4lcidtest.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Thu 15 Nov 2007 15:27:16 CET
// RCS-ID:      
// Copyright:   Copyright (C) 2007 eurosource 
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include <wx/socket.h>
#include <wx/datetime.h>
#include <wx/tokenzr.h>


////@begin includes
////@end includes

#include <my_global.h>
#include <mysql.h>

#include "../../common/vscptcpif.h"
#include "fli4lcidtest.h"

////@begin XPM images
////@end XPM images


/*!
 * fli4lCIDtest type definition
 */

IMPLEMENT_DYNAMIC_CLASS( fli4lCIDtest, wxDialog )


/*!
 * fli4lCIDtest event table definition
 */

BEGIN_EVENT_TABLE( fli4lCIDtest, wxDialog )

////@begin fli4lCIDtest event table entries
    EVT_BUTTON( ID_BUTTON_FETCH, fli4lCIDtest::OnButtonFetchClick )

////@end fli4lCIDtest event table entries

END_EVENT_TABLE()

// Prototypes
bool findPhoneName( MYSQL *pmysql, const wxString& strPhone, wxString& strResult ) ;


/*!
 * fli4lCIDtest constructors
 */

fli4lCIDtest::fli4lCIDtest()
{
    Init();
}

fli4lCIDtest::fli4lCIDtest( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * fli4lCIDtest creator
 */

bool fli4lCIDtest::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin fli4lCIDtest creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end fli4lCIDtest creation
    return true;
}


/*!
 * fli4lCIDtest destructor
 */

fli4lCIDtest::~fli4lCIDtest()
{
////@begin fli4lCIDtest destruction
////@end fli4lCIDtest destruction
}


/*!
 * Member initialisation
 */

void fli4lCIDtest::Init()
{
////@begin fli4lCIDtest member initialisation
    m_ctrlEditValue = NULL;
    m_btnFetch = NULL;
////@end fli4lCIDtest member initialisation
}


/*!
 * Control creation for fli4lCIDtest
 */

void fli4lCIDtest::CreateControls()
{    
////@begin fli4lCIDtest content construction
    fli4lCIDtest* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    m_ctrlEditValue = new wxTextCtrl;
    m_ctrlEditValue->Create( itemDialog1, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxSize(500, -1), 0 );
    itemBoxSizer2->Add(m_ctrlEditValue, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_btnFetch = new wxButton;
    m_btnFetch->Create( itemDialog1, ID_BUTTON_FETCH, _("Fetch"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(m_btnFetch, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end fli4lCIDtest content construction

}


/*!
 * Should we show tooltips?
 */

bool fli4lCIDtest::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap fli4lCIDtest::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin fli4lCIDtest bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end fli4lCIDtest bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon fli4lCIDtest::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin fli4lCIDtest icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end fli4lCIDtest icon retrieval
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_FETCH
 */

void fli4lCIDtest::OnButtonFetchClick( wxCommandEvent& event )
{
  bool bFirstCall = true;
  char buf[ 512 ];
  wxSocketClient m_psock;
  wxString str;
  wxString qstr;
  wxString wxLastPhoneCall;
  
  wxString host = _("192.168.1.254");
  short port = 5001;

 
  // Connect to the OHAS database.
  MYSQL *conn = mysql_init( NULL );
  if ( NULL == conn ) return;
  if ( NULL == mysql_real_connect( conn, "localhost", "root", "ernst320", "odin", 0, NULL, 0 ) ) {
    wxMessageBox(_("Failed to connect to database server."));
    return;
  }
  
/*   
  if ( 0 != mysql_query( conn, "select cid from phonecidlinks where cid='1634'") ) {
    wxMessageBox(_("Failed to perform query."));
    return;
  }
  
  MYSQL_RES *res_set;
  if ( NULL == ( res_set = mysql_store_result( conn ) ) ) {
    wxMessageBox(_("result set store failed."));
  }
  
  MYSQL_ROW row;
  while ( NULL != (row = mysql_fetch_row( res_set ) ) ) {
    wxString str = wxString::FromAscii( row[0] );
    wxMessageBox( str );
  }
  
  mysql_free_result( res_set );
*/  
  
  //if ( 0 != mysql_query( conn, "INSERT INTO phonecall SET `from` = 'this is s test'") ) {
  //  wxMessageBox(_("Failed to perform query."));
  //  return;
  //}
  
  //mysql_close( conn );
  //return;
  
  m_btnFetch->Show( false );
  
  // Here if we are connected
	//m_psock->SetFlags( wxSOCKET_NOWAIT );
	m_psock.SetTimeout(2);

	wxIPV4address addr;
	addr.Hostname( host );
	addr.Service( port );

  while ( true ) {
  
    // Connect to server
    m_psock.Connect( addr, false );
  
    if ( !m_psock.WaitOnConnect( 30 ) ) {	
      wxMessageBox(_("Failed to conect!"));
      return;
    }
  
    m_psock.Read( buf, sizeof( buf ) - 1 );
    if ( m_psock.LastCount() > 0 ) {
      if ( m_psock.LastCount() > 0 ) {
        buf[ m_psock.LastCount() ] = 0;
        str = wxString( buf, wxConvUTF8 );
        m_ctrlEditValue->SetValue( str );
      }
    }
  
    // Disconnect
    m_psock.Close();
    
    if ( bFirstCall ) {
      bFirstCall = false;
      wxLastPhoneCall = str;
    }
    else {
      if ( !wxLastPhoneCall.IsSameAs( str ) ) {
      
        wxLastPhoneCall = str;
        
        wxStringTokenizer tkz( str, _(" ") );
        
        // Move past date and time information
        tkz.GetNextToken();
        tkz.GetNextToken();
        
        // Get number calling 
        wxString fromPhone = tkz.GetNextToken();
        
        // Get number that is eceiving the call
        wxString toPhone = tkz.GetNextToken();
      
        wxMessageBox(_("Phone call: from:") + fromPhone + _(" to:") + toPhone );
        
        // Build the phone number event string
        if ( fromPhone.IsSameAs( wxString(_("0"))) || fromPhone.IsSameAs( wxString(_("00"))) ) {
          fromPhone.Clear();
        }
        
        wxString strPhoneEvent = fromPhone + _(",") + toPhone;
        short nFrames =  strPhoneEvent.Length() / 5;   // Five characters per frame
        if ( strPhoneEvent.Length() % 5 ) nFrames++;
        
        wxString str;
        findPhoneName( conn, fromPhone, str ) ;
        
        qstr = _("select id,cid from phonecidlinks where cid='");
        qstr += fromPhone;
        qstr += _("'");

        if ( 0 != mysql_query( conn, qstr.mb_str() ) ) {
          wxMessageBox(_("Failed to perform query."));
          return;
        }
        
        MYSQL_RES *res_set;
        if ( NULL == ( res_set = mysql_store_result( conn ) ) ) {
          wxMessageBox(_("result set store failed."));
        }
  
        if ( NULL == mysql_fetch_row( res_set ) ) {
        
          // Phone number not present - insert in phonecidcals
          wxString qstr = _("INSERT INTO phonecidlinks (`cid`) VALUES ('");
          qstr += fromPhone;
          qstr += _("')");
          
          if ( 0 != mysql_query( conn, qstr.mb_str() ) ) {
            wxMessageBox(_("Failed to perform query."));
            return;
          }
      
        }
        else {
          // There is a match!
          //    find the phone entry
        }
        
        mysql_free_result( res_set );
        
        
        // Insert the number if the phonecall table
        wxDateTime now = wxDateTime::Now();
        qstr = _("INSERT INTO phonecall (`date`,`from`,`to`,`link_to_phone`) VALUES ('");
        qstr += now.Format( _("%F %H:%M:%S") );
        qstr += _("','");
        qstr += fromPhone;  // Insert description or number
        qstr += _("','");
        qstr += toPhone;    // Insert description or number
        qstr += _("','");
        qstr += _("0");
        qstr += _("')");
          
        if ( 0 != mysql_query( conn, qstr.mb_str() ) ) {
          wxMessageBox(_("Failed to perform query."));
          return;
        }
 
        wxMessageBox( strPhoneEvent + wxString::Format(_( "%d"), nFrames ) );
        
        break;
        
      }
      
    }
    
    // Sleep for a while before next test
    wxMilliSleep( 500 );
    
  } // while
  
  // Close the db connection
  mysql_close( conn );
  
  event.Skip();
}

////////////////////////////////////////////////////////////////////////////////
// findPhoneName
//
// Search the phonennumber is strPhone in the phonecidlink table. 
//
// If not there already store the new number in phonecidlink table and return.
// If there look up he name (company, contact or seco) and return the name in 
// strResult.  

bool findPhoneName( MYSQL *pmysql, const wxString& strPhone, wxString& strResult ) 
{
  wxString str;
  wxString qstr;
  
  if ( NULL == pmysql ) return false; 

  qstr = _("select * from phonecidlinks where cid='");
  qstr += strPhone;
  qstr += _("'");

  if ( 0 != mysql_query( pmysql, qstr.mb_str() ) ) {
      //wxMessageBox(_("Failed to perform query."));
      return false;
  }
        
  MYSQL_RES *res_set;
  if ( NULL == ( res_set = mysql_store_result( pmysql ) ) ) {
    //wxMessageBox(_("result set store failed."));
    return false;
  }
  
  MYSQL_ROW     row;
  MYSQL_FIELD   *field;
  unsigned int num_fields = mysql_num_fields( res_set );
    
  if ( NULL != ( row = mysql_fetch_row( res_set ) ) ) {
  
    // OK its there - Follow the link
    unsigned long *lengths;
    lengths = mysql_fetch_lengths( res_set );
    if ( lengths[2] ) {
      str = wxString::FromAscii( row[2] );
      wxMessageBox( _("[") + str + _("]") );
    }
    else {
      str = _("0");
    }
    
    mysql_free_result( res_set );
    
    MYSQL_RES *result;
    //if ( !str.IsSameAs(_("0")) ) {
      // 
      qstr = _("SELECT contact.id, contact.name, contact.linkto_company FROM contact LEFT JOIN phone ON contact.id = phone.linkto_contact WHERE phone.id = '");
      qstr += str;
      qstr += _("'");
    //}
    
    if ( 0 != mysql_query( pmysql, qstr.mb_str() ) ) {
      wxMessageBox(_("No contact 1."));
      return false;
    }
        
    if ( NULL == ( result = mysql_store_result( pmysql ) ) ) {
      wxMessageBox(_("No Contact 2."));
      return false;
    }
    
    MYSQL_ROW     row2;
    if ( NULL != ( row2 = mysql_fetch_row( result ) ) ) {
        wxMessageBox( _("row") );
        unsigned long *lengths;
        lengths = mysql_fetch_lengths( result );
        if ( lengths[0] ) {
          str = _(":::::::") + wxString::FromAscii( row2[0] );
        }
        else {
          str = _("NULL");
        }
        
         wxMessageBox( str );
    }
    else {
      wxMessageBox( _("No row") );
    }
  
  }
  
  // SELECT contact.id, contact.name, contact.linkto_company FROM contact LEFT JOIN phone ON contact.id = phone.linkto_contact WHERE phone.id = 3 
  // SELECT * FROM contact LEFT JOIN phone ON contact.id = phone.linkto_contact
  // SELECT * FROM contact LEFT JOIN phone ON contact.id = phone.linkto_contact WHERE phone.linkto_contact = 2
  // SELECT contact.id,contact.name FROM contact LEFT JOIN phone ON contact.id = phone.linkto_contact WHERE phone.linkto_contact = 2
  
  mysql_free_result( res_set );
        
  return true;
}

