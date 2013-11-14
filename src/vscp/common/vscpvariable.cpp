// vscpvariable.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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



#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif


#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/stdpaths.h>

#include <wx/listimpl.cpp>

#include "vscp.h"
#include "vscphelper.h"
#include "variablecodes.h"
#include "vscpvariable.h"

WX_DEFINE_LIST( listVscpVariable );

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVSCPVariable::CVSCPVariable( void )
{
    m_type = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    m_bPersistent = false;			// Not persistent by default
    m_bArray = false;				// Nor an array by default

    m_boolValue = false;
    m_event.pdata = NULL;
    m_longValue = 0;
    m_floatValue = 0;
    m_normIntSize = 0;
    memset( m_normInteger, 0, sizeof( m_normInteger ) );
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPVariable::~CVSCPVariable( void )
{
    // Clean up VSCP event data
    if ( NULL != m_event.pdata ) delete m_event.pdata;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableType
//

uint8_t CVSCPVariable::getVariableTypeFromString( const wxString& strVariableType )
{
    uint8_t type = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    wxString str = strVariableType;
    long val;

    if ( str.IsNumber() ) {
        str.ToLong( &val, 10 );
        type = val;
    }
    else {
        str.Trim();
        str.Trim( false );
        str = str.Upper();

        if ( 0 == str.Find( _("STRING") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        }
        else if ( 0 == str.Find( _("BOOL") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_BOOLEAN;
        }
        else if ( 0 == str.Find( _("BOOLEAN") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_BOOLEAN;
        }
        else if ( 0 == str.Find( _("INT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_INTEGER;
        }
        else if ( 0 == str.Find( _("INTEGER") )  ) {
            type = VSCP_DAEMON_VARIABLE_CODE_INTEGER;
        }
        else if ( 0 == str.Find( _("LONG") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_LONG;
        }
        else if ( 0 == str.Find( _("FLOAT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DOUBLE;
        }
        else if ( 0 == str.Find( _("DOUBLE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DOUBLE;
        }
        else if ( 0 == str.Find( _("MEASUREMENT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT;
        }
        else if ( 0 == str.Find( _("EVENT") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT;
        }
        else if ( 0 == str.Find( _("GUID") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID;
        }
        else if ( 0 == str.Find( _("DATA") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA;
        }
        else if ( 0 == str.Find( _("CLASS") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS;
        }
        else if ( 0 == str.Find( _("TYPE") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE;
        }
        else if ( 0 == str.Find( _("TIMESTAMP") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP;
        }
        else if ( 0 == str.Find( _("DATETIME") ) ) {
            type = VSCP_DAEMON_VARIABLE_CODE_DATETIME;
        }

    }

    return type;

}

///////////////////////////////////////////////////////////////////////////////
// setName
//

void CVSCPVariable::setName( const wxString& strName )
{
    wxString str = strName;
    str.Trim( false );
    str.Trim( true );
    m_name = str= strName.Upper();

    // Check if it is a persistent variable
    if ( str[ 0 ] == VSCP_DAEMON_VARIABLE_PREFIX_PERSISTENT ) {
        m_bPersistent = true;
        // We save with standard name
        m_name = str.Right( str.Length() - 1 );
    }

    // Check if it is an array
    if ( str[ 0 ] == VSCP_DAEMON_VARIABLE_PREFIX_ARRAY ) {
        m_bArray = true;
        // We save with standard name
        m_name = str.Right( str.Length() - 1 );
    }
}



///////////////////////////////////////////////////////////////////////////////
// writeVariableToString
//

bool CVSCPVariable::writeVariableToString( wxString& strVariable )
{
    wxString str = wxT("");
    switch ( m_type ) { 

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            strVariable = m_strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
            if ( m_boolValue ) {
                strVariable.Printf(_("true"));
            }
            else {
                strVariable.Printf(_("false"));
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            strVariable.Printf(_("%d"), m_longValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            strVariable.Printf(_("%ld"), m_longValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            strVariable.Printf(_("%f"), m_floatValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            writeVscpDataWithSizeToString( m_normIntSize, m_normInteger, strVariable );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            writeVscpEventToString( &m_event, strVariable );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            writeGuidToString( &m_event, strVariable );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            writeVscpDataToString( &m_event, strVariable );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            strVariable.Printf(_("%d"), m_event.vscp_class );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            strVariable.Printf(_("%d"), m_event.vscp_type );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            strVariable.Printf(_("%d"), m_event.timestamp );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            strVariable = m_timestamp.FormatISODate();
            strVariable += wxT(" ");
            strVariable += m_timestamp.FormatISOTime();
            break;

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:

            // Fall through

        default:
            strVariable.Printf(_("UNKNOWN"));
            break;
    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// setValueFromString
//

bool CVSCPVariable::setValueFromString( int type, const wxString& strValue )
{	
    // Convert to uppercase
    wxString strUpper;

    switch ( type ) { 

        case VSCP_DAEMON_VARIABLE_CODE_STRING:
            m_strValue = strValue;
            break;

        case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
            {
                strUpper = strValue.Upper();
                if ( wxNOT_FOUND != strUpper.Find( _("TRUE") ) ) {
                    m_boolValue = true;
                }
                else {
                    m_boolValue = false;
                }
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            m_longValue = readStringValue( strValue );
            break;
            
        case VSCP_DAEMON_VARIABLE_CODE_LONG:
            m_longValue = readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
            strValue.ToDouble( &m_floatValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
            {
                uint8_t data[ VSCP_MAX_DATA ];
                uint16_t sizeData = 0;
                getVscpDataArrayFromString( data, &sizeData, strValue );
                if ( sizeData > 8 ) sizeData = 8;
                if (sizeData) memcpy( m_normInteger, data, sizeData );
                m_normIntSize = sizeData;
            }
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
            getVscpEventFromString( &m_event, strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
            getGuidFromString( &m_event, strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
            getVscpDataFromString( &m_event, strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
            m_event.vscp_class = readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
            m_event.vscp_type = readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
            m_event.timestamp = readStringValue( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
            m_timestamp.ParseDateTime( strValue );
            break;

        case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
            
            // Fall through - Not allowed here

        default:
            return false;

    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// getVariableFromString
//
// Format is: "variable name",type,"persistence","value"

bool CVSCPVariable::getVariableFromString( const wxString& strVariable )
{
    wxString strName;			      // Name of variable
    wxString strValue;				  // Variable value
    int		 typeVariable;			  // Type of variable;
    bool	 bPersistent = false;	// Persitence of variable		

    wxStringTokenizer tkz( strVariable, _(",") );

    // Get name of variable
    if ( tkz.HasMoreTokens() ) {
        strName = tkz.GetNextToken();
    }
    else {
        return false;	
    }

    // Get the type of the variable
    if ( tkz.HasMoreTokens() ) {
        typeVariable = CVSCPVariable::getVariableTypeFromString( tkz.GetNextToken() );
    }
    else {
        return false;	
    }

    // Get the persistance of the variable
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str = str.Upper();
        if ( wxNOT_FOUND != str.Find( _("TRUE") ) ) {
            bPersistent = true;
        }
    }
    else {
        return false;	
    }

    // Get the value of the variable
    if ( tkz.HasMoreTokens() ) {
        strValue = tkz.GetNextToken();
        strValue.Upper();
    }
    else {
        return false;	
    }

    // Set values
    m_type = typeVariable;
    m_bPersistent = bPersistent;
    setName( strName );
    setValueFromString( typeVariable, strVariable );

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// Reset
//

void CVSCPVariable::Reset( void )
{
    if ( m_bPersistent ) {
        setValueFromString( m_type, m_persistant );
    }
    else {
        switch ( m_type ) { 

            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                m_strValue = _("");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                m_boolValue = false;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                m_longValue = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                m_floatValue = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                // TODO
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                if ( NULL != m_event.pdata ) {
                    delete [] m_event.pdata;
                }
                m_event.pdata = NULL;
                m_event.crc = 0;
                memset( m_event.GUID, 0, 16 );
                m_event.head = 0;
                m_event.obid = 0;
                m_event.sizeData = 0;
                m_event.timestamp = 0;
                m_event.vscp_class = 0;
                m_event.vscp_type = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                memset( m_event.GUID, 0, 16 );
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                if ( NULL != m_event.pdata ) {
                    delete [] m_event.pdata;
                }
                m_event.pdata = NULL;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                m_event.vscp_class = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                m_event.vscp_type = 0;
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                m_timestamp= m_timestamp.Today();
                break;

            case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
            
                // Fall through - Not allowed here

            default:
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// isTrue
//

bool CVSCPVariable::isTrue( void )
{
  if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() && m_boolValue ) {
    return true;  
  }

  if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() && m_longValue ) {
    return true;  
  }

  if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() && m_longValue ) {
    return true;  
  }

  if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() && m_floatValue ) {
    return true;  
  }

  return false;

}

///////////////////////////////////////////////////////////////////////////////
// setTrue
//

void CVSCPVariable::setTrue( void )
{
  if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() ) {
    m_boolValue = true;   
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() ) {
    m_longValue = -1;
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() ) {
    m_longValue = -1; 
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() ) {
    m_floatValue = -1;
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_STRING == getType() ) {
    m_strValue = wxT("true"); 
  }
  
}


///////////////////////////////////////////////////////////////////////////////
// setFalse
//

void CVSCPVariable::setFalse( void )
{
  if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == getType() ) {
    m_boolValue = false;   
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == getType() ) {
    m_longValue = 0;
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == getType() ) {
    m_longValue = 0; 
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == getType() ) {
    m_floatValue = 0;
  }
  else if ( VSCP_DAEMON_VARIABLE_CODE_STRING == getType() ) {
    m_strValue = wxT("false"); 
  }
  
}



//*****************************************************************************
//								CVariableStorage
//*****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVariableStorage::CVariableStorage()
{
#ifndef BUILD_VSCPD_SERVICE
    //wxStandardPaths stdPath;

    // Set the default dm configuration path
#ifdef WIN32
	m_configPath = wxStandardPaths::Get().GetConfigDir();
    m_configPath += _("/vscp/variables.xml");
#else	
    m_configPath = _("/srv/vscp/variables.xml");
#endif	
#endif    
}


///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVariableStorage::~CVariableStorage()
{
    CVSCPVariable *pVariable;

    listVscpVariable::iterator it;
    for( it = m_listVariable.begin(); it != m_listVariable.end(); ++it ) {
        
        if ( NULL == ( pVariable = *it ) ) continue;
        remove( pVariable->getName() );
    }
}



///////////////////////////////////////////////////////////////////////////////
// find
//

CVSCPVariable * CVariableStorage::find( const wxString& name )
{
    wxString strName = name.Upper();
    strName.Trim( true );
  strName.Trim( false );
  CVariableStorage *pVariable = NULL;

    // Check if it is a persistent variable
    if ( strName[ 0 ] == VSCP_DAEMON_VARIABLE_PREFIX_PERSISTENT ) {
        strName = strName.Right( strName.Length() - 1 );
    }

    // Check if it is an array
    if ( strName[ 0 ] == VSCP_DAEMON_VARIABLE_PREFIX_ARRAY ) {
        strName = strName.Right( strName.Length() - 1 );
    }

    if ( m_hashVariable.end() != m_hashVariable.find( strName ) ) {
        return m_hashVariable[ strName ];
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// add
//

bool CVariableStorage::add( const wxString& varName, const wxString& value, uint8_t type, bool bPersistent )
{
    // Name is always upper case
    wxString name = varName.Upper();
  name.Trim( true );
  name.Trim( false );

    CVSCPVariable *pVar = new CVSCPVariable;
  if ( NULL == pVar ) return false;

  pVar->setType( type );			// Store the type

    // Store persistence
    if ( bPersistent ) {
        pVar->setPersistent( true );
    }
    else {
        pVar->setPersistent( false );
    }

    pVar->setName( name );

    // Store value
    if ( !pVar->setValueFromString( type, value ) ) return false;

    // If persistent store persistant value
    if ( bPersistent ) {
        pVar->setPersistatValue( value );
    }

    if ( NULL != find( name ) ) {
        
        // The variable is there already - just change value
        m_hashVariable[ name ]->setValueFromString( m_hashVariable[ name ]->getType(), value );

        delete pVar;	// No use for the variable

    }
    else {
        // New variable
        m_hashVariable[ name ] = pVar;
        m_listVariable.Append( pVar );
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// add
//

bool CVariableStorage::add( CVSCPVariable *pVar )
{
    // Check that the pointer is valid
    if ( NULL == pVar ) return false;

    if ( NULL != find( pVar->getName() ) ) {
        
        // The variable is there already - remove so we could add the new
    remove( pVar->getName() );

        // New variable
    m_hashVariable[ pVar->getName() ] = pVar;
        m_listVariable.Append( pVar );

    }
    else {
        // New variable
    m_hashVariable[ pVar->getName() ] = pVar;
        m_listVariable.Append( pVar );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// remove
//

bool CVariableStorage::remove( wxString& name ) 
{
    m_listVariable.DeleteObject( m_hashVariable[ name ] );
    return m_hashVariable.erase( name ) ? true : false;
}

///////////////////////////////////////////////////////////////////////////////
// load
//
// Read persistent variables from file
//

bool CVariableStorage::load( void )
{
    bool bArray;
    wxXmlDocument doc;

#ifdef BUILD_VSCPD_SERVICE
    wxStandardPaths stdPath;

    // Set the default dm configuration path
#ifdef WIN32	
    m_configPath = stdPath.GetConfigDir();
    m_configPath += _("/vscp/variables.xml");
#else
	m_configPath = _("/srv/vscp/variables.xml");
#endif	
#endif

    if (!doc.Load( m_configPath ) ) {
        return false;
    }

    //::wxLogDebug ( _("Loading variables from: \n\t") + m_configPath );

    // start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT("persistent") ) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == wxT("variable")) {

            CVSCPVariable *pVar = new CVSCPVariable;
            pVar->setPersistent( true );     // Loaded variables are persistent
            bArray = false;

            // Get variable type - String is default
#if wxCHECK_VERSION(3,0,0)            
            pVar->setType( pVar->getVariableTypeFromString( child->GetAttribute( wxT("type"), wxT("string") ) ) );  
#else
            pVar->setType( pVar->getVariableTypeFromString( child->GetPropVal( wxT("type"), wxT("string") ) ) );
#endif             

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("name")) {
                    wxString strName = subchild->GetNodeContent();
                    strName.Trim();
                    strName.Trim(false);
                    // Replace spaces in name with underscore
                    int pos;
                    while (wxNOT_FOUND != ( pos = strName.Find(_(" ")))){
                        strName.SetChar(pos,wxChar('_'));
                    }
                    pVar->setName( strName );
                }
                else if (subchild->GetName() == wxT("value")) {
                    pVar->setValueFromString( pVar->getType(), subchild->GetNodeContent() );
                    pVar->setPersistatValue( subchild->GetNodeContent() );
                }
                else if (subchild->GetName() == wxT("note")) {
                    pVar->setNote( subchild->GetNodeContent() );
                }

                subchild = subchild->GetNext();

            }

            // Add the variable (always persistent if from file storage)
            add( pVar );

        }

        child = child->GetNext();

    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// save
//
// Write persistent variables to file 
//

bool CVariableStorage::save()
{
#ifdef BUILD_VSCPD_SERVICE
    wxStandardPaths stdPath;

    // Set the default dm configuration path
    m_configPath = stdPath.GetConfigDir();
    m_configPath += _("/vscp/variable.xml");
#endif
	return save( m_configPath );
}


bool CVariableStorage::save( wxString& path )
{
    CVSCPVariable *pVariable;
    wxString str;
#ifdef BUILD_VSCPD_SERVICE
    wxStandardPaths stdPath;

    // Set the default variable configuration path
    m_configPath = stdPath.GetConfigDir();
    m_configPath += _("/vscp/variable.xml");
#endif

    wxFFileOutputStream *pFileStream = new wxFFileOutputStream( path );
    if ( NULL == pFileStream ) return false;

    pFileStream->Write("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n", 
                        strlen("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n\n") );

    // VSCP variables
    pFileStream->Write("<persistent>\n\n", strlen("<persistent>\n\n") );

    listVscpVariable::iterator it;
    for( it = m_listVariable.begin(); it != m_listVariable.end(); ++it ) {
        
        if ( NULL == ( pVariable = *it ) ) continue;
        wxString name = pVariable->getName();

        if ( ( NULL != pVariable ) && pVariable->isPersistent() ) {
    
            switch ( pVariable->getType() ) {

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                    
                    str.Printf( _("  <variable type=\"event\">\n"), pVariable->getType() );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //pFileStream->Write( pVariable->m_strValue, pVariable->m_strValue.Length() );
                    pVariable->writeVariableToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_STRING:

                    str.Printf( _("  <variable type=\"string\">\n"), pVariable->getType() );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pFileStream->Write( pVariable->m_strValue.mb_str(), 
                            strlen( pVariable->m_strValue.mb_str() ) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:

                    str.Printf( _("  <variable type=\"bool\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    if ( pVariable->m_boolValue ) {
                        pFileStream->Write( "true", strlen("true") );
                    }
                    else {
                        pFileStream->Write( "false", strlen("false") );
                    }
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_INTEGER:

                    str.Printf( _("  <variable type=\"integer\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%d"), pVariable->m_longValue );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_LONG:

                    str.Printf( _("  <variable type=\"long\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%d"), pVariable->m_longValue );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:

                    str.Printf( _("  <variable type=\"double\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%f"), pVariable->m_floatValue );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:

                    str.Printf( _("  <variable type=\"measurement\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //writeVscpDataWithSizeToString( pVariable->m_normIntSize, 
                    //                                pVariable->m_normInteger, 
                    //                                str,
                    //                                true );
                    //pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pVariable->writeVariableToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;
                    
                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:

                    str.Printf( _("  <variable type=\"data\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeVariableToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;    

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:

                    str.Printf( _("  <variable type=\"class\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%d"), pVariable->m_event.vscp_class );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:

                    str.Printf( _("  <variable type=\"type\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    str.Printf( _("%d"), pVariable->m_event.vscp_type );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;


                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:

                    str.Printf( _("  <variable type=\"timestamp\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //str.Printf( _("%d"), pVariable->m_event.timestamp );
                    //pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pVariable->writeVariableToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:

                    str.Printf( _("  <variable type=\"guid\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    pVariable->writeVariableToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DATETIME:

                    str.Printf( _("  <variable type=\"datetime\">\n") );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    writeGuidToString( &pVariable->m_event, str );

                    // Write name
                    pFileStream->Write( "    <name>", strlen("    <name>") );
                    str = pVariable->getName();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</name>\n", strlen("</name>\n") );

                    // Write note
                    pFileStream->Write( "    <note>", strlen("    <note>") );
                    str = pVariable->getNote();
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</note>\n", strlen("</note>\n") );

                    // Write value
                    pFileStream->Write( "    <value>", strlen("    <value>") );
                    //pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    //str = pVariable->m_timestamp.FormatISODate();
                    //str += _(" ");
                    //str = pVariable->m_timestamp.FormatISOTime();
                    pVariable->writeVariableToString( str );
                    pFileStream->Write( str.mb_str(), strlen(str.mb_str()) );
                    pFileStream->Write( "</value>\n", strlen("</value>\n") );

                    pFileStream->Write( "  </variable>\n\n", strlen("  </variable>\n\n") );
                    break;

            }	

        }
        
    }

    // DM matrix information end
    pFileStream->Write("</persistent>\n",strlen("</persistent>\n"));

    // Close the file
    pFileStream->Close();

    return true;
}



